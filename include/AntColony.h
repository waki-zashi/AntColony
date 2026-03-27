#pragma once
#include <vector>
#include <string>
#include <random>
#include <limits>

using namespace std;

struct Ant {
    vector<int> path;
    vector<bool> visited;
    double pathLength;
    bool reachedEnd;

    Ant(int n) : visited(n, false), pathLength(0.0), reachedEnd(false) {}
};

struct ACOResult {
    vector<int> bestPath;
    string bestPathLabels;
    double bestLength;
    int totalIterations;
    bool pathFound;

    ACOResult()
        : bestLength(numeric_limits<double>::max()),
        totalIterations(0),
        pathFound(false) {}
};

class AntColony {
private:
    int n;
    int start, end;

    const vector<vector<double>>* graph;
    vector<vector<double>> pheromone;
    vector<string> labels;

    // ACO parameters
    double alpha = 1.0;
    double beta = 4.0;
    double evaporation = 0.3;
    double Q = 100.0;

    double tau_min = 1e-6;
    double tau_max = 1000.0;

    int numAnts;
    int stagnationLimit;

    vector<int> globalBestPath;
    double globalBestLength = numeric_limits<double>::max();
    int noImprovement = 0;
    int totalIterations = 0;

    mt19937 gen;
    uniform_real_distribution<> dist;

    int selectNext(Ant& ant, int current);
    void evaporate();
    void deposit(const Ant& bestAnt);
    void clampPheromones();

public:
    AntColony(const vector<vector<double>>& g,
        const vector<string>& names,
        int s, int e);

    void setGraph(const vector<vector<double>>& g);

    void iterate();
    void singleIteration();
    void runIterations(int k);
    void run(int iterations = 200);

    ACOResult getResult() const;
    void resetPheromones();
    double getPheromoneLoadMetric() const;
};