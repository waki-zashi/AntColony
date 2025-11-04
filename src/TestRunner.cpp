#include "TestRunner.h"
#include "AntColony.h"
#include "FileReader.h"
#include <iostream>
#include <sstream>
#include <algorithm>

using namespace std;

int TestRunner::countEdges(const vector<vector<double>>& graph) {
    int n = graph.size();
    int edgeCount = 0;

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (graph[i][j] > 0) {
                edgeCount++;
            }
        }
    }

    return edgeCount;
}

bool TestRunner::fileExists(const string& filename) {
    ifstream file(filename);
    return file.good();
}

vector<string> readTestFilesList(const string& testDirectory) {
    vector<string> files;
    string listFile = testDirectory + "/test_files_list.txt";

    ifstream file(listFile);
    if (!file.is_open()) {
        cout << "Warning: Cannot open test files list: " << listFile << endl;
        return files;
    }

    string line;
    while (getline(file, line)) {
        if (!line.empty()) {
            files.push_back(line);
        }
    }

    file.close();
    return files;
}

void TestRunner::runTestSuite(const string& testDirectory) {
    cout << "=== ACO Algorithm Test Suite ===" << endl;
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
    saveResultsToCSV("aco_results.csv");
}

void TestRunner::runSingleTest(const string& graphFile, const string& testName) {
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

    AntColony colony(graph, labels, start, end);
    ACOResult result = colony.run();

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
    testResult.iterations = result.iterations;
    testResult.bestPathSequence = pathSequence;

    results.push_back(testResult);

    cout << "  Result: time=" << executionTime << "s, length=";

    if (result.pathFound) {
        cout << result.bestLength;
    }
    else {
        cout << "NO_PATH";
    }

    cout << ", found=" << (result.pathFound ? "yes" : "no")
        << ", iterations=" << result.iterations << endl;
}

void TestRunner::saveResultsToCSV(const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Cannot open results file: " << filename << endl;
        return;
    }

    file << "TestName,Vertices,Edges,Time,PathLength,FoundPath,Iterations,PathSequence\n";

    for (const auto& result : results) {
        file << result.testName << ","
            << result.vertices << ","
            << result.edges << ","
            << result.executionTime << ","
            << result.bestPathLength << ","
            << (result.foundPath ? "true" : "false") << ","
            << result.iterations << ","
            << "\"" << result.bestPathSequence << "\"" << "\n";
    }

    cout << "Results saved to: " << filename << endl;
}

void TestRunner::printSummary() {
    if (results.empty()) {
        cout << "No results to display." << endl;
        return;
    }

    double totalTime = 0.0;
    int successfulTests = 0;
    int totalVertices = 0;
    int totalEdges = 0;

    for (const auto& result : results) {
        totalTime += result.executionTime;
        totalVertices += result.vertices;
        totalEdges += result.edges;
        if (result.foundPath) {
            successfulTests++;
        }
    }

    cout << "\n=== TEST SUMMARY ===" << endl;
    cout << "Total tests: " << results.size() << endl;
    cout << "Successful: " << successfulTests << " ("
        << (successfulTests * 100.0 / results.size()) << "%)" << endl;
    cout << "Total time: " << totalTime << " seconds" << endl;
    cout << "Average time: " << totalTime / results.size() << " seconds" << endl;
    cout << "Average vertices: " << totalVertices / results.size() << endl;
    cout << "Average edges: " << totalEdges / results.size() << endl;
}