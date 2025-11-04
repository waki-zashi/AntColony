#pragma once
#include "TestRunner.h"
#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <queue>
#include <algorithm>

using namespace std;

struct DijkstraResult {
    vector<int> bestPath;
    double bestLength;
    bool pathFound;

    DijkstraResult() : bestLength(numeric_limits<double>::max()), pathFound(false) {}
};

class Dijkstra {
public:
    static DijkstraResult findShortestPath(const vector<vector<double>>& graph,
        const vector<string>& labels,
        int start, int end) {
        DijkstraResult result;
        int n = graph.size();

        vector<double> dist(n, numeric_limits<double>::max());
        vector<int> prev(n, -1);
        vector<bool> visited(n, false);

        priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> pq;

        dist[start] = 0.0;
        pq.push({ 0.0, start });

        while (!pq.empty()) {
            double currentDist = pq.top().first;
            int current = pq.top().second;
            pq.pop();

            if (visited[current]) continue;
            visited[current] = true;

            if (current == end) break;

            for (int neighbor = 0; neighbor < n; neighbor++) {
                if (graph[current][neighbor] > 0 && !visited[neighbor]) {
                    double newDist = currentDist + graph[current][neighbor];

                    if (newDist < dist[neighbor]) {
                        dist[neighbor] = newDist;
                        prev[neighbor] = current;
                        pq.push({ newDist, neighbor });
                    }
                }
            }
        }

        if (dist[end] < numeric_limits<double>::max()) {
            result.pathFound = true;
            result.bestLength = dist[end];

            vector<int> path;
            for (int v = end; v != -1; v = prev[v]) {
                path.push_back(v);
            }
            reverse(path.begin(), path.end());
            result.bestPath = path;
        }

        return result;
    }
};

class DijkstraTestRunner : public TestRunner {
public:
    void runSingleTest(const string& graphFile, const string& testName) override {
        bool fileLoaded;
        vector<vector<double>> graph;
        vector<string> labels;

        readGraphFromFile(graphFile, fileLoaded, graph, labels);

        if (!fileLoaded || graph.empty()) {
            cerr << "  Failed to load graph: " << graphFile << endl;
            return;
        }

        int n = labels.size();
        if (n == 0) {
            cerr << "  Empty graph: " << graphFile << endl;
            return;
        }

        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dist(0, n - 1);

        int start = dist(gen);
        int end;
        do {
            end = dist(gen);
        } while (end == start);

        cout << "  Path: " << labels[start] << " -> " << labels[end];
        cout << " (vertices: " << n << ", edges: " << countEdges(graph) << ")" << endl;

        auto startTime = chrono::high_resolution_clock::now();

        DijkstraResult result = Dijkstra::findShortestPath(graph, labels, start, end);

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
        cout << "=== Dijkstra Algorithm Test Suite ===" << endl;
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
        saveResultsToCSV("dijkstra_results.csv");
    }
};