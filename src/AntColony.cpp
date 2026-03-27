#include "AntColony.h"
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace std;

AntColony::AntColony(const vector<vector<double>>& g, const vector<string>& names, int s, int e)
    : gen(random_device{}()),
    dist(0.0, 1.0)
{
    graph = &g;
    labels = names;
    n = g.size();
    start = s;
    end = e;

    pheromone.assign(n, vector<double>(n, 1.0));
    numAnts = max(10, n / 4);
    stagnationLimit = max(30, n / 5);
}

void AntColony::setGraph(const vector<vector<double>>& g) {
    graph = &g;
}

void AntColony::iterate() {

    vector<Ant> ants;
    ants.reserve(numAnts);

    totalIterations++;

    for (int k = 0; k < numAnts; k++) {
        ants.emplace_back(n);
        ants[k].path.push_back(start);
        ants[k].visited[start] = true;
    }

    // Construction
    for (auto& ant : ants) {

        while (ant.path.back() != end) {

            int current = ant.path.back();
            int next = selectNext(ant, current);

            if (next == -1)
                break;

            ant.path.push_back(next);
            ant.visited[next] = true;
            ant.pathLength += (*graph)[current][next];
        }

        if (ant.path.back() == end)
            ant.reachedEnd = true;
    }

    Ant* bestAnt = nullptr;

    for (auto& ant : ants)
        if (ant.reachedEnd)
            if (!bestAnt || ant.pathLength < bestAnt->pathLength)
                bestAnt = &ant;

    evaporate();

    if (bestAnt) {
        deposit(*bestAnt);

        if (bestAnt->pathLength < globalBestLength) {
            globalBestLength = bestAnt->pathLength;
            globalBestPath = bestAnt->path;
            noImprovement = 0;
        }
        else {
            noImprovement++;
        }
    }
    else {
        noImprovement++;
    }

    clampPheromones();
}

void AntColony::singleIteration() {
    iterate();
}

void AntColony::runIterations(int k) {
    for (int i = 0; i < k; i++)
        iterate();
}

void AntColony::run(int iterations) {
    runIterations(iterations);
}

ACOResult AntColony::getResult() const {

    ACOResult result;

    if (!globalBestPath.empty()) {

        result.pathFound = true;
        result.bestPath = globalBestPath;
        result.bestLength = globalBestLength;
        result.totalIterations = totalIterations;

        for (size_t i = 0; i < globalBestPath.size(); i++) {
            result.bestPathLabels += labels[globalBestPath[i]];
            if (i + 1 < globalBestPath.size())
                result.bestPathLabels += " -> ";
        }
    }

    return result;
}

void AntColony::resetPheromones() {

    pheromone.assign(n, vector<double>(n, 1.0));
    globalBestPath.clear();
    globalBestLength = numeric_limits<double>::max();
    totalIterations = 0;
    noImprovement = 0;
}

int AntColony::selectNext(Ant& ant, int current) {

    vector<double> probabilities(n, 0.0);
    double sum = 0.0;

    for (int j = 0; j < n; j++) {
        if (!ant.visited[j] && (*graph)[current][j] > 0) {

            double heuristic = 1.0 / max((*graph)[current][j], 1e-9);

            probabilities[j] =
                pow(pheromone[current][j], alpha) *
                pow(heuristic, beta);

            sum += probabilities[j];
        }
    }

    if (sum <= 0.0)
        return -1;

    double r = dist(gen) * sum;
    double cumulative = 0.0;

    for (int j = 0; j < n; j++) {
        cumulative += probabilities[j];
        if (cumulative >= r)
            return j;
    }

    return -1;
}

void AntColony::evaporate() {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            pheromone[i][j] *= (1.0 - evaporation);
}

void AntColony::deposit(const Ant& bestAnt) {

    if (!bestAnt.reachedEnd)
        return;

    double delta = Q / max(bestAnt.pathLength, 1e-9);

    for (size_t i = 0; i + 1 < bestAnt.path.size(); i++) {
        int u = bestAnt.path[i];
        int v = bestAnt.path[i + 1];

        pheromone[u][v] += delta;
        pheromone[v][u] += delta;
    }
}

void AntColony::clampPheromones() {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            pheromone[i][j] =
            max(tau_min, min(tau_max, pheromone[i][j]));
}

double AntColony::getPheromoneLoadMetric() const {
    double totalPheromone = 0.0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            totalPheromone += pheromone[i][j];
        }
    }
    return totalPheromone / (n * n);
}
