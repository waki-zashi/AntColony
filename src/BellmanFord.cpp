#include "BellmanFord.h"
#include "FileReader.h"

#include <iostream>
#include <chrono>
#include <algorithm>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

BellmanFordSolver::BellmanFordSolver(const vector<vector<double>>& g,
                                     const vector<string>& names,
                                     int s,
                                     int e)
    : graph(g), labels(names), start(s), end(e) {}

string BellmanFordSolver::buildPathLabels(const vector<int>& path) const {
    string result;

    for (size_t i = 0; i < path.size(); ++i) {
        result += labels[path[i]];
        if (i + 1 < path.size()) {
            result += " -> ";
        }
    }

    return result;
}

BellmanFordResult BellmanFordSolver::run() {
    BellmanFordResult result;

    const int n = static_cast<int>(graph.size());
    if (n == 0 || start < 0 || end < 0 || start >= n || end >= n) {
        return result;
    }

    const double INF = numeric_limits<double>::max();
    vector<double> dist(n, INF);
    vector<int> parent(n, -1);

    dist[start] = 0.0;

    struct Edge {
        int u;
        int v;
        double w;
    };

    vector<Edge> edges;
    edges.reserve(n * n);

    for (int u = 0; u < n; ++u) {
        for (int v = u + 1; v < n; ++v) {
            if (graph[u][v] > 0.0) {
                edges.push_back({u, v, graph[u][v]});
                edges.push_back({v, u, graph[u][v]});
            }
        }
    }

    int performedIterations = 0;

    for (int i = 0; i < n - 1; ++i) {
        bool updated = false;
        performedIterations++;

        for (const auto& edge : edges) {
            if (dist[edge.u] == INF) {
                continue;
            }

            const double candidate = dist[edge.u] + edge.w;
            if (candidate < dist[edge.v]) {
                dist[edge.v] = candidate;
                parent[edge.v] = edge.u;
                updated = true;
            }
        }

        if (!updated) {
            break;
        }
    }

    result.iterations = performedIterations;

    for (const auto& edge : edges) {
        if (dist[edge.u] == INF) {
            continue;
        }

        if (dist[edge.u] + edge.w < dist[edge.v]) {
            result.negativeCycleDetected = true;
            break;
        }
    }

    if (result.negativeCycleDetected) {
        return result;
    }

    if (dist[end] == INF) {
        return result;
    }

    vector<int> path;
    for (int v = end; v != -1; v = parent[v]) {
        path.push_back(v);
    }
    reverse(path.begin(), path.end());

    if (path.empty() || path.front() != start) {
        return result;
    }

    result.bestPath = path;
    result.bestLength = dist[end];
    result.pathFound = true;
    result.bestPathLabels = buildPathLabels(path);

    return result;
}

string BellmanFordTestRunner::getDefaultOutputFile() const {
    return "results/bellman_ford_results.csv";
}

void BellmanFordTestRunner::runTestSuite(const string& testDirectory) {
    clearResults();

    cout << "=== Bellman-Ford Algorithm Test Suite ===" << endl;
    cout << "Looking for test files in: " << testDirectory << endl;

    vector<string> testFiles = readTestFilesList(testDirectory);

    if (testFiles.empty()) {
        cout << "ERROR: No test files list found or list is empty!" << endl;
        cout << "Please run generate_test_suite first to create test graphs." << endl;
        return;
    }

    cout << "Found " << testFiles.size() << " test files in the list." << endl;

    int testCount = 0;
    const int maxTests = min(100, static_cast<int>(testFiles.size()));

    for (size_t i = 0; i < testFiles.size() && testCount < maxTests; i++) {
        const string filename = testFiles[i];
        const string fullPath = testDirectory + "/" + filename;

        if (!fileExists(fullPath)) {
            cout << "Warning: File from list not found: " << fullPath << endl;
            continue;
        }

        string testName = filename;
        const size_t dotPos = testName.find_last_of(".");
        if (dotPos != string::npos) {
            testName = testName.substr(0, dotPos);
        }

        cout << "[" << (testCount + 1) << "] Running: " << testName << endl;
        runSingleTest(fullPath, testName);
        testCount++;
    }

    if (testCount == 0) {
        cout << "ERROR: No valid test files found!" << endl;
        cout << "Files from list exist but cannot be loaded." << endl;
        return;
    }

    cout << "\n=== Completed " << testCount << " tests ===" << endl;
    printSummary();
    saveResultsToCSV(getDefaultOutputFile());
}

void BellmanFordTestRunner::runSingleTest(const string& graphFile, const string& testName) {
    bool fileLoaded = false;
    vector<vector<double>> graph;
    vector<string> labels;
    int start = -1;
    int end = -1;

    readGraphFromFile(graphFile, fileLoaded, graph, labels, start, end);

    if (!fileLoaded || graph.empty()) {
        cerr << "  Failed to load graph: " << graphFile << endl;
        return;
    }

    const int n = static_cast<int>(labels.size());
    if (n == 0) {
        cerr << "  Empty graph: " << graphFile << endl;
        return;
    }

    cout << "  Path: " << labels[start] << " -> " << labels[end];
    cout << " (vertices: " << n << ", edges: " << countEdges(graph) << ")" << endl;

    BellmanFordSolver solver(graph, labels, start, end);

    auto startTime = chrono::high_resolution_clock::now();
    BellmanFordResult result = solver.run();
    auto endTime = chrono::high_resolution_clock::now();

    const double executionTime = chrono::duration<double>(endTime - startTime).count();

    string pathSequence = "NO_PATH";
    if (result.pathFound && !result.bestPath.empty()) {
        pathSequence.clear();
        for (size_t i = 0; i < result.bestPath.size(); i++) {
            pathSequence += labels[result.bestPath[i]];
            if (i < result.bestPath.size() - 1) {
                pathSequence += "->";
            }
        }
    }

    TestResult testResult;
    testResult.testName = testName;
    testResult.executionTime = executionTime;
    testResult.bestPathLength = result.bestLength;
    testResult.vertices = n;
    testResult.edges = countEdges(graph);
    testResult.foundPath = result.pathFound;
    testResult.iterations = result.iterations;
    testResult.bestPathSequence = pathSequence;

    results.push_back(testResult);

    cout << "  Result: time=" << executionTime << "s, length=";
    if (result.pathFound) {
        cout << result.bestLength;
    } else {
        cout << "NO_PATH";
    }

    cout << ", found=" << (result.pathFound ? "yes" : "no")
         << ", iterations=" << result.iterations;

    if (result.negativeCycleDetected) {
        cout << ", negative_cycle_detected=yes";
    }

    cout << endl;
}