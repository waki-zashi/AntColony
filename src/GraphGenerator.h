#pragma once
#include <vector>
#include <string>
#include <random>
#include <fstream>
#include <queue>
#include <algorithm>
#include <cmath>

using namespace std;

class GraphGenerator {
private:
    mt19937 gen;

public:
    GraphGenerator(int seed = 42) : gen(seed) {}

    vector<vector<double>> generateConnectedRandomGraph(int n, double density = 0.3);
    vector<vector<double>> generateStarGraph(int n);
    vector<vector<double>> generatePathGraph(int n);
    vector<vector<double>> generateGridGraph(int rows, int cols);
    vector<vector<double>> generateCompleteGraph(int n);

    bool isConnected(const vector<vector<double>>& graph);
    vector<string> generateLabels(int n);
    void saveGraphToFile(const vector<vector<double>>& graph, const vector<string>& labels, const string& filename);
};