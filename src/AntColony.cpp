#include "AntColony.h"

#include <iostream>
#include <cmath>
#include <algorithm>

using namespace std;

Ant::Ant(int n)
    : visited(n, false), pathLength(0.0) {}

AntColony::AntColony(const vector<vector<double>>& g,
                     const vector<string>& names,
                     int s,
                     int e)
    : n(static_cast<int>(g.size())),
      start(s),
      end(e),
      graph(g),
      labels(names),
      alpha(1.0),
      beta(3.0),
      evaporation(0.3),
      Q(100.0),
      numAnts(max(10, static_cast<int>(g.size()))),
      maxIterations(max(100, static_cast<int>(g.size()) * 20)),
      stagnationLimit(max(20, static_cast<int>(g.size()) / 2)),
      tauMin(1e-4),
      tauMax(10.0),
      gen(random_device{}()),
      dist(0.0, 1.0)
{
    pheromone.assign(n, vector<double>(n, 1.0));
}

int AntColony::selectNext(const Ant& ant, int current) {
    vector<double> probabilities(n, 0.0);
    double sum = 0.0;

    for (int j = 0; j < n; ++j) {
        if (!ant.visited[j] && graph[current][j] > 0.0) {
            const double tau = max(tauMin, pheromone[current][j]);
            const double eta = 1.0 / graph[current][j];

            probabilities[j] = pow(tau, alpha) * pow(eta, beta);
            sum += probabilities[j];
        }
    }

    if (sum <= 0.0) {
        return -1;
    }

    const double r = dist(gen) * sum;
    double cumulative = 0.0;

    for (int j = 0; j < n; ++j) {
        if (probabilities[j] > 0.0) {
            cumulative += probabilities[j];
            if (cumulative >= r) {
                return j;
            }
        }
    }

    for (int j = 0; j < n; ++j) {
        if (probabilities[j] > 0.0) {
            return j;
        }
    }

    return -1;
}

void AntColony::evaporatePheromones() {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            pheromone[i][j] *= (1.0 - evaporation);
        }
    }
}

void AntColony::clampPheromones() {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            pheromone[i][j] = min(tauMax, max(tauMin, pheromone[i][j]));
        }
    }
}

void AntColony::depositPath(const vector<int>& path, double pathLength, double multiplier) {
    if (path.empty() || pathLength <= 0.0 || pathLength == numeric_limits<double>::max()) {
        return;
    }

    const double pheromoneAmount = multiplier * (Q / pathLength);

    for (size_t i = 0; i + 1 < path.size(); ++i) {
        const int u = path[i];
        const int v = path[i + 1];

        pheromone[u][v] += pheromoneAmount;
        pheromone[v][u] += pheromoneAmount;
    }
}

string AntColony::buildPathLabels(const vector<int>& path) const {
    string result;

    for (size_t i = 0; i < path.size(); ++i) {
        result += labels[path[i]];
        if (i + 1 < path.size()) {
            result += " -> ";
        }
    }

    return result;
}

ACOResult AntColony::run() {
    ACOResult result;

    vector<int> globalBestPath;
    double globalBestLength = numeric_limits<double>::max();

    int noImprovement = 0;
    bool foundAnyPathEver = false;

    for (int it = 0; it < maxIterations; ++it) {
        result.iterations = it + 1;

        vector<Ant> ants;
        ants.reserve(numAnts);

        for (int k = 0; k < numAnts; ++k) {
            ants.emplace_back(n);
            ants.back().path.push_back(start);
            ants.back().visited[start] = true;
        }

        vector<int> iterationBestPath;
        double iterationBestLength = numeric_limits<double>::max();
        bool foundPathThisIteration = false;

        for (auto& ant : ants) {
            while (!ant.path.empty() && ant.path.back() != end) {
                const int current = ant.path.back();
                const int next = selectNext(ant, current);

                if (next == -1) {
                    break;
                }

                ant.path.push_back(next);
                ant.visited[next] = true;
                ant.pathLength += graph[current][next];
            }

            if (!ant.path.empty() && ant.path.back() == end) {
                foundPathThisIteration = true;
                foundAnyPathEver = true;

                if (ant.pathLength < iterationBestLength) {
                    iterationBestLength = ant.pathLength;
                    iterationBestPath = ant.path;
                }

                if (ant.pathLength < globalBestLength) {
                    globalBestLength = ant.pathLength;
                    globalBestPath = ant.path;
                    result.pathFound = true;
                }
            }
        }

        evaporatePheromones();

        for (const auto& ant : ants) {
            if (!ant.path.empty() && ant.path.back() == end) {
                depositPath(ant.path, ant.pathLength, 1.0);
            }
        }

        if (foundPathThisIteration) {
            depositPath(iterationBestPath, iterationBestLength, 2.0);
        }

        if (result.pathFound) {
            depositPath(globalBestPath, globalBestLength, 3.0);
        }

        clampPheromones();

        bool improved = false;
        if (result.pathFound && globalBestLength < result.bestLength) {
            result.bestLength = globalBestLength;
            result.bestPath = globalBestPath;
            improved = true;
        }

        if (improved) {
            noImprovement = 0;
        } else {
            noImprovement++;
        }

        if (it % 10 == 0) {
            cout << "Iteration " << it + 1;
            if (result.pathFound) {
                cout << " - Best: " << result.bestLength;
            } else {
                cout << " - No path found yet";
            }
            cout << endl;
        }

       if (foundAnyPathEver && noImprovement >= stagnationLimit) {
            cout << "Stopped early after " << it + 1
                 << " iterations due to stagnation." << endl;
            break;
        }
    }

    if (result.pathFound) {
        result.bestPathLabels = buildPathLabels(result.bestPath);

        cout << "SUCCESS: Path found: " << result.bestPathLabels
             << " (length: " << result.bestLength
             << ", iterations: " << result.iterations << ")" << endl;
    } else {
        result.bestLength = numeric_limits<double>::max();
        cout << "FAIL: No path found from " << labels[start]
             << " to " << labels[end] << endl;
    }

    return result;
}