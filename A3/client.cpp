#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <sys/time.h>

using namespace std;

//---------------------------------------------------------------------------------
// Constants used in the handshake:
//
// SERVER_IP:       IP address of the server (localhost)
// SERVER_PORT:     Port on which the server listens (12345)
// CLIENT_SYN_SEQ:  Sequence number for our SYN packet (200)
// SERVER_SYN_SEQ:  Expected server sequence in the SYN-ACK (400)
// CLIENT_ACK_SEQ:  Sequence number for our final ACK (600)
// TIMEOUT_SECONDS: Overall hardcoded timeout (in seconds) for waiting for a valid SYN-ACK
//---------------------------------------------------------------------------------
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define CLIENT_SYN_SEQ 200
#define SERVER_SYN_SEQ 400
#define CLIENT_ACK_SEQ 600
#define TIMEOUT_SECONDS 5

//---------------------------------------------------------------------------------
// compute_checksum: Calculates the checksum for the IP header.
// This function computes a simple checksum over the provided data.
//---------------------------------------------------------------------------------
unsigned short compute_checksum(unsigned short *ptr, int nbytes)
{
    long sum = 0;
    while (nbytes > 1)
    {
        sum += *ptr++;
        nbytes -= 2;
    }
    if (nbytes == 1)
    {
        sum += *(unsigned char *)ptr;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

//---------------------------------------------------------------------------------
// send_tcp_packet: Constructs and sends a TCP packet using raw sockets.
//
// Parameters:
//   sock     - The raw socket file descriptor.
//   src_ip   - Source IP address as a string.
//   dst_ip   - Destination IP address as a string.
//   src_port - Source port number.
//   dst_port - Destination port number.
//   seq      - TCP sequence number for the packet.
//   ack_seq  - TCP acknowledgment number (set to 0 if not applicable).
//   syn      - If true, sets the SYN flag.
//   ack      - If true, sets the ACK flag.
//
// This function builds both an IP header and a TCP header, then sends the packet.
void send_tcp_packet(int sock, const char *src_ip, const char *dst_ip,
                     int src_port, int dst_port, uint32_t seq, uint32_t ack_seq,
                     bool syn, bool ack)
{
    // Buffer to hold the combined IP and TCP headers.
    char packet[sizeof(struct iphdr) + sizeof(struct tcphdr)];
    memset(packet, 0, sizeof(packet));

    // Pointers to the IP and TCP headers within our packet.
    struct iphdr *ip = (struct iphdr *)packet;
    struct tcphdr *tcp = (struct tcphdr *)(packet + sizeof(struct iphdr));

    // ----- Build the IP Header -----
    ip->ihl = 5;                         // Header length (5 * 4 = 20 bytes)
    ip->version = 4;                     // IPv4
    ip->tos = 0;                         // Type of service
    ip->tot_len = htons(sizeof(packet)); // Total packet length
    ip->id = htons(54321);               // Identification (arbitrary)
    ip->frag_off = 0;                    // No fragmentation
    ip->ttl = 64;                        // Time to live
    ip->protocol = IPPROTO_TCP;          // Protocol: TCP
    // For our server's logic, the source IP is set to the client's address.
    ip->saddr = inet_addr(src_ip);                                            // Source IP address (client)
    ip->daddr = inet_addr(dst_ip);                                            // Destination IP address (server)
    ip->check = 0;                                                            // Checksum initially 0
    ip->check = compute_checksum((unsigned short *)ip, sizeof(struct iphdr)); // Compute checksum

    // ----- Build the TCP Header -----
    tcp->source = htons(src_port); // Source port (client port)
    tcp->dest = htons(dst_port);   // Destination port (server port)
    tcp->seq = htonl(seq);         // Set TCP sequence number
    tcp->ack_seq = htonl(ack_seq); // Set TCP acknowledgment number
    tcp->doff = 5;                 // Data offset: 5 (20 bytes TCP header)
    tcp->syn = syn ? 1 : 0;        // Set SYN flag if needed
    tcp->ack = ack ? 1 : 0;        // Set ACK flag if needed
    tcp->window = htons(8192);     // Window size (arbitrary)
    tcp->check = 0;                // TCP checksum (left at 0 for simplicity)
    tcp->urg_ptr = 0;              // Urgent pointer (not used)

    // Set up the destination address structure.
    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = htons(dst_port);
    dest.sin_addr.s_addr = inet_addr(dst_ip);

    // Send the packet using sendto().
    if (sendto(sock, packet, sizeof(packet), 0,
               (struct sockaddr *)&dest, sizeof(dest)) < 0)
    {
        perror("sendto() failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        if (syn && !ack)
            cout << "[+] Sent SYN packet with sequence " << seq << endl;
        else if (ack && !syn)
            cout << "[+] Sent ACK packet with sequence " << seq << endl;
        else
            cout << "[+] Sent packet" << endl;
    }
}

//---------------------------------------------------------------------------------
// receive_tcp_packet: Receives a TCP packet from the raw socket and extracts the TCP header.
//
// Parameters:
//   sock         - The raw socket file descriptor.
//   tcp_header   - Reference to a tcphdr structure to store the received TCP header.
//   source_addr  - Reference to a sockaddr_in structure to store the sender's address.
//
// Returns:
//   true if a packet is successfully received and parsed; false otherwise.
bool receive_tcp_packet(int sock, struct tcphdr &tcp_header, struct sockaddr_in &source_addr)
{
    char buffer[65536];
    memset(buffer, 0, sizeof(buffer));
    socklen_t addr_len = sizeof(source_addr);

    int data_size = recvfrom(sock, buffer, sizeof(buffer), 0,
                             (struct sockaddr *)&source_addr, &addr_len);
    if (data_size < 0)
    {
        perror("recvfrom() failed");
        return false;
    }

    // Extract the IP header to determine its length.
    struct iphdr *ip = (struct iphdr *)buffer;
    int ip_header_len = ip->ihl * 4; // Convert header length from 32-bit words to bytes

    // Extract the TCP header (immediately following the IP header).
    struct tcphdr *tcp = (struct tcphdr *)(buffer + ip_header_len);
    memcpy(&tcp_header, tcp, sizeof(struct tcphdr));
    return true;
}

//---------------------------------------------------------------------------------
// main: Implements the client-side TCP handshake using raw sockets.
//
// Handshake process:
//   1. Send a SYN packet with sequence number CLIENT_SYN_SEQ (200).
//   2. Wait (using a loop with a hardcoded overall timeout) for a valid SYN-ACK packet.
//      - A valid SYN-ACK must have both SYN and ACK flags set, and the acknowledgment number must equal CLIENT_SYN_SEQ + 1.
//   3. If a valid SYN-ACK is received, send the final ACK packet with sequence number CLIENT_ACK_SEQ (600)
//      and acknowledgment number SERVER_SYN_SEQ + 1 (401).
//   4. The handshake only completes if the correct sequence numbers are used; otherwise, it fails.
//---------------------------------------------------------------------------------
int main()
{
    const char *client_ip = "127.0.0.1"; // Client's IP (localhost)
    const char *server_ip = SERVER_IP;   // Server's IP (localhost)
    int client_port = 54321;             // Arbitrary client port
    int server_port = SERVER_PORT;       // Server listening port (12345)

    // Create a raw socket for TCP.
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Enable the IP_HDRINCL option so that our custom IP header is used.
    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0)
    {
        perror("setsockopt() failed");
        exit(EXIT_FAILURE);
    }

    // ----- STEP 1: Send SYN Packet -----
    send_tcp_packet(sock, client_ip, server_ip, client_port, server_port,
                    CLIENT_SYN_SEQ, 0, true, false);

    // ----- STEP 2: Wait for a Valid SYN-ACK Packet -----
    // Loop until a valid SYN-ACK is received or the overall timeout expires.
    struct timeval start, now;
    gettimeofday(&start, NULL);
    bool valid_syn_ack = false;
    struct tcphdr recv_tcp;
    struct sockaddr_in source_addr;

    while (true)
    {
        // Check elapsed time.
        gettimeofday(&now, NULL);
        long elapsed = now.tv_sec - start.tv_sec;
        if (elapsed >= TIMEOUT_SECONDS)
        {
            break;
        }

        // Set up select() for the remaining time.
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        struct timeval remaining;
        remaining.tv_sec = TIMEOUT_SECONDS - elapsed;
        remaining.tv_usec = 0;

        int sel = select(sock + 1, &readfds, NULL, NULL, &remaining);
        if (sel <= 0)
        {
            // No data available in the remaining time; check timeout.
            continue;
        }

        // Try to receive a packet.
        if (!receive_tcp_packet(sock, recv_tcp, source_addr))
        {
            continue;
        }

        // Debug output for troubleshooting.
        cout << "[DEBUG] Received packet flags: SYN=" << (int)recv_tcp.syn
             << " ACK=" << (int)recv_tcp.ack
             << " Seq=" << ntohl(recv_tcp.seq)
             << " Ack=" << ntohl(recv_tcp.ack_seq) << endl;

        // Validate: Check if the packet has both SYN and ACK flags set and the correct acknowledgment number.
        if (recv_tcp.syn && recv_tcp.ack && (ntohl(recv_tcp.ack_seq) == CLIENT_SYN_SEQ + 1))
        {
            valid_syn_ack = true;
            break;
        }
    }

    if (!valid_syn_ack)
    {
        cerr << "[-] Timeout or invalid SYN-ACK received. Handshake failed." << endl;
        close(sock);
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "[+] Received valid SYN-ACK from server." << endl;
    }

    // ----- STEP 3: Send Final ACK Packet to Complete Handshake -----
    send_tcp_packet(sock, client_ip, server_ip, client_port, server_port,
                    CLIENT_ACK_SEQ, SERVER_SYN_SEQ + 1, false, true);
    cout << "[+] Handshake complete." << endl;

    close(sock);
    return 0;
}
