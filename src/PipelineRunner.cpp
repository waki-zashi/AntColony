#include "PipelineRunner.h"

#include "TestRunner.h"
#include "Dijkstra.h"
#include "FloydWarshall.h"
#include "AStar.h"
#include "ResultAnalyzer.h"
#include "GenerateTestSuite.h"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

using namespace std;
namespace fs = std::filesystem;

namespace {
    string escapeJson(const string& s) {
        string out;
        out.reserve(s.size());
        for (char c : s) {
            switch (c) {
                case '\"': out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                default: out += c; break;
            }
        }
        return out;
    }

    string currentTimestampUTC() {
        using namespace std::chrono;
        const auto now = system_clock::now();
        const std::time_t now_c = system_clock::to_time_t(now);

        std::tm tm{};
    #ifdef _WIN32
        gmtime_s(&tm, &now_c);
    #else
        gmtime_r(&now_c, &tm);
    #endif

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
        return oss.str();
    }
}

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

void PipelineRunner::clearResultsDirectory() const {
    fs::create_directories("results");

    const vector<string> filesToRemove = {
        "results/aco_results.csv",
        "results/dijkstra_results.csv",
        "results/astar_results.csv",
        "results/fw_results.csv",
        "results/detailed_comparison.csv",
        "results/summary_comparison.csv",
        "results/experiment_meta.json"
    };

    for (const auto& file : filesToRemove) {
        if (fs::exists(file)) {
            fs::remove(file);
        }
    }
}

void PipelineRunner::writeExperimentMetadata(const string& mode) const {
    fs::create_directories("results");

    ofstream file("results/experiment_meta.json");
    if (!file.is_open()) {
        cerr << "Warning: cannot write results/experiment_meta.json" << endl;
        return;
    }

    const bool hasSuite = ensureTestSuiteExists();

    file << "{\n";
    file << "  \"timestamp_utc\": \"" << escapeJson(currentTimestampUTC()) << "\",\n";
    file << "  \"mode\": \"" << escapeJson(mode) << "\",\n";
    file << "  \"test_directory\": \"" << escapeJson(testDirectory) << "\",\n";
    file << "  \"test_suite_exists\": " << (hasSuite ? "true" : "false") << ",\n";
    file << "  \"results_directory\": \"results\",\n";
    file << "  \"expected_result_files\": [\n";
    file << "    \"results/aco_results.csv\",\n";
    file << "    \"results/dijkstra_results.csv\",\n";
    file << "    \"results/astar_results.csv\",\n";
    file << "    \"results/fw_results.csv\",\n";
    file << "    \"results/detailed_comparison.csv\",\n";
    file << "    \"results/summary_comparison.csv\"\n";
    file << "  ],\n";
    file << "  \"pipeline\": {\n";
    file << "    \"generate_if_missing\": true,\n";
    file << "    \"run_algorithms\": [\"ACO\", \"Dijkstra\", \"A*\", \"Floyd-Warshall\"],\n";
    file << "    \"analyze_after_run\": true\n";
    file << "  }\n";
    file << "}\n";
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
    clearResultsDirectory();
    writeExperimentMetadata("full-pipeline-start");

    if (!ensureTestSuiteExists()) {
        if (!generateIfMissing) {
            cerr << "Error: test suite not found in " << testDirectory << endl;
            cerr << "Run with --generate-tests first." << endl;
            return;
        }

        generateTests();
        cout << endl;
    }

    writeExperimentMetadata("full-pipeline-running");

    runAllAlgorithms();
    analyzeResults();

    writeExperimentMetadata("full-pipeline-finished");
}