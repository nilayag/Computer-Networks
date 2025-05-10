# CS425 Assignment 4: Routing Algorithms Simulation

## Overview

This project simulates two fundamental network routing algorithms: Distance Vector Routing (DVR) and Link State Routing (LSR). Implemented in C++, the program processes a network topology provided as an adjacency matrix and generates routing tables for all nodes using both DVR and LSR methods.

## Features Implemented

- Simulation of Distance Vector Routing using the Bellman-Ford algorithm.
- Simulation of Link State Routing using Dijkstra’s algorithm.
- File-based input parsing for adjacency matrix representation of the network graph.
- Generation and display of routing tables after each algorithm completes.

## How to Compile and Run

### Compilation

To compile the project, run the following command in the terminal:

```
make
```

This will create an executable named `routing_sim`.

### Execution

To run the executable, use the following command:

```
./routing_sim inputfile.txt
```

Where `inputfile.txt` should be a text file containing the network adjacency matrix.

### Clean-Up

To remove the compiled executable, run:

```
make clean
```

## High-Level Description of Key Functions

| Function Name       | Description                                                        |
| ------------------- | ------------------------------------------------------------------ |
| `readGraphFromFile` | Parses the input file to construct the graph's adjacency matrix    |
| `simulateDVR`       | Simulates Distance Vector Routing using the Bellman-Ford algorithm |
| `simulateLSR`       | Simulates Link State Routing using Dijkstra’s algorithm            |
| `printDVRTable`     | Displays routing tables resulting from the DVR simulation          |
| `printLSRTable`     | Displays routing tables resulting from the LSR simulation          |
| `main`              | Drives the overall flow of the program                             |

## Code Flow

1. The program begins execution in the `main` function.
2. The function `readGraphFromFile` is used to parse the adjacency matrix from the input file.
3. The `simulateDVR` function runs the Distance Vector algorithm iteratively until convergence.
4. Routing tables for all nodes are printed using `printDVRTable`.
5. The `simulateLSR` function executes Dijkstra’s algorithm for each node to determine shortest paths.
6. The output from LSR is displayed using `printLSRTable`.

## Individual Contributions

| Member Name             | Roll Number | Contributions (50% each)                                                                                  |
| ----------------------- | ----------- | --------------------------------------------------------------------------------------------------------- |
| Nilay Agarwal           | 220714      | Implemented the Link State Routing algorithm, Dijkstra’s logic, and structured program control flow       |
| Aarsh Ashish Walavalkar | 220013      | Implemented the Distance Vector Routing algorithm, file parsing, and output formatting for routing tables |

## Sources

- "Computer Networking: A Top-Down Approach" by Kurose and Ross
- GeeksforGeeks tutorials on Bellman-Ford and Dijkstra algorithms
- C++ Standard Library Documentation (cppreference.com)
- CS425 Lecture Slides and Assignment Prompt

## Declaration

We declare that the work submitted in this assignment is our own. We have not engaged in any form of plagiarism, and all external sources have been properly cited. We understand and uphold the values of academic integrity and are aware of the consequences of violating these policies.
