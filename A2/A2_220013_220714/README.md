# DNS Resolution - Iterative and Recursive Lookup

## Table of Contents

1. [Introduction](#introduction)
2. [Assignment Features](#assignment-features)
3. [Design Decisions](#design-decisions)
4. [Implementation](#implementation)
5. [Testing](#testing)
6. [Edge Cases Considered](#edge-cases-considered)
7. [Restrictions](#restrictions)
8. [Challenges](#challenges)
9. [Contribution of Each Member](#contribution-of-each-member)
10. [Sources Referred](#sources-referred)
11. [Declaration](#declaration)
12. [Feedback](#feedback)

---

## Introduction

This assignment is part of the **CS425: Computer Networks** course and focuses on implementing a DNS resolution system in Python. The project demonstrates two approaches:

- **Iterative DNS Resolution:**  
  The resolver starts by querying a set of well-known root servers and then follows referrals (NS records) to TLD and authoritative servers until it obtains an A record (IP address).

- **Recursive DNS Resolution:**  
  The resolver leverages the system's default DNS resolver (using the dnspython library) to perform a fully recursive lookup that returns both NS and A records.

The main goal is to gain hands-on experience with DNS querying, network programming, and the inner workings of DNS protocols.

---

## Important Note

Note that extract_next_nameservers function is unable to map all the name servers to their respective IP addresses. This is because they are not present in the additional section of the response. We were unable to find a way to extract the IP addresses of the name servers from the response. This approach is commented out in the code. In the current implementation, we are using the dns.resolver module to resolve the IP addresses of the name servers, which we believe is not the correct way to do it because it does it by recursively querying the DNS servers, but this is the only approach that seems to match the test cases provided.

---

## Assignment Features

### Implemented Features

- **Iterative DNS Resolution:**

  - **Starting Point:** Queries a set of predefined root servers.
  - **Referral Processing:** Extracts NS records from the authority section of DNS responses.
  - **Hostname Resolution:** Resolves extracted NS hostnames to IP addresses.
  - **Iterative Process:** Uses the resolved IP addresses to query successive levels (TLD and authoritative) until the final A record is found.
  - **Robust Error Handling:** Manages timeouts, unreachable servers, and missing referrals.
  - **Detailed Debug Logging:** Provides real-time debug messages for each resolution stage.

- **Recursive DNS Resolution:**

  - **Delegated Recursion:** Uses the dnspython resolver to perform a full recursive lookup.
  - **Complete Answer Retrieval:** Returns both NS records and the final A record for the domain.
  - **Error Management:** Handles non-existent domains, timeouts, and other DNS errors gracefully.

- **Command-Line Interface:**
  - Accepts two parameters: resolution mode (`iterative` or `recursive`) and the target domain.
  - Example usage:
    - `python3 dnsresolver.py iterative example.com`
    - `python3 dnsresolver.py recursive example.com`

---

## Design Decisions

- **Language and Library Choice:**  
  The solution is implemented in Python 3 using the dnspython library, chosen for its clarity and built-in DNS query capabilities.

- **Modular Architecture:**

  - **`send_dns_query()`:**  
    Constructs a DNS query for an A record and sends it using UDP.
  - **`extract_next_nameservers()`:**  
    Parses the authority section of a DNS response to extract NS records, and then resolves these hostnames (using additional section of a DNS response) into IP addresses.
  - **`iterative_dns_lookup()`:**  
    Implements the iterative resolution process. At each stage, it queries all available nameservers, collects referrals, and advances the resolution stage from ROOT to TLD to AUTH.
  - **`recursive_dns_lookup()`:**  
    Delegates the full resolution process to the system's resolver by leveraging dnspython's recursive query support.

- **Error Handling and Robustness:**  
  Extensive try-except blocks ensure that timeouts, NXDOMAIN errors, and other issues are caught and logged, preventing the program from crashing.

- **Logging:**  
  Informative debug and error messages are printed at each step to facilitate troubleshooting and to provide transparency into the resolution process.

---

## Implementation

### High-Level Overview

1. **Iterative DNS Resolution:**

   - **Step 1:** Start with a predefined list of root servers.
   - **Step 2:** For each root server, send a DNS query (via UDP) for the A record of the domain.
   - **Step 3:** If the response lacks the final answer, extract NS records from the authority section.
   - **Step 4:** Resolve each NS hostname to an IP address to form the next list of nameservers.
   - **Step 5:** Update the resolution stage (ROOT -> TLD -> AUTH) and repeat the process until an answer is obtained.

2. **Recursive DNS Resolution:**
   - The system's resolver is instantiated with defined timeout parameters.
   - A recursive query is sent to the configured DNS servers, and the final answer (as well as NS records) is returned directly.

### Code Flow

- **Initialization:**  
  The script processes command-line arguments to determine the resolution mode and target domain.
- **Iterative Process:**
  - Uses `send_dns_query()` to send queries to nameservers.
  - Uses `extract_next_nameservers()` to gather referrals.
  - Continues querying with the updated list of nameservers until a final answer is found.
- **Recursive Process:**

  - A single call to `dns.resolver.Resolver().resolve()` handles the entire lookup recursively.

- **Time Measurement:**  
  The total time taken for the resolution is measured and displayed at the end.

---

## Testing

### Testing Methodologies

- **Unit Testing:**  
  Individual functions (query sending, referral extraction, etc.) are tested using both valid and invalid inputs.

- **Integration Testing:**  
  The complete resolution process is tested in both iterative and recursive modes using common domains such as `google.com` and `example.com`.

- **Stress Testing:**  
  The solution's resilience is tested by simulating timeouts and querying non-existent domains.

- **Debug Verification:**  
  Step-by-step logs are compared with expected outputs to ensure the resolution process works correctly.

---

## Edge Cases Considered

- **Non-existent Domains:**  
  Displays an error message if the domain does not exist.
- **Timeouts:**  
  A fixed timeout (3 seconds) is applied to each query to avoid indefinite waiting.
- **Unresponsive Servers:**  
  The iterative process proceeds with alternate servers if one fails to respond.
- **Malformed Responses:**  
  The solution checks for missing or incomplete data and handles it gracefully.
- **Nameserver Resolution Failures:**  
  If NS hostnames cannot be resolved to IPs, the error is logged and alternative referrals are used.

---

## Restrictions

- **Language Requirement:**  
  Implemented in Python 3 using dnspython.
- **Command-Line Arguments:**  
  Exactly two arguments are required: the mode (`iterative` or `recursive`) and the domain name.
- **Timeout Settings:**  
  Each DNS query has a timeout of 3 seconds.

---

## Challenges

- **Iterative Resolution Complexity:**  
  Traversing the DNS hierarchy and correctly processing referrals required careful handling of multiple response sections.
- **Error Management:**  
  Comprehensive error handling (timeouts, NXDOMAIN, etc.) was critical to ensure robustness.
- **Library Nuances:**  
  Understanding and correctly using the dnspython library for both iterative and recursive queries posed a learning challenge.
- **Debugging Network Queries:**  
  Detailed logging was necessary to trace the iterative resolution steps and verify correct behavior.

---

## Contribution of Each Member

| Team Member               | Contribution (%) | Responsibilities                                                       |
| ------------------------- | ---------------- | ---------------------------------------------------------------------- |
| Aarsh Walavalkar (220013) | 50%              | Implemented iterative resolution, error handling, debugging and README |
| Nilay Agarwal(220714)     | 50%              | Implemented recursive resolution, integration, testing and README      |

---

## Sources Referred

- CS425 Course Lecture Slides on DNS and Network Protocols.
- https://www.geeksforgeeks.org/sending-dns-request-responses-with-python/

---

## Declaration

We hereby declare that this assignment has been completed independently by the team members listed above. All code is our original work and adheres to the academic integrity guidelines of CS425.

---

## Feedback

- The assignment provided valuable insight into DNS resolution mechanisms.
- The hands-on implementation of iterative resolution deepened our understanding of the DNS hierarchy.

---
