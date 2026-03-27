#include "DynamicEnvironment.h"
#include <algorithm>

DynamicEnvironment::DynamicEnvironment(
    const vector<vector<double>>& initialGraph,
    unsigned int seed)
    : gen(seed),
    probDist(0.0, 1.0),
    noiseDist(0.0, 1.0)
{
    graph = initialGraph;
    n = graph.size();
}

const vector<vector<double>>& DynamicEnvironment::getGraph() const {
    return graph;
}

int DynamicEnvironment::getStateIndex() const {
    return stateIndex;
}

double DynamicEnvironment::getLastChangeMagnitude() const {
    return lastChangeMagnitude;
}

void DynamicEnvironment::reset(const vector<vector<double>>& newGraph) {
    graph = newGraph;
    n = graph.size();
    stateIndex = 0;
}

void DynamicEnvironment::update() {

    stateIndex++;
    lastChangeMagnitude = 0.0;
    int changedEdges = 0;

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {

            if (graph[i][j] <= 0)
                continue;

            if (probDist(gen) > changeProbability)
                continue;

            double oldWeight = graph[i][j];

            double drift = driftFactor * graph[i][j];
            double noise = noiseLevel * graph[i][j] * noiseDist(gen);

            double newWeight = graph[i][j] + drift + noise;

            newWeight = max(minWeight, min(maxWeight, newWeight));

            double change = abs(newWeight - oldWeight);
            lastChangeMagnitude += change;
            changedEdges++;

            graph[i][j] = newWeight;
            graph[j][i] = newWeight;
        }
    }

    if (changedEdges > 0) {
        lastChangeMagnitude /= changedEdges;
    }
}