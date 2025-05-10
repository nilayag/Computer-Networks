#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <cctype>
#include <cstdlib>

#define PORT 12345
#define BUFFER_SIZE 1024

using namespace std;

// Global data structures:
// - 'clients' maps each client socket to its authenticated username.
// - 'users' holds valid username:password pairs loaded from a file.
// - 'groups' maps group names to the set of client sockets that are members.
unordered_map<int, string> clients;
mutex clients_mutex;
unordered_map<string, string> users;
unordered_map<string, unordered_set<int>> groups;
mutex groups_mutex;

// Loads user credentials from a file where each line is formatted as "username:password".
void load_users(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Error: Unable to open file \"" << filename << "\".\n";
        return;
    }
    string line;
    while (getline(file, line))
    {
        size_t pos = line.find(':');
        if (pos != string::npos)
        {
            string username = line.substr(0, pos);
            string password = line.substr(pos + 1);
            users[username] = password;
        }
    }
    file.close();
}

// Helper function to check if a string contains any spaces.
bool contains_space(const string &s)
{
    return s.find(' ') != string::npos;
}

// Sends a formatted group message to all members of a group except the sender.
// Assumes that group existence and membership have already been validated.
void group_message(int client_socket, const string &group_name, const string &message)
{
    lock_guard<mutex> lock(groups_mutex);
    string full_message = "[Group " + group_name + "]: " + message + "\n";
    for (int member_socket : groups[group_name])
    {
        if (member_socket != client_socket)
        {
            send(member_socket, full_message.c_str(), full_message.size(), 0);
        }
    }
}

// Handles communication with a connected client.
void handle_client(int client_socket)
{
    char buffer[BUFFER_SIZE];
    string username;

    // --- Authentication Phase ---
    string prompt = "Enter username: ";
    send(client_socket, prompt.c_str(), prompt.size(), 0);
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0)
    {
        close(client_socket);
        return;
    }
    username = string(buffer, bytes_received);

    string password_prompt = "Enter password: ";
    send(client_socket, password_prompt.c_str(), password_prompt.size(), 0);
    memset(buffer, 0, BUFFER_SIZE);
    bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0)
    {
        close(client_socket);
        return;
    }
    string password = string(buffer, bytes_received);

    // Validate credentials.
    if (users.find(username) != users.end() && users[username] == password)
    {
        {
            // Prevent duplicate connections using the same username.
            lock_guard<mutex> lock(clients_mutex);
            for (const auto &client : clients)
            {
                if (client.second == username)
                {
                    string err = "Error: User \"" + username + "\" is already connected.\n";
                    send(client_socket, err.c_str(), err.size(), 0);
                    close(client_socket);
                    return;
                }
            }
            // Add the new client to the active client list.
            clients[client_socket] = username;

            // Inform other connected clients of the new connection.
            string join_msg = username + " has joined the chat.\n";
            for (const auto &client : clients)
            {
                if (client.first != client_socket)
                {
                    send(client.first, join_msg.c_str(), join_msg.size(), 0);
                }
            }
        }
        cout << username << " connected." << endl;
        string welcome = "Welcome to the chat server!\n";
        send(client_socket, welcome.c_str(), welcome.size(), 0);

        // --- Main Communication Loop ---
        while (true)
        {
            memset(buffer, 0, BUFFER_SIZE);
            bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_received <= 0)
                break;

            string message = string(buffer, bytes_received);

            // Disconnect if client types "exit".
            if (message == "exit")
            {
                string bye = "Goodbye.\n";
                send(client_socket, bye.c_str(), bye.size(), 0);
                break;
            }

            // Ensure the message is not empty.
            if (message.empty())
            {
                string err = "Error: Message cannot be empty.\n";
                send(client_socket, err.c_str(), err.size(), 0);
                continue;
            }

            // Process commands based on their prefix.
            // Command: /msg <username> <message>
            if (message.substr(0, 4) == "/msg")
            {
                size_t space1 = message.find(' ', 5);
                if (space1 == string::npos)
                {
                    string err = "Error: Incorrect format. Use: /msg <username> <message>\n";
                    send(client_socket, err.c_str(), err.size(), 0);
                    continue;
                }
                string target_user = message.substr(5, space1 - 5);
                string private_msg = message.substr(space1 + 1);
                if (private_msg.empty())
                {
                    string err = "Error: Private message content is empty.\n";
                    send(client_socket, err.c_str(), err.size(), 0);
                    continue;
                }
                int target_socket = -1;
                {
                    lock_guard<mutex> lock(clients_mutex);
                    for (const auto &client : clients)
                    {
                        if (client.second == target_user)
                        {
                            target_socket = client.first;
                            break;
                        }
                    }
                }
                if (target_socket != -1)
                {
                    if (target_socket == client_socket)
                    {
                        string self_msg = "Error: Cannot send a private message to yourself.\n";
                        send(client_socket, self_msg.c_str(), self_msg.size(), 0);
                    }
                    else
                    {
                        string full_msg = "[" + username + "]: " + private_msg + "\n";
                        send(target_socket, full_msg.c_str(), full_msg.size(), 0);
                    }
                }
                else
                {
                    string not_found = "Error: User \"" + target_user + "\" not found.\n";
                    send(client_socket, not_found.c_str(), not_found.size(), 0);
                }
            }
            // Command: /broadcast <message>
            else if (message.substr(0, 10) == "/broadcast")
            {
                if (message.size() <= 10 || message[10] != ' ')
                {
                    string err = "Error: Incorrect format. Use: /broadcast <message>\n";
                    send(client_socket, err.c_str(), err.size(), 0);
                    continue;
                }
                string broadcast_content = message.substr(11);
                if (broadcast_content.empty())
                {
                    string err = "Error: Broadcast message content is empty.\n";
                    send(client_socket, err.c_str(), err.size(), 0);
                    continue;
                }
                string broadcast_msg = "[" + username + "] (Broadcast): " + broadcast_content + "\n";
                lock_guard<mutex> lock(clients_mutex);
                for (const auto &client : clients)
                {
                    if (client.first != client_socket)
                        send(client.first, broadcast_msg.c_str(), broadcast_msg.size(), 0);
                }
            }
            // Command: /create_group <group name>
            else if (message.substr(0, 13) == "/create_group")
            {
                if (message.size() <= 13 || message[13] != ' ')
                {
                    string err = "Error: Incorrect format. Use: /create_group <group name>\n";
                    send(client_socket, err.c_str(), err.size(), 0);
                    continue;
                }
                string group_name = message.substr(14);
                if (group_name.empty())
                {
                    string err = "Error: Group name cannot be empty.\n";
                    send(client_socket, err.c_str(), err.size(), 0);
                    continue;
                }
                // Check that group names do not contain spaces.
                if (contains_space(group_name))
                {
                    string err = "Error: Group name must not contain spaces.\n";
                    send(client_socket, err.c_str(), err.size(), 0);
                    continue;
                }
                {
                    lock_guard<mutex> lock(groups_mutex);
                    if (!groups.count(group_name))
                    {
                        groups[group_name].insert(client_socket);
                        string reply = "Group \"" + group_name + "\" created successfully.\n";
                        send(client_socket, reply.c_str(), reply.size(), 0);
                    }
                    else
                    {
                        string reply = "Error: Group \"" + group_name + "\" already exists.\n";
                        send(client_socket, reply.c_str(), reply.size(), 0);
                    }
                }
            }
            // Command: /join_group <group name>
            else if (message.substr(0, 11) == "/join_group")
            {
                if (message.size() <= 11 || message[11] != ' ')
                {
                    string err = "Error: Incorrect format. Use: /join_group <group name>\n";
                    send(client_socket, err.c_str(), err.size(), 0);
                    continue;
                }
                string group_name = message.substr(12);
                if (group_name.empty())
                {
                    string err = "Error: Group name cannot be empty.\n";
                    send(client_socket, err.c_str(), err.size(), 0);
                    continue;
                }
                {
                    lock_guard<mutex> lock(groups_mutex);
                    if (!groups.count(group_name))
                    {
                        string reply = "Error: Group \"" + group_name + "\" does not exist.\n";
                        send(client_socket, reply.c_str(), reply.size(), 0);
                    }
                    else
                    {
                        // Prevent joining the same group more than once.
                        if (groups[group_name].find(client_socket) != groups[group_name].end())
                        {
                            string err = "Error: Already a member of group \"" + group_name + "\".\n";
                            send(client_socket, err.c_str(), err.size(), 0);
                        }
                        else
                        {
                            groups[group_name].insert(client_socket);
                            string reply = "Joined group \"" + group_name + "\" successfully.\n";
                            send(client_socket, reply.c_str(), reply.size(), 0);
                        }
                    }
                }
            }
            // Command: /group_msg <group name> <message>
            else if (message.substr(0, 10) == "/group_msg")
            {
                if (message.size() <= 10 || message[10] != ' ')
                {
                    string err = "Error: Incorrect format. Use: /group_msg <group name> <message>\n";
                    send(client_socket, err.c_str(), err.size(), 0);
                    continue;
                }
                size_t space1 = message.find(' ', 11);
                if (space1 == string::npos)
                {
                    string err = "Error: Incorrect format. Use: /group_msg <group name> <message>\n";
                    send(client_socket, err.c_str(), err.size(), 0);
                    continue;
                }
                string group_name = message.substr(11, space1 - 11);
                string group_msg = message.substr(space1 + 1);
                if (group_msg.empty())
                {
                    string err = "Error: Group message content is empty.\n";
                    send(client_socket, err.c_str(), err.size(), 0);
                    continue;
                }
                {
                    lock_guard<mutex> lock(groups_mutex);
                    if (!groups.count(group_name))
                    {
                        string err = "Error: Group \"" + group_name + "\" does not exist.\n";
                        send(client_socket, err.c_str(), err.size(), 0);
                        continue;
                    }
                    // Verify that the sender is a member of the group.
                    if (groups[group_name].find(client_socket) == groups[group_name].end())
                    {
                        string err = "Error: Not a member of group \"" + group_name + "\".\n";
                        send(client_socket, err.c_str(), err.size(), 0);
                        continue;
                    }
                }
                group_message(client_socket, group_name, group_msg);
            }
            // Command: /leave_group <group name>
            else if (message.substr(0, 12) == "/leave_group")
            {
                if (message.size() <= 12 || message[12] != ' ')
                {
                    string err = "Error: Incorrect format. Use: /leave_group <group name>\n";
                    send(client_socket, err.c_str(), err.size(), 0);
                    continue;
                }
                string group_name = message.substr(13);
                if (group_name.empty())
                {
                    string err = "Error: Group name cannot be empty.\n";
                    send(client_socket, err.c_str(), err.size(), 0);
                    continue;
                }
                {
                    lock_guard<mutex> lock(groups_mutex);
                    if (groups.count(group_name))
                    {
                        if (groups[group_name].find(client_socket) != groups[group_name].end())
                        {
                            groups[group_name].erase(client_socket);
                            string reply = "Left group \"" + group_name + "\" successfully.\n";
                            send(client_socket, reply.c_str(), reply.size(), 0);
                        }
                        else
                        {
                            string reply = "Error: Not a member of group \"" + group_name + "\".\n";
                            send(client_socket, reply.c_str(), reply.size(), 0);
                        }
                    }
                    else
                    {
                        string reply = "Error: Group \"" + group_name + "\" does not exist.\n";
                        send(client_socket, reply.c_str(), reply.size(), 0);
                    }
                }
            }
            // Unknown command.
            else
            {
                string err = "Error: Unknown command.\n";
                send(client_socket, err.c_str(), err.size(), 0);
            }
        }

        // --- Client Disconnection ---
        {
            lock_guard<mutex> lock(clients_mutex);
            string leave_msg = username + " has left the chat.\n";
            for (const auto &client : clients)
            {
                if (client.first != client_socket)
                {
                    send(client.first, leave_msg.c_str(), leave_msg.size(), 0);
                }
            }
            clients.erase(client_socket);
        }
        cout << username << " disconnected." << endl;
    }
    else
    {
        string auth_fail = "Error: Authentication failed.\n";
        send(client_socket, auth_fail.c_str(), auth_fail.size(), 0);
    }
    close(client_socket);
}

int main()
{
    // Load valid user credentials from file.
    load_users("users.txt");

    // Create a TCP socket.
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        cerr << "Error: Unable to create socket.\n";
        return 1;
    }

    // Set up the server address structure.
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the specified port.
    if (::bind(server_socket, (sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        cerr << "Error: Unable to bind socket.\n";
        close(server_socket);
        return 1;
    }

    // Listen for incoming connections.
    if (listen(server_socket, 5) < 0)
    {
        cerr << "Error: Unable to listen on port " << PORT << ".\n";
        close(server_socket);
        return 1;
    }

    cout << "Server is now listening on port " << PORT << "...\n";

    // --- Server Control Thread ---
    // Allows the server administrator to type "exit" in the server terminal to shut down the server.
    thread server_control([]()
                          {
        string command;
        while(getline(cin, command)) {
            if(command == "exit") {
                cout << "Server shutting down...\n";
                exit(0);
            }
        } });
    server_control.detach();

    // Main loop to accept and handle client connections.
    while (true)
    {
        sockaddr_in client_addr{};
        socklen_t addr_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0)
        {
            cerr << "Error: Failed to accept client connection.\n";
            continue;
        }
        // Create a new thread to handle the connected client.
        thread client_thread(handle_client, client_socket);
        client_thread.detach();
    }

    close(server_socket);
    return 0;
}
