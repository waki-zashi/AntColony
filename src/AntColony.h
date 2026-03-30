#pragma once

#include <vector>
#include <string>
#include <random>
#include <limits>

struct Ant {
    std::vector<int> path;
    std::vector<bool> visited;
    double pathLength;

    explicit Ant(int n);
};

struct ACOResult {
    std::vector<int> bestPath;
    std::string bestPathLabels;
    double bestLength;
    int iterations;
    bool pathFound;

    ACOResult()
        : bestLength(std::numeric_limits<double>::max()),
          iterations(0),
          pathFound(false) {}
};

class AntColony {
private:
    int n;
    int start;
    int end;

    std::vector<std::vector<double>> graph;
    std::vector<std::vector<double>> pheromone;
    std::vector<std::string> labels;

    double alpha;
    double beta;
    double evaporation;
    double Q;

    int numAnts;
    int maxIterations;
    int stagnationLimit;

    double tauMin;
    double tauMax;

    std::mt19937 gen;
    std::uniform_real_distribution<> dist;

private:
    int selectNext(const Ant& ant, int current);
    void evaporatePheromones();
    void clampPheromones();
    void depositPath(const std::vector<int>& path, double pathLength, double multiplier = 1.0);
    std::string buildPathLabels(const std::vector<int>& path) const;

public:
    AntColony(const std::vector<std::vector<double>>& g,
              const std::vector<std::string>& names,
              int s,
              int e);

    ACOResult run();
};