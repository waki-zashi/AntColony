#include "PipelineRunner.h"

#include "TestRunner.h"
#include "Dijkstra.h"
#include "FloydWarshall.h"
#include "AStar.h"
#include "ResultAnalyzer.h"
#include "GenerateTestSuite.h"

#include <iostream>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

PipelineRunner::PipelineRunner(std::string testDirectory)
    : testDirectory(std::move(testDirectory)) {}

const string& PipelineRunner::getTestDirectory() const {
    return testDirectory;
}

bool PipelineRunner::ensureTestSuiteExists() const {
    const fs::path dir(testDirectory);
    const fs::path listFile = dir / "test_files_list.txt";

    return fs::exists(dir) &&
           fs::is_directory(dir) &&
           fs::exists(listFile) &&
           fs::is_regular_file(listFile);
}

void PipelineRunner::generateTests() const {
    cout << "=== GENERATING TEST SUITE ===" << endl;
    generateConnectedTestSuite(testDirectory);
    cout << "=== TEST GENERATION COMPLETE ===" << endl;
}

void PipelineRunner::runACO() const {
    cout << "=== ACO Algorithm Test Suite ===" << endl;
    TestRunner runner;
    runner.runTestSuite(testDirectory);
    cout << "=== Testing complete ===" << endl;
}

void PipelineRunner::runDijkstra() const {
    cout << "=== Dijkstra Algorithm Test Suite ===" << endl;
    DijkstraTestRunner runner;
    runner.runTestSuite(testDirectory);
    cout << "=== Dijkstra Testing complete ===" << endl;
}

void PipelineRunner::runFloydWarshall() const {
    cout << "=== Floyd-Warshall Algorithm Test Suite ===" << endl;
    FloydWarshallTestRunner runner;
    runner.runTestSuite(testDirectory);
    cout << "=== Floyd-Warshall Testing complete ===" << endl;
}

void PipelineRunner::runAStar() const {
    cout << "=== A* Algorithm Test Suite ===" << endl;
    AStarTestRunner runner;
    runner.runTestSuite(testDirectory);
    cout << "=== A* Testing complete ===" << endl;
}

void PipelineRunner::runAllAlgorithms() const {
    fs::create_directories("results");

    runACO();
    cout << endl;

    runDijkstra();
    cout << endl;

    runFloydWarshall();
    cout << endl;

    runAStar();
    cout << endl;
}

void PipelineRunner::analyzeResults() const {
    cout << "=== ALGORITHM COMPARISON TOOL ===" << endl;

    ResultsAnalyzer analyzer("results");
    analyzer.loadResults(
        "aco_results.csv",
        "dijkstra_results.csv",
        "astar_results.csv",
        "fw_results.csv"
    );

    analyzer.generateComparativeAnalysis();

    cout << "=== Analysis complete! ===" << endl;
    cout << "Results saved to:" << endl;
    cout << "- results/detailed_comparison.csv" << endl;
    cout << "- results/summary_comparison.csv" << endl;
}

void PipelineRunner::fullPipeline(bool generateIfMissing) const {
    if (!ensureTestSuiteExists()) {
        if (!generateIfMissing) {
            cerr << "Error: test suite not found in " << testDirectory << endl;
            cerr << "Run with --generate-tests first." << endl;
            return;
        }

        generateTests();
        cout << endl;
    }

    runAllAlgorithms();
    analyzeResults();
}