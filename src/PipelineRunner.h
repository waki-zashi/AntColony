#pragma once

#include <string>

class PipelineRunner {
public:
    explicit PipelineRunner(std::string testDirectory = "data/test_cases");

    bool ensureTestSuiteExists() const;
    void generateTests() const;

    void runACO() const;
    void runDijkstra() const;
    void runFloydWarshall() const;
    void runAStar() const;

    void runAllAlgorithms() const;
    void analyzeResults() const;
    void fullPipeline(bool generateIfMissing = true) const;

    const std::string& getTestDirectory() const;

private:
    std::string testDirectory;
};