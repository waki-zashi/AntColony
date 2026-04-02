#pragma once

#include "TestRunner.h"

#include <vector>
#include <string>
#include <limits>

struct BellmanFordResult {
    std::vector<int> bestPath;
    std::string bestPathLabels;
    double bestLength;
    int iterations;
    bool pathFound;
    bool negativeCycleDetected;

    BellmanFordResult()
        : bestLength(std::numeric_limits<double>::max()),
          iterations(0),
          pathFound(false),
          negativeCycleDetected(false) {}
};

class BellmanFordSolver {
public:
    BellmanFordSolver(const std::vector<std::vector<double>>& g,
                      const std::vector<std::string>& names,
                      int s,
                      int e);

    BellmanFordResult run();

private:
    std::vector<std::vector<double>> graph;
    std::vector<std::string> labels;
    int start;
    int end;

    std::string buildPathLabels(const std::vector<int>& path) const;
};

class BellmanFordTestRunner : public TestRunner {
public:
    std::string getDefaultOutputFile() const override;
    void runTestSuite(const std::string& testDirectory) override;
    void runSingleTest(const std::string& graphFile, const std::string& testName) override;
};