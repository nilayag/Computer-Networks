# Computer Networks

This repository contains a collection of 4 assignments completed as part of the CS425: Computer Networks course at IIT Kanpur under Prof. Adithya Vadapalli (Jan '25 – Apr '25). Each assignment focuses on a core networking concept, implemented from scratch using C++ and Python.

---

## 📁 Contents

### 1. Chat Server (C++)
A multi-threaded TCP chat server featuring:
- User authentication
- Private and broadcast messaging
- Dynamic group creation and management
- Thread synchronization using mutex-guarded maps

### 2. DNS Resolver (Python)
Iterative and recursive DNS resolvers implemented using `dnspython`, with:
- Full traversal from root → TLD → authoritative
- Timeout and error handling for non-responsive servers

### 3. TCP Handshake (C++)
Custom TCP client using raw sockets:
- Manual construction of SYN, SYN-ACK, and ACK packets
- Header checksum and sequence number manipulation

### 4. Routing Simulator (C++)
Simulation of:
- Distance Vector algorithm (Bellman-Ford)
- Link State algorithm (Dijkstra)
- Arbitrary graph input and convergence analysis
