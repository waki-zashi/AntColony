#pragma once

#include <vector>
#include <string>
#include <random>

class GraphGenerator {
public:
    explicit GraphGenerator(unsigned int seed = 42);

    std::vector<std::vector<double>> generateConnectedRandomGraph(int n, double density);
    std::vector<std::vector<double>> generateStarGraph(int n);
    std::vector<std::vector<double>> generatePathGraph(int n);
    std::vector<std::vector<double>> generateGridGraph(int rows, int cols);
    std::vector<std::vector<double>> generateCompleteGraph(int n);

    std::vector<std::string> generateLabels(int n);
    bool isConnected(const std::vector<std::vector<double>>& graph) const;

    void saveGraphToFile(const std::vector<std::vector<double>>& graph,
                         const std::vector<std::string>& labels,
                         const std::string& filename);

private:
    std::mt19937 gen;

    double randomWeight(double minW = 1.0, double maxW = 10.0);
    void addUndirectedEdge(std::vector<std::vector<double>>& graph, int u, int v, double w);
};