#pragma once
#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <random>

using namespace std;

struct DynamicStateResult {
    string testName;
    int state;
    double bestLength;
    double pheromoneLoad;
    double graphChangeMagnitude;
};

vector<DynamicStateResult> dynamicResults;

enum class TestMode {
    STATIC,
    DYNAMIC
};

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

void readGraphFromFile(const string& filename, bool& fileLoaded,
    vector<vector<double>>& graph,
    vector<string>& labels,
    int& start, int& end);

vector<string> readTestFilesList(const string& testDirectory);

class TestRunner {
public:
    vector<TestResult> results;

    // �����
    TestMode mode = TestMode::STATIC;

    // ��������� static ������
    int staticIterations = 200;

    // ��������� dynamic ������
    int dynamicStates = 20;
    int iterationsPerState = 10;

    int countEdges(const vector<vector<double>>& graph);
    bool fileExists(const string& filename);

    virtual void runTestSuite(const string& testDirectory);
    void runTestSuite(const string& directory, TestMode mode, int dynamicStates = 10, int iterationsPerState = 3);
    virtual void runSingleTest(const string& graphFile, const string& testName);
    virtual void runDynamicTest(const string& filePath, const string& testName, int dynamicStates, int iterationsPerState);

    void saveDynamicStateResult(const string& testName,int state, double bestLength, double pheromoneLoad, double graphChangeMagnitude);
    void saveDynamicResultsToCSV(const string& filename);

    void saveResultsToCSV(const string& filename);
    void printSummary();
};