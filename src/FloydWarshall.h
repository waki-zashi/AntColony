#pragma once

#include "TestRunner.h"
#include <vector>
#include <string>
#include <limits>
#include <algorithm>
#include <iostream>
#include <chrono>

using namespace std;

struct FloydWarshallResult {
    vector<int> bestPath;
    double bestLength;
    bool pathFound;

    FloydWarshallResult()
        : bestLength(numeric_limits<double>::max()), pathFound(false) {}
};

class FloydWarshall {
public:
    static FloydWarshallResult findShortestPath(const vector<vector<double>>& graph,
                                                const vector<string>& labels,
                                                int start,
                                                int end) {
        FloydWarshallResult result;

        const int n = static_cast<int>(graph.size());
        if (n == 0 || start < 0 || end < 0 || start >= n || end >= n) {
            return result;
        }

        if (start == end) {
            result.pathFound = true;
            result.bestLength = 0.0;
            result.bestPath = { start };
            return result;
        }

        const double INF = 1e100;

        vector<vector<double>> dist(n, vector<double>(n, INF));
        vector<vector<int>> next(n, vector<int>(n, -1));

        for (int i = 0; i < n; ++i) {
            dist[i][i] = 0.0;
            next[i][i] = i;

            for (int j = 0; j < n; ++j) {
                if (graph[i][j] > 0.0) {
                    dist[i][j] = graph[i][j];
                    next[i][j] = j;
                }
            }
        }

        for (int k = 0; k < n; ++k) {
            for (int i = 0; i < n; ++i) {
                if (dist[i][k] >= INF) {
                    continue;
                }

                for (int j = 0; j < n; ++j) {
                    if (dist[k][j] >= INF) {
                        continue;
                    }

                    const double throughK = dist[i][k] + dist[k][j];
                    if (throughK < dist[i][j]) {
                        dist[i][j] = throughK;
                        next[i][j] = next[i][k];
                    }
                }
            }
        }

        if (dist[start][end] >= INF || next[start][end] == -1) {
            return result;
        }

        result.pathFound = true;
        result.bestLength = dist[start][end];
        result.bestPath = reconstructPath(next, start, end);

        if (result.bestPath.empty()) {
            result.pathFound = false;
            result.bestLength = numeric_limits<double>::max();
        }

        return result;
    }

private:
    static vector<int> reconstructPath(const vector<vector<int>>& next, int start, int end) {
        if (start < 0 || end < 0 ||
            start >= static_cast<int>(next.size()) ||
            end >= static_cast<int>(next.size())) {
            return {};
        }

        if (next[start][end] == -1) {
            return {};
        }

        if (start == end) {
            return { start };
        }

        vector<int> path;
        path.push_back(start);

        int current = start;
        const int n = static_cast<int>(next.size());

        for (int steps = 0; steps < n; ++steps) {
            current = next[current][end];

            if (current == -1 || current < 0 || current >= n) {
                return {};
            }

            path.push_back(current);

            if (current == end) {
                return path;
            }
        }

        return {};
    }
};

class FloydWarshallTestRunner : public TestRunner {
public:
    void runSingleTest(const string& graphFile, const string& testName) override {
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

        if (start < 0 || end < 0 || start >= n || end >= n) {
            cerr << "  Invalid start/end vertices in file: " << graphFile << endl;
            return;
        }

        cout << "  Path: " << labels[start] << " -> " << labels[end];
        cout << " (vertices: " << n << ", edges: " << countEdges(graph) << ")" << endl;

        auto startTime = chrono::high_resolution_clock::now();

        FloydWarshallResult result = FloydWarshall::findShortestPath(graph, labels, start, end);

        auto endTime = chrono::high_resolution_clock::now();
        const double executionTime = chrono::duration<double>(endTime - startTime).count();

        string pathSequence = "NO_PATH";
        if (result.pathFound && !result.bestPath.empty()) {
            pathSequence.clear();
            for (size_t i = 0; i < result.bestPath.size(); ++i) {
                pathSequence += labels[result.bestPath[i]];
                if (i + 1 < result.bestPath.size()) {
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
        testResult.iterations = 1;
        testResult.bestPathSequence = pathSequence;

        results.push_back(testResult);

        cout << "  Result: time=" << executionTime << "s, length=";
        if (result.pathFound) {
            cout << result.bestLength;
        } else {
            cout << "NO_PATH";
        }
        cout << ", found=" << (result.pathFound ? "yes" : "no") << endl;
    }

    void runTestSuite(const string& testDirectory) override {
        cout << "=== Floyd Warshall Algorithm Test Suite ===" << endl;
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

        for (size_t i = 0; i < testFiles.size() && testCount < maxTests; ++i) {
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
            ++testCount;
        }

        if (testCount == 0) {
            cout << "ERROR: No valid test files found!" << endl;
            cout << "Files from list exist but cannot be loaded." << endl;
            return;
        }

        cout << "\n=== Completed " << testCount << " tests ===" << endl;
        printSummary();
        saveResultsToCSV("results/fw_results.csv");
    }
};