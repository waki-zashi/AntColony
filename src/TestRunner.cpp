#include "TestRunner.h"
#include "AntColony.h"
#include "FileReader.h"
#include "DynamicEnvironment.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <filesystem>

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
    runTestSuite(testDirectory, TestMode::STATIC, 0, 0);
}

void TestRunner::runTestSuite(const string& testDirectory,
    TestMode mode,
    int dynamicStates,
    int iterationsPerState) {

    cout << "=== ACO Algorithm Test Suite ===" << endl;
    cout << "Looking for test files in: " << testDirectory << endl;

    if (mode == TestMode::DYNAMIC) {
        cout << "Mode: DYNAMIC" << endl;
        cout << "Graph states: " << dynamicStates << endl;
        cout << "ACO iterations per state: " << iterationsPerState << endl;
    }
    else {
        cout << "Mode: STATIC" << endl;
    }

    vector<string> testFiles = readTestFilesList(testDirectory);

    if (testFiles.empty()) {
        cout << "ERROR: No test files list found or list is empty!" << endl;
        return;
    }

    cout << "Found " << testFiles.size() << " test files in the list." << endl;

    int testCount = 0;
    const int maxTests = min(100, (int)testFiles.size());

    for (size_t i = 0; i < testFiles.size() && testCount < maxTests; i++) {

        string filename = testFiles[i];
        string fullPath = testDirectory + "/" + filename;

        if (!fileExists(fullPath)) {
            cout << "Warning: File not found: " << fullPath << endl;
            continue;
        }

        string testName = filename;
        size_t dotPos = testName.find_last_of(".");
        if (dotPos != string::npos) {
            testName = testName.substr(0, dotPos);
        }

        cout << "\n[" << (testCount + 1) << "] Running: " << testName << endl;

        if (mode == TestMode::STATIC) {
            runSingleTest(fullPath, testName);
        }
        else { // DYNAMIC
            runDynamicTest(fullPath, testName,
                dynamicStates,
                iterationsPerState);
        }

        testCount++;
    }

    cout << "\n=== Completed " << testCount << " tests ===" << endl;
    printSummary();

    if (mode == TestMode::STATIC)
        saveResultsToCSV("aco_results.csv");
    else {
        saveResultsToCSV("aco_dynamic_summary.csv");
        saveDynamicResultsToCSV("aco_dynamic_states.csv");
    }
}

void TestRunner::runSingleTest(const string& graphFile,
    const string& testName) {

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

    cout << "  Path: " << labels[start] << " -> " << labels[end];
    cout << " (vertices: " << n
        << ", edges: " << countEdges(graph) << ")" << endl;

    auto startTime = chrono::high_resolution_clock::now();

    AntColony colony(graph, labels, start, end);
    colony.resetPheromones();
    colony.run(staticIterations);

    ACOResult result = colony.getResult();

    auto endTime = chrono::high_resolution_clock::now();
    double executionTime =
        chrono::duration<double>(endTime - startTime).count();

    TestResult testResult;
    testResult.testName = testName;
    testResult.executionTime = executionTime;
    testResult.bestPathLength = result.bestLength;
    testResult.vertices = n;
    testResult.edges = countEdges(graph);
    testResult.foundPath = result.pathFound;
    testResult.iterations = result.totalIterations;

    results.push_back(testResult);

    cout << "  Result: time=" << executionTime
        << "s, length=" << result.bestLength
        << ", found=" << (result.pathFound ? "yes" : "no")
        << endl;
}

void TestRunner::runDynamicTest(const string& filePath,
    const string& testName,
    int dynamicStates,
    int iterationsPerState) {

    bool fileLoaded;
    vector<vector<double>> graph;
    vector<string> labels;
    int start = -1;
    int end = -1;

    readGraphFromFile(filePath, fileLoaded, graph, labels, start, end);

    if (!fileLoaded || graph.empty()) {
        cerr << "  Failed to load graph: " << filePath << endl;
        return;
    }

    cout << "  Dynamic test started" << endl;

    DynamicEnvironment env(graph, 42);
    AntColony colony(graph, labels, start, end);

    auto startTime = chrono::high_resolution_clock::now();

    for (int state = 0; state < dynamicStates; state++) {

        cout << "   -> State " << state + 1
            << "/" << dynamicStates << endl;

        const auto& currentGraph = env.getGraph();

        colony.setGraph(currentGraph);

        for (int i = 0; i < iterationsPerState; i++) {
            colony.singleIteration();
        }

        auto result = colony.getResult();

        saveDynamicStateResult(
            testName,
            state,
            result.bestLength,
            colony.getPheromoneLoadMetric(),
            env.getLastChangeMagnitude()
        );

        env.update();
    }

    auto finalResult = colony.getResult();

    auto endTime = chrono::high_resolution_clock::now();
    double executionTime =
        chrono::duration<double>(endTime - startTime).count();

    TestResult testResult;
    testResult.testName = testName;
    testResult.executionTime = executionTime;
    testResult.bestPathLength = finalResult.bestLength;
    testResult.vertices = graph.size();
    testResult.edges = countEdges(graph);
    testResult.foundPath = finalResult.pathFound;
    testResult.iterations = finalResult.totalIterations;

    results.push_back(testResult);

    cout << "  Final dynamic result: length="
        << finalResult.bestLength
        << endl;
}

void TestRunner::saveDynamicStateResult(
    const string& testName,
    int state,
    double bestLength,
    double pheromoneLoad,
    double graphChangeMagnitude)
{
    DynamicStateResult r;
    r.testName = testName;
    r.state = state;
    r.bestLength = bestLength;
    r.pheromoneLoad = pheromoneLoad;
    r.graphChangeMagnitude = graphChangeMagnitude;

    dynamicResults.push_back(r);
}

void TestRunner::saveDynamicResultsToCSV(const string& filename) {

    ofstream file(filename);

    if (!file.is_open()) {
        cerr << "Cannot open dynamic results file\n";
        return;
    }

    file << "TestName,State,BestLength,PheromoneLoad,GraphChange\n";

    for (const auto& r : dynamicResults) {
        file << r.testName << ","
            << r.state << ","
            << r.bestLength << ","
            << r.pheromoneLoad << ","
            << r.graphChangeMagnitude << "\n";
    }

    cout << "Dynamic results saved to: "
        << filename << endl;
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