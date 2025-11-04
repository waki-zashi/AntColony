#pragma once
#include <vector>
#include <string>
#include <random>

using namespace std;

struct Ant {
    vector<int> path;
    vector<bool> visited;
    double pathLength;
    Ant(int n);
};

struct ACOResult {
    vector<int> bestPath;
    string bestPathLabels;
    double bestLength;
    int iterations;
    bool pathFound;

    ACOResult() : bestLength(std::numeric_limits<double>::max()), iterations(0), pathFound(false) {}
};

class AntColony {
private:
    int n;
    int start, end;
    vector<vector<double>> graph;
    vector<vector<double>> pheromone;
    vector<string> labels;

    double alpha = 1.0;
    double beta = 5.0;
    double evaporation = 0.5;
    double Q = 100.0;
    int numAnts = 10;
    int maxIterations;
    int stagnationLimit = 5;

    std::mt19937 gen;
    std::uniform_real_distribution<> dist;

    int selectNext(Ant& ant, int current);

public:
    AntColony(const vector<vector<double>>& g, const vector<string>& names, int s, int e);

    ACOResult run();
};