# Chat Server with Groups and Private Messages

## Table of Contents

1. [Introduction](#introduction)
2. [How to Run](#how-to-run)
3. [Assignment Features](#assignment-features)
4. [Design Decisions](#design-decisions)
5. [Implementation](#implementation)
6. [Testing](#testing)
7. [Edge Cases Considered](#edge-cases-considered)
8. [Restrictions](#restrictions)
9. [Challenges](#challenges)
10. [Contribution of Each Member](#contribution-of-each-member)
11. [Sources Referred](#sources-referred)
12. [Declaration](#declaration)
13. [Feedback](#feedback)

---

## Introduction

This is a multi-threaded chat server that enables user authentication, private messaging, group communication, and real-time interaction using TCP sockets. The project is part of the CS425: Computer Networks course.

## How to Run

- **Building the Project**:
  - Open a terminal in the project directory.
  - Run `make` to compile the project. This will generate the executables `server_grp` and `client_grp`.
- **Running the Server**:
  - In a terminal window, run `./server_grp` to start the server.
  - To shut down the server, type `exit` in the server terminal and press Enter.
- **Running a Client**:
  - In a separate terminal window, run `./client_grp` to start a client session.
  - Multiple client terminals can be opened to simulate simultaneous users.

## Assignment Features

### Implemented Features

- Multi-client handling with threading
- User authentication using `users.txt`
- Private messaging between users
- Broadcast messaging to all connected clients
- Group management: create, join, leave groups
- Group messaging
- Server shutdown functionality

## Design Decisions

- **Threading Model**: We create a new thread for each client connection. This ensures concurrency and allows multiple users to interact without blocking others.
- **Synchronization**: `std::mutex` is used to protect shared resources like client lists and group mappings.
- **Authentication Handling**: Credentials are stored in a `users.txt` file and loaded into memory.
- **Message Parsing**: Commands follow a structured format (e.g., `/msg`, `/group_msg`) for easy processing.

## Implementation

### High-Level Overview

- **Server Startup**:
  - **Credential Loading**: On startup, the server reads the `users.txt` file and loads valid username-password pairs into an in-memory data structure for quick authentication.
  - **Socket Creation & Binding**: A TCP socket is created, bound to a predefined port (specified by the `PORT` macro), and set to listen for incoming connections.
  - **Listening for Connections**: The server enters a loop where it accepts each new client connection and spawns a dedicated thread for that client.
- **Client Connection Handling**:
  - **Authentication**: Upon connection, clients are prompted for a username and password. These credentials are verified against the in-memory list. Duplicate logins (using the same username) from different terminals are rejected to avoid ambiguity in private messaging.
  - **Client Registration**: Successful authentication leads to the client being added to a global client map, and all connected clients are notified of the new connection.
- **Message Routing & Command Processing**:
  - **Command Parsing**: The server inspects the prefix of each message to determine the command (e.g., `/msg`, `/broadcast`, `/create_group`, etc.).
  - **Validation & Routing**: Each command is validated for correct format and parameters. Private messages are delivered only to connected users, broadcasts are sent to all clients except the sender, and group messages are relayed only if the sender is a member of the specified group.
- **Group Management**:
  - **Group Creation**: Users can create groups using the `/create_group <group name>` command. Group names must not contain spaces.
  - **Joining and Leaving Groups**: Users can join groups with `/join_group <group name>` and leave groups with `/leave_group <group name>`. The server enforces that a user cannot join the same group multiple times or leave a group they are not part of.
  - **Group Messaging**: Only members of a group can send messages to that group via the `/group_msg <group name> <message>` command.
- **Server Shutdown**:
  - **Control Thread**: A separate control thread listens for input from the server terminal. When the administrator types `exit`, the server shuts down gracefully by closing all active connections and terminating the process.
- **Resource Cleanup**:
  - **Disconnection Handling**: When a client disconnects (either voluntarily by typing `exit` or due to a network failure), the server removes the client from all data structures (active client list and any group memberships) to ensure proper resource cleanup.

## Testing

### Testing Methodologies

- **Correctness Testing**: Verified expected input/output for each command.
- **Stress Testing**: Simulated multiple clients sending messages simultaneously.
- **Edge Case Testing**: Handled scenarios such as incorrect usernames, empty messages, and invalid group operations.

## Edge Cases Considered

- **Invalid Credentials**: Users entering incorrect usernames or passwords are denied access.
- **Simultaneous Login Restriction:** Two users with the exact same credentials cannot log in from different terminals simultaneously (we assume that two people cannot have the same username since the private messaging functionality would otherwise be ambiguous).
- **Private Messaging Connection Requirement**: A user can only send a private message to another user if they are connected to the server.
- **Empty Messages**: Rejects empty messages.
- **Self Messaging**: Prevents users from sending private messages to themselves.
- **Non-existent Group Messaging**: Ensures messages are not sent to non-existent groups.
- **Unauthorized Group Messaging**: Prevents non-members from sending messages to a group.
- **Multiple Group Joins**: Prevents users from joining the same group multiple times.
- **Leaving a Non-joined Group**: Ensures users cannot leave groups they are not part of.
- **Group Name Validation**: Disallows spaces in group names.
- **Server Crash Handling**: Ensures proper cleanup of disconnected clients to prevent memory leaks.
- **Client Exit Handling**: If a user types `exit` in their terminal, they are disconnected from the server with a goodbye message.

## Restrictions

- **Maximum Clients**: Limited by system resources but practically tested with 10 clients.
- **Maximum Groups**: No enforced limit.
- **Maximum Group Members**: No enforced limit.
- **Maximum Message Size**: Limited by `BUFFER_SIZE` (1024 bytes).

## Challenges

- **Synchronization Issues**: Initially faced race conditions when updating shared structures.
- **Thread Management**: Ensuring proper cleanup of disconnected clients.
- **Message Parsing**: Handling different command formats efficiently.

## Contribution of Each Member

| Team Member              | Contribution (%) | Responsibilities                                                                   |
| ------------------------ | ---------------- | ---------------------------------------------------------------------------------- |
| Nilay Agarwal(220714)    | 50%              | Server architecture, Threading, Authentication, Message handling, Debugging        |
| Aarsh Walavalkar(220013) | 50%              | Message handling, Group management, Testing, README preparation, Final refinements |

## Sources Referred

- CS425 Lecture Slides
- [GeeksforGeeks: Socket Programming in C++](https://www.geeksforgeeks.org/socket-programming-in-cpp/)
- [Medium: IP TCP Programming for Beginners Using C++](https://medium.com/@naseefcse/ip-tcp-programming-for-beginners-using-c-5bafb3788001)

## Declaration

We declare that this assignment was completed independently without any form of plagiarism.

## Feedback

- The assignment was insightful for understanding socket programming.
- Consider adding bonus features like file transfer for future iterations.
