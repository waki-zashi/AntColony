#include "GraphGenerator.h"

#include <fstream>
#include <queue>
#include <stdexcept>
#include <filesystem>
#include <algorithm>
#include <random>

using namespace std;

GraphGenerator::GraphGenerator(unsigned int seed)
    : gen(seed) {}

double GraphGenerator::randomWeight(double minW, double maxW) {
    uniform_real_distribution<double> dist(minW, maxW);
    return dist(gen);
}

void GraphGenerator::addUndirectedEdge(vector<vector<double>>& graph, int u, int v, double w) {
    if (u == v) return;
    graph[u][v] = w;
    graph[v][u] = w;
}

vector<vector<double>> GraphGenerator::generateConnectedRandomGraph(int n, double density) {
    if (n <= 0) {
        return {};
    }

    if (density < 0.0) density = 0.0;
    if (density > 1.0) density = 1.0;

    vector<vector<double>> graph(n, vector<double>(n, 0.0));

    for (int i = 1; i < n; ++i) {
        uniform_int_distribution<int> parentDist(0, i - 1);
        int parent = parentDist(gen);
        addUndirectedEdge(graph, i, parent, randomWeight());
    }

    const int maxEdges = n * (n - 1) / 2;
    const int minEdges = n - 1;
    int targetEdges = static_cast<int>(density * maxEdges);

    if (targetEdges < minEdges) {
        targetEdges = minEdges;
    }
    if (targetEdges > maxEdges) {
        targetEdges = maxEdges;
    }

    int currentEdges = minEdges;

    vector<pair<int, int>> missingEdges;
    missingEdges.reserve(maxEdges);

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (graph[i][j] == 0.0) {
                missingEdges.push_back({i, j});
            }
        }
    }

    std::shuffle(missingEdges.begin(), missingEdges.end(), gen);

    for (const auto& [u, v] : missingEdges) {
        if (currentEdges >= targetEdges) {
            break;
        }
        addUndirectedEdge(graph, u, v, randomWeight());
        currentEdges++;
    }

    return graph;
}

vector<vector<double>> GraphGenerator::generateStarGraph(int n) {
    if (n <= 0) {
        return {};
    }

    vector<vector<double>> graph(n, vector<double>(n, 0.0));
    const int center = 0;

    for (int i = 1; i < n; ++i) {
        addUndirectedEdge(graph, center, i, randomWeight());
    }

    return graph;
}

vector<vector<double>> GraphGenerator::generatePathGraph(int n) {
    if (n <= 0) {
        return {};
    }

    vector<vector<double>> graph(n, vector<double>(n, 0.0));

    for (int i = 0; i + 1 < n; ++i) {
        addUndirectedEdge(graph, i, i + 1, randomWeight());
    }

    return graph;
}

vector<vector<double>> GraphGenerator::generateGridGraph(int rows, int cols) {
    if (rows <= 0 || cols <= 0) {
        return {};
    }

    const int n = rows * cols;
    vector<vector<double>> graph(n, vector<double>(n, 0.0));

    auto id = [cols](int r, int c) {
        return r * cols + c;
    };

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (r + 1 < rows) {
                addUndirectedEdge(graph, id(r, c), id(r + 1, c), randomWeight());
            }
            if (c + 1 < cols) {
                addUndirectedEdge(graph, id(r, c), id(r, c + 1), randomWeight());
            }
        }
    }

    return graph;
}

vector<vector<double>> GraphGenerator::generateCompleteGraph(int n) {
    if (n <= 0) {
        return {};
    }

    vector<vector<double>> graph(n, vector<double>(n, 0.0));

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            addUndirectedEdge(graph, i, j, randomWeight());
        }
    }

    return graph;
}

vector<string> GraphGenerator::generateLabels(int n) {
    vector<string> labels;
    labels.reserve(n);

    for (int i = 0; i < n; ++i) {
        if (i < 26) {
            labels.push_back(string(1, static_cast<char>('A' + i)));
        } else {
            labels.push_back("V" + to_string(i + 1));
        }
    }

    return labels;
}

bool GraphGenerator::isConnected(const vector<vector<double>>& graph) const {
    const int n = static_cast<int>(graph.size());
    if (n == 0) {
        return true;
    }

    vector<bool> visited(n, false);
    queue<int> q;

    visited[0] = true;
    q.push(0);

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        for (int v = 0; v < n; ++v) {
            if (graph[u][v] > 0.0 && !visited[v]) {
                visited[v] = true;
                q.push(v);
            }
        }
    }

    for (bool seen : visited) {
        if (!seen) {
            return false;
        }
    }

    return true;
}

void GraphGenerator::saveGraphToFile(const vector<vector<double>>& graph,
                                     const vector<string>& labels,
                                     const string& filename) {
    namespace fs = std::filesystem;

    const int n = static_cast<int>(graph.size());
    if (n == 0 || static_cast<int>(labels.size()) != n) {
        throw runtime_error("Invalid graph or labels in saveGraphToFile()");
    }

    fs::create_directories(fs::path(filename).parent_path());

    ofstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Cannot open file for writing: " + filename);
    }

    file << ",";
    for (int i = 0; i < n; ++i) {
        file << labels[i];
        if (i + 1 < n) file << ",";
    }
    file << "\n";

    for (int i = 0; i < n; ++i) {
        file << labels[i];
        for (int j = 0; j < n; ++j) {
            file << "," << graph[i][j];
        }
        file << "\n";
    }

    uniform_int_distribution<int> dist(0, n - 1);
    int start = dist(gen);
    int end = dist(gen);
    while (end == start && n > 1) {
        end = dist(gen);
    }

    file << start << "," << end << "\n";
}