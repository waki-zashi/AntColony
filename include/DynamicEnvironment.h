#pragma once
#include <vector>
#include <random>

using namespace std;

class DynamicEnvironment {
private:
    int n;
    int stateIndex = 0;

    vector<vector<double>> graph;

    double driftFactor = 0.02;
    double noiseLevel = 0.005;
    double changeProbability = 0.2;

    double minWeight = 1e-6;
    double maxWeight = 1e6;
    double lastChangeMagnitude = 0.0;

    mt19937 gen;
    uniform_real_distribution<> probDist;
    normal_distribution<> noiseDist;

public:
    DynamicEnvironment(const vector<vector<double>>& initialGraph,
        unsigned int seed = 42);

    const vector<vector<double>>& getGraph() const;

    void update();
    int getStateIndex() const;    double getLastChangeMagnitude() const;
    void reset(const vector<vector<double>>& newGraph);
};