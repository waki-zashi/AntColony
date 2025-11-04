#pragma once
#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <random>

using namespace std;

struct TestResult {
    string testName;
    double bestPathLength;
    double executionTime;
    int vertices;
    int edges;
    bool foundPath;
    int iterations;
    string bestPathSequence;
};

void readGraphFromFile(const string& filename, bool& fileLoaded, vector<vector<double>>& graph, vector<string>& labels);
vector<string> readTestFilesList(const string& testDirectory);

class TestRunner {
public:
    vector<TestResult> results;

    int countEdges(const vector<vector<double>>& graph);
    bool fileExists(const string& filename);

    virtual void runTestSuite(const string& testDirectory);
    virtual void runSingleTest(const string& graphFile, const string& testName);
    void saveResultsToCSV(const string& filename);
    void printSummary();
};