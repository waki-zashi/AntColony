#pragma once
#include "TestRunner.h"
#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <queue>
#include <algorithm>
#include <cmath>

using namespace std;

struct AStarResult {
    vector<int> bestPath;
    double bestLength;
    bool pathFound;

    AStarResult() : bestLength(numeric_limits<double>::max()), pathFound(false) {}
};

class AStar {
public:
    static AStarResult findShortestPath(const vector<vector<double>>& graph,
        const vector<string>& labels,
        int start, int end) {
        AStarResult result;
        int n = graph.size();

        vector<double> gScore(n, numeric_limits<double>::max());
        vector<double> fScore(n, numeric_limits<double>::max());
        vector<int> cameFrom(n, -1);
        vector<bool> closedSet(n, false);

        double minEdge = findMinEdge(graph);

        priority_queue<pair<double, int>,
            vector<pair<double, int>>,
            greater<pair<double, int>>> openSet;

        gScore[start] = 0.0;
        fScore[start] = heuristic(start, end, minEdge);
        openSet.push({ fScore[start], start });

        while (!openSet.empty()) {
            int current = openSet.top().second;
            openSet.pop();

            if (current == end) {
                result.pathFound = true;
                result.bestLength = gScore[end];
                result.bestPath = reconstructPath(cameFrom, start, end);
                return result;
            }

            closedSet[current] = true;

            for (int neighbor = 0; neighbor < n; neighbor++) {
                if (graph[current][neighbor] > 0 && !closedSet[neighbor]) {
                    double tentativeGScore = gScore[current] + graph[current][neighbor];

                    if (tentativeGScore < gScore[neighbor]) {
                        cameFrom[neighbor] = current;
                        gScore[neighbor] = tentativeGScore;
                        fScore[neighbor] = gScore[neighbor] + heuristic(neighbor, end, minEdge);

                        openSet.push({ fScore[neighbor], neighbor });
                    }
                }
            }
        }

        return result;
    }

private:
    static double heuristic(int from, int to, double minEdge) {
        return minEdge * abs(from - to);
    }

    static double findMinEdge(const vector<vector<double>>& graph) {
        double minEdge = numeric_limits<double>::max();
        int n = graph.size();

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (graph[i][j] > 0 && graph[i][j] < minEdge) {
                    minEdge = graph[i][j];
                }
            }
        }

        return (minEdge < numeric_limits<double>::max()) ? minEdge : 1.0;
    }

    static vector<int> reconstructPath(const vector<int>& cameFrom, int start, int end) {
        vector<int> path;
        int current = end;

        while (current != -1) {
            path.push_back(current);
            current = cameFrom[current];
        }

        reverse(path.begin(), path.end());
        return path;
    }
};

class AStarTestRunner : public TestRunner {
public:
    void runSingleTest(const string& graphFile, const string& testName) override {
        bool fileLoaded;
        vector<vector<double>> graph;
        vector<string> labels;
        int start = -1;
        int end = -1;

        readGraphFromFile(graphFile, fileLoaded, graph, labels, start, end);

        if (!fileLoaded || graph.empty()) {
            cerr << "  Failed to load graph: " << graphFile << endl;
            return;
        }

        int n = labels.size();
        if (n == 0) {
            cerr << "  Empty graph: " << graphFile << endl;
            return;
        }

        /*random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dist(0, n - 1);

        int start = dist(gen);
        int end;
        do {
            end = dist(gen);
        } while (end == start);*/

        cout << "  Path: " << labels[start] << " -> " << labels[end];
        cout << " (vertices: " << n << ", edges: " << countEdges(graph) << ")" << endl;

        auto startTime = chrono::high_resolution_clock::now();

        AStarResult result = AStar::findShortestPath(graph, labels, start, end);

        auto endTime = chrono::high_resolution_clock::now();
        double executionTime = chrono::duration<double>(endTime - startTime).count();

        string pathSequence = "NO_PATH";
        if (result.pathFound && !result.bestPath.empty()) {
            pathSequence = "";
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
        testResult.iterations = 1;
        testResult.bestPathSequence = pathSequence;

        results.push_back(testResult);

        cout << "  Result: time=" << executionTime << "s, length=";

        if (result.pathFound) {
            cout << result.bestLength;
        }
        else {
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
        const int maxTests = min(100, (int)testFiles.size());

        for (size_t i = 0; i < testFiles.size() && testCount < maxTests; i++) {
            string filename = testFiles[i];
            string fullPath = testDirectory + "/" + filename;

            if (!fileExists(fullPath)) {
                cout << "Warning: File from list not found: " << fullPath << endl;
                continue;
            }

            string testName = filename;
            size_t dotPos = testName.find_last_of(".");
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
        saveResultsToCSV("astar_results.csv");
    }
};