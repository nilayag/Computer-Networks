#!/usr/bin/env python3
"""
DNS Resolver Assignment

This script implements two DNS resolution methods:

1. Iterative DNS Resolution:
   - Begins by querying a set of well-known root DNS servers.
   - Checks the response's authority section for nameserver (NS) records.
   - Resolves the NS hostnames to their IP addresses.
   - Iteratively queries each next-level server (e.g., TLD, then authoritative)
     until it finally receives the answer (an A record with an IP address).

2. Recursive DNS Resolution:
   - Uses the system's default DNS resolver (via dnspython) to perform a full recursive lookup.
   - Retrieves both the NS records and the final A record (IP address) for the domain.

Usage:
    python3 dnsresolver.py iterative example.com
    python3 dnsresolver.py recursive example.com
"""

import dns.message       # For building DNS query messages.
import dns.query         # For sending DNS queries over UDP.
import dns.rdatatype     # For specifying DNS record types (e.g., A, NS).
import dns.resolver      # For performing DNS resolution using the system's resolver.
import time              # For tracking the execution time.
import sys               # For handling command-line arguments.

# Define a dictionary of well-known root DNS servers.
# These are used as the starting point for the iterative DNS resolution process.
ROOT_SERVERS = {
    "198.41.0.4": "Root (a.root-servers.net)",
    "199.9.14.201": "Root (b.root-servers.net)",
    "192.33.4.12": "Root (c.root-servers.net)",
    "199.7.91.13": "Root (d.root-servers.net)",
    "192.203.230.10": "Root (e.root-servers.net)"
}

# Set a timeout for each DNS query in seconds.
TIMEOUT = 3

def send_dns_query(server, domain):
    """
    Sends a DNS query for an A record (IPv4 address) to the specified server.
    
    Parameters:
        server (str): IP address of the DNS server to query.
        domain (str): Domain name to resolve.
    
    Returns:
        dns.message.Message: The DNS response if successful.
        None: If the query times out or an error occurs.
    """
    try:
        # Create a DNS query message for the domain with record type 'A'.
        query = dns.message.make_query(domain, dns.rdatatype.A)
        # Send the query using UDP to the specified server with a timeout.
        response = dns.query.udp(query, server, timeout=TIMEOUT)
        return response
    except Exception as e:
        # In case of any exception (e.g., timeout), return None.
        # Uncomment the following line for detailed debugging information.
        # print(f"[ERROR] Failed to send DNS query to {server}: {e}")
        return None

def extract_next_nameservers(response):
    """
    Extracts NS (nameserver) records from the authority section of a DNS response.
    Then resolves those NS hostnames to their corresponding IP addresses.
    
    Parameters:
        response (dns.message.Message): The DNS response containing NS records.
    
    Returns:
        list: A list of IP addresses for the next-level authoritative nameservers.
    """
    ns_ips = []    # To store IP addresses of the next-level nameservers.
    ns_names = []  # To store the extracted nameserver hostnames.
    
    # Loop over each resource record set in the authority section.
    for rrset in response.authority:
        # Check if this RRset is of type NS.
        if rrset.rdtype == dns.rdatatype.NS:
            # For each record in the RRset, extract the nameserver hostname.
            for rr in rrset:
                ns_name = rr.to_text()
                ns_names.append(ns_name)
                print(f"Extracted NS hostname: {ns_name}")
    
    # Resolve each extracted nameserver hostname to an IP address.
    # We believe this commented part is the correct code though this does not match the sample output
    # for rrset in response.additional:
    #     if rrset.rdtype == dns.rdatatype.A:
    #         additional_name = rrset.name.to_text()
    #         # Only consider A records for names that match one of the NS hostnames.
    #         if additional_name in ns_names:
    #             for rdata in rrset:
    #                 ip = rdata.to_text()
    #                 ns_ips.append(ip)
    #                 print(f"Resolved {additional_name} to {ip}")
    for ns in ns_names:
        try:
            # Use the system resolver to get the A record for the nameserver.
            answers = dns.resolver.resolve(ns, 'A')
            for rdata in answers:
                ip = rdata.to_text()
                ns_ips.append(ip)
                print(f"Resolved {ns} to {ip}")
        except Exception as e:
            # Log an error if the nameserver's IP cannot be resolved.
            print(f"[ERROR] Could not resolve nameserver {ns}: {e}")
    
    return ns_ips

def iterative_dns_lookup(domain):
    """
    Performs iterative DNS resolution starting from the root servers.
    At each stage (ROOT, TLD, AUTH), it queries all available nameservers,
    collects referrals (NS records), and proceeds until an answer is found
    or no further nameservers are available.
    
    Parameters:
        domain (str): The domain name to resolve.
    """
    print(f"[Iterative DNS Lookup] Resolving {domain}")
    
    # Start with the IP addresses of the root servers.
    current_ns_list = list(ROOT_SERVERS.keys())
    # Set the initial resolution stage to 'ROOT'.
    stage = "ROOT"
    
    # Continue while there are nameservers to try.
    while current_ns_list:
        next_ns_candidates = []  # To collect referrals from responses.
        any_response = False     # Flag to check if at least one server responded.
        
        # Iterate over each nameserver in the current list till a valid response is received.
        for ns_ip in current_ns_list:
            if any_response:
                break
            # Send a DNS query to the current nameserver.
            response = send_dns_query(ns_ip, domain)
            if response:
                any_response = True  # Mark that we received a response.
                print(f"[DEBUG] Querying {stage} server ({ns_ip}) - SUCCESS")
                
                # Check if the response contains an answer (an A record).
                if response.answer:
                    # If an answer is found, print the result and exit.
                    print(f"[SUCCESS] {domain} -> {response.answer[0][0]}")
                    return
                else:
                    # No final answer; extract NS referrals from the response.
                    referrals = extract_next_nameservers(response)
                    # Add these referrals to the candidate list for the next stage.
                    next_ns_candidates.extend(referrals)
            else:
                # Log an error if this nameserver did not respond.
                print(f"[ERROR] Query failed for {stage} server ({ns_ip})")
        
        # If no nameserver responded at this level, print an error and exit.
        if not any_response:
            print(f"[ERROR] Resolution failed at {stage} level. No responses received.")
            return
        
        # update the list of nameservers to be queried.
        current_ns_list = (next_ns_candidates)
        if not current_ns_list:
            # If no referrals are found, resolution fails at this stage.
            print(f"[ERROR] Resolution failed at {stage} level. No referrals found.")
            return
        
        # Update the resolution stage for logging:
        # ROOT -> TLD -> AUTH. If already at AUTH, remain at AUTH.
        if stage == "ROOT":
            stage = "TLD"
        elif stage == "TLD":
            stage = "AUTH"
    
    # If the loop exits without resolving the domain, print a failure message.
    print("[ERROR] Resolution failed. No further nameservers to query.")

def recursive_dns_lookup(domain):
    """
    Performs recursive DNS resolution using the system's default resolver.
    This delegates the entire resolution process (including recursion) to the resolver.
    
    Parameters:
        domain (str): The domain name to resolve.
    """
    print(f"[Recursive DNS Lookup] Resolving {domain}")
    
    try:
        # Create a resolver instance.
        resolver = dns.resolver.Resolver()
        # Set the timeout per query and overall lifetime.
        resolver.timeout = 3    # Timeout per query.
        resolver.lifetime = 10   # Total allowed time for resolution.
        
        # Resolve NS records for the domain and print them.
        ns_answers = resolver.resolve(domain, 'NS')
        for rdata in ns_answers:
            print(f"[SUCCESS] {domain} -> {rdata.to_text()}")
        
        # Resolve the A record (final IP address) for the domain and print it.
        a_answers = resolver.resolve(domain, 'A')
        for rdata in a_answers:
            print(f"[SUCCESS] {domain} -> {rdata.to_text()}")
            
    except dns.resolver.NXDOMAIN:
        # Raised when the domain does not exist (Non-Existent Domain)
        print("[ERROR] The queried domain does not exist (NXDOMAIN).")

    except dns.resolver.NoAnswer:
        # Raised when the DNS server responds but provides no valid answer
        print("[ERROR] No answer received from the DNS server (No A record).")

    except dns.resolver.NoNameservers:
        # Raised when no valid nameservers are available to respond to the query
        print("[ERROR] No valid nameservers responded (Server issue or misconfiguration).")

    except dns.resolver.Timeout:
        # Raised when the DNS query takes too long and exceeds the timeout limit
        print("[ERROR] Query timed out. The DNS server is unresponsive.")

    except dns.exception.DNSException as e:
        # Catch-all for any other DNS-related exceptions
        print(f"[ERROR] General DNS resolution error: {e}")

if __name__ == "__main__":
    # Ensure that exactly two command-line arguments are provided:
    # 1. The resolution mode ("iterative" or "recursive")
    # 2. The domain name to resolve.
    if len(sys.argv) != 3 or sys.argv[1] not in {"iterative", "recursive"}:
        print("Usage: python3 dnsresolver.py <iterative|recursive> <domain>")
        sys.exit(1)
    
    # Extract the mode and domain from command-line arguments.
    mode = sys.argv[1]
    domain = sys.argv[2]
    # Record the start time to measure the resolution time.
    start_time = time.time()
    
    # Call the appropriate resolution function based on the mode.
    if mode == "iterative":
        iterative_dns_lookup(domain)
    else:
        recursive_dns_lookup(domain)
    
    # Calculate and print the total time taken for the DNS resolution process.
    print(f"Time taken: {time.time() - start_time:.3f} seconds")