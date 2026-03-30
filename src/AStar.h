#pragma once

#include "TestRunner.h"
#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <queue>
#include <algorithm>
#include <cmath>
#include <chrono>

using namespace std;

struct AStarResult {
    vector<int> bestPath;
    double bestLength;
    bool pathFound;

    AStarResult()
        : bestLength(numeric_limits<double>::max()), pathFound(false) {}
};

class AStar {
public:
    static AStarResult findShortestPath(const vector<vector<double>>& graph,
                                        const vector<string>& labels,
                                        int start, int end) {
        AStarResult result;

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

        vector<double> gScore(n, numeric_limits<double>::max());
        vector<double> fScore(n, numeric_limits<double>::max());
        vector<int> cameFrom(n, -1);

        using QueueEntry = pair<double, int>;
        priority_queue<QueueEntry, vector<QueueEntry>, greater<QueueEntry>> openSet;

        gScore[start] = 0.0;
        fScore[start] = heuristic(start, end, graph, labels);
        openSet.push({ fScore[start], start });

        while (!openSet.empty()) {
            const double currentF = openSet.top().first;
            const int current = openSet.top().second;
            openSet.pop();

            if (currentF > fScore[current]) {
                continue;
            }

            if (current == end) {
                result.pathFound = true;
                result.bestLength = gScore[end];
                result.bestPath = reconstructPath(cameFrom, start, end);
                return result;
            }

            for (int neighbor = 0; neighbor < n; ++neighbor) {
                if (graph[current][neighbor] <= 0.0) {
                    continue;
                }

                const double tentativeGScore = gScore[current] + graph[current][neighbor];

                if (tentativeGScore < gScore[neighbor]) {
                    cameFrom[neighbor] = current;
                    gScore[neighbor] = tentativeGScore;
                    fScore[neighbor] = tentativeGScore + heuristic(neighbor, end, graph, labels);
                    openSet.push({ fScore[neighbor], neighbor });
                }
            }
        }

        return result;
    }

private:
    static double heuristic(int from,
                            int to,
                            const vector<vector<double>>& graph,
                            const vector<string>& labels) {
        (void)from;
        (void)to;
        (void)graph;
        (void)labels;
        return 0.0;
    }

    static vector<int> reconstructPath(const vector<int>& cameFrom, int start, int end) {
        vector<int> path;
        int current = end;

        while (current != -1) {
            path.push_back(current);
            if (current == start) {
                break;
            }
            current = cameFrom[current];
        }

        reverse(path.begin(), path.end());

        if (path.empty() || path.front() != start) {
            return {};
        }

        return path;
    }
};

class AStarTestRunner : public TestRunner {
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

        AStarResult result = AStar::findShortestPath(graph, labels, start, end);

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
        cout << "=== A-Star Algorithm Test Suite ===" << endl;
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
        saveResultsToCSV("results/astar_results.csv");
    }
};