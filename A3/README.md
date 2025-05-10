# TCP Handshake Assignment – Client-Side Implementation

## Overview

This assignment implements the client side of a simplified TCP three-way handshake using raw sockets in C++. The provided server code (`server.cpp`) listens for a SYN packet with sequence number 200, sends back a SYN-ACK with sequence number 400 (and acknowledgment number 201), and expects a final ACK with sequence number 600 (and acknowledgment number 401).

A hardcoded timeout of 5 seconds ensures that if a valid SYN-ACK is not received, the handshake is aborted. We also verified that if wrong sequence numbers are used (by manually changing them), the handshake fails as expected.

## Features

### Implemented

- Creation of raw sockets.
- Custom IP and TCP header construction.
- Three-way handshake logic using hardcoded sequence numbers.
- A hardcoded timeout of 5 seconds to abort the handshake if the SYN-ACK is not received.
- Basic error handling and debug outputs.

## Design and Functionality

### High-Level Idea

The client initiates a handshake by sending a SYN packet with sequence number 200. Upon receiving a SYN-ACK (with sequence number 400 and acknowledgment number 201), the client responds with a final ACK (sequence number 600, acknowledgment number 401). If this exchange occurs correctly, the handshake is deemed successful.

### Important Functions

- `compute_checksum()`: Calculates the checksum for our custom-built IP header.
- `send_tcp_packet()`: Builds and sends a packet by constructing both the IP and TCP headers with the proper flags and sequence numbers.
- `receive_tcp_packet()`: Waits for an incoming packet on the raw socket, extracts the TCP header, and returns it for validation.
- `main()`: Orchestrates the handshake by:
  - Sending the SYN packet.
  - Waiting (up to 5 seconds) for a valid SYN-ACK.
  - Sending the final ACK if the SYN-ACK is correct.
  - Exiting with success or an error message if the handshake fails.

## Code Flow

1. **Socket Initialization**:  
   A raw socket is created and the `IP_HDRINCL` option is enabled to allow custom IP header usage.

2. **Sending SYN**:  
   The client sends a SYN packet with sequence number 200.

3. **Waiting for SYN-ACK**:  
   The client enters a loop that uses `select()` with a timeout of 5 seconds.
   For each packet received, it checks that:

   - Both SYN and ACK flags are set.
   - The acknowledgment number is 201 (i.e., 200 + 1).

4. **Sending Final ACK**:  
   If a valid SYN-ACK is received, the client sends a final ACK with:

   - Sequence number 600
   - Acknowledgment number 401 (i.e., 400 + 1)

5. **Completion**:  
   If the expected values are observed, the handshake completes successfully. Otherwise, the handshake times out or fails with an error message.

## Expected Output

### Successful Handshake (Correct Sequence Numbers)

**Server Output**

```
[+] Server listening on port 12345...
[+] TCP Flags:  SYN: 1 ACK: 0 FIN: 0 RST: 0 PSH: 0 SEQ: 200
[+] Received SYN from 127.0.0.1
[+] Sent SYN-ACK
```

**Client Output**

```
[+] Sent SYN packet with sequence 200
[DEBUG] Received packet flags: SYN=1 ACK=1 Seq=400 Ack=201
[+] Received valid SYN-ACK from server.
[+] Sent ACK packet with sequence 600
[+] Handshake complete.
```

### Incorrect Handshake (Wrong SYN Sequence Number)

```
[-] Timeout or invalid SYN-ACK received. Handshake failed.
```

Note: Initially, the client will always complete the handshake successfully due to hardcoded values. However, if sequence numbers are changed, the handshake will fail as expected.

## How to Compile and Run

1. **Compilation**

Using Makefile:

```
make
```

Or manually:

```
g++ client.cpp -o client
g++ server.cpp -o server -lpthread
```

2. **Running**

Start the server:

```
sudo ./server
```

Run the client (in a separate terminal):

```
sudo ./client
```

Note: Root privileges (`sudo`) are required due to the use of raw sockets.

3. **Handshake Behavior**

The client’s SYN and final ACK packets are constructed with hardcoded sequence numbers. This will work only as long as the server expects the same values. If modified, the client may not receive a valid SYN-ACK or will fail the final ACK validation, causing the handshake to abort after timeout.

## Testing

- The program was tested with both valid and invalid sequence numbers.
- Proper debug messages were added to help identify handshake progression.
- The client-side timeout mechanism was verified using `select()`.

## Challenges and Solutions

- **Raw Socket Privileges**: Required root access to execute.
- **Packet Validation**: Ensured correct parsing of TCP headers and flag bits.
- **Timeout Handling**: Prevented indefinite wait by implementing `select()`.

## Contribution of Each Member

| Team Member               | Contribution (%) | Responsibilities                                                                |
| ------------------------- | ---------------- | ------------------------------------------------------------------------------- |
| Nilay Agarwal (220714)    | 50%              | Client architecture, handshake implementation, sequence number logic, debugging |
| Aarsh Walavalkar (220013) | 50%              | Timeout logic, packet validation, testing, documentation and README preparation |

## References

- CS425 Lecture Slides and Notes
- "TCP/IP Illustrated" by W. Richard Stevens
- Linux man pages: `raw`, `socket`, `ip`, `tcp`, `select`

## Declaration

We hereby declare that this assignment was completed independently and no plagiarism was involved. All code and documentation are original and appropriately referenced.

## Feedback

### Suggestions for Improvement

- Additional guidance on platform-specific nuances (e.g., behavior on macOS vs. Linux) would be helpful.
