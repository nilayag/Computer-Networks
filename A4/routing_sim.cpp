#include <iostream>
#include <vector>
#include <limits>
#include <queue>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

// Constant representing an unreachable cost
const int INF = 9999;

/*
 * Function: printDVRTable
 * -----------------------
 * Prints the routing table for a given node using the computed cost (distance) and next hop arrays.
 */
void printDVRTable(int node, const vector<vector<int>> &table, const vector<vector<int>> &nextHop)
{
    cout << "Node " << node << " Routing Table:\n";
    cout << "Dest\tCost\tNext Hop\n";
    for (int i = 0; i < table.size(); ++i)
    {
        cout << i << "\t" << table[node][i] << "\t";
        if (nextHop[node][i] == -1)
            cout << "-";
        else
            cout << nextHop[node][i];
        cout << "\n";
    }
    cout << "\n";
}

/*
 * Function: simulateDVR
 * ---------------------
 * Simulates the Distance Vector Routing algorithm using an iterative update process.
 */
void simulateDVR(const vector<vector<int>> &graph)
{
    int n = graph.size();
    vector<vector<int>> dist = graph;
    vector<vector<int>> nextHop(n, vector<int>(n, -1));

    // initialize nextHop: direct neighbors
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            if (i == j)
                nextHop[i][j] = -1;
            else if (graph[i][j] != INF)
                nextHop[i][j] = j;
        }
    }

    bool updated = true;
    while (updated)
    {
        updated = false;
        for (int i = 0; i < n; ++i)
        {
            for (int j = 0; j < n; ++j)
            {
                if (i == j || graph[i][j] == INF)
                    continue;
                for (int k = 0; k < n; ++k)
                {
                    if (dist[j][k] == INF)
                        continue;
                    int newCost = dist[i][j] + dist[j][k];
                    if (newCost < dist[i][k])
                    {
                        dist[i][k] = newCost;
                        // corrected next-hop update: always route via j
                        nextHop[i][k] = nextHop[i][j];
                        updated = true;
                    }
                }
            }
        }
    }

    cout << "--- DVR Final Tables ---\n";
    for (int i = 0; i < n; ++i)
    {
        printDVRTable(i, dist, nextHop);
    }
}

/*
 * Function: printLSRTable
 * -----------------------
 * Prints the routing table for a given source node after running Dijkstra's algorithm.
 */
void printLSRTable(int src, const vector<int> &dist, const vector<int> &prev)
{
    cout << "Node " << src << " Routing Table:\n";
    cout << "Dest\tCost\tNext Hop\n";
    for (int i = 0; i < dist.size(); ++i)
    {
        if (i == src)
            continue;
        cout << i << "\t" << dist[i] << "\t";
        int hop = i;
        while (prev[hop] != src && prev[hop] != -1)
        {
            hop = prev[hop];
        }
        cout << (prev[hop] == -1 ? -1 : hop) << "\n";
    }
    cout << "\n";
}

/*
 * Function: simulateLSR
 * ---------------------
 * Simulates the Link State Routing algorithm by running Dijkstra's algorithm for every node.
 */
void simulateLSR(const vector<vector<int>> &graph)
{
    int n = graph.size();
    for (int src = 0; src < n; ++src)
    {
        vector<int> dist(n, INF), prev(n, -1);
        vector<bool> visited(n, false);
        dist[src] = 0;

        for (int i = 0; i < n; ++i)
        {
            int u = -1;
            for (int j = 0; j < n; ++j)
            {
                if (!visited[j] && (u == -1 || dist[j] < dist[u]))
                {
                    u = j;
                }
            }
            if (u == -1 || dist[u] == INF)
                break;
            visited[u] = true;
            for (int v = 0; v < n; ++v)
            {
                if (graph[u][v] != INF && !visited[v])
                {
                    int newCost = dist[u] + graph[u][v];
                    if (newCost < dist[v])
                    {
                        dist[v] = newCost;
                        prev[v] = u;
                    }
                }
            }
        }

        printLSRTable(src, dist, prev);
    }
}

/*
 * Function: readGraphFromFile
 * ---------------------------
 * Reads the adjacency matrix from a file.
 */
vector<vector<int>> readGraphFromFile(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Error: Could not open file " << filename << "\n";
        exit(1);
    }
    int n;
    file >> n;
    vector<vector<int>> graph(n, vector<int>(n));
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            file >> graph[i][j];
        }
    }
    file.close();
    return graph;
}

/*
 * Function: main
 * --------------
 * Entry point: reads input, runs DVR and LSR simulations, and prints results.
 */
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }
    vector<vector<int>> graph = readGraphFromFile(argv[1]);
    cout << "\n--- Distance Vector Routing Simulation ---\n";
    simulateDVR(graph);
    cout << "\n--- Link State Routing Simulation ---\n";
    simulateLSR(graph);
    return 0;
}
