#include "AntColony.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <limits>
#include <cmath>

using namespace std;

Ant::Ant(int n) : visited(n, false), pathLength(0.0) {}

AntColony::AntColony(const vector<vector<double>>& g, const vector<string>& names, int s, int e) : gen(std::random_device{}()), dist(0.0, 1.0) {
    graph = g;
    labels = names;
    n = g.size();
    start = s;
    end = e;
    pheromone.assign(n, vector<double>(n, 1.0));
    maxIterations = n * 10;
}

int AntColony::selectNext(Ant& ant, int current) {
    vector<double> probabilities(n, 0.0);
    double sum = 0.0;

    for (int j = 0; j < n; j++) {
        if (!ant.visited[j] && graph[current][j] > 0) {
            probabilities[j] = pow(pheromone[current][j], alpha) *
                pow(1.0 / graph[current][j], beta);
            sum += probabilities[j];
        }
    }
    if (sum == 0.0) return -1;

    double r = dist(gen) * sum;
    double cum = 0.0;
    for (int j = 0; j < n; j++) {
        cum += probabilities[j];
        if (cum >= r) return j;
    }
    return -1;
}

ACOResult AntColony::run() {
    ACOResult result;
    vector<int> bestPath;
    double bestLength = numeric_limits<double>::max();
    bool foundAnyPath = false;

    int noImprovement = 0;

    for (int it = 0; it < maxIterations; it++) {
        result.iterations = it + 1;

        vector<Ant> ants;
        for (int k = 0; k < numAnts; k++) {
            ants.emplace_back(n);
            ants[k].path.push_back(start);
            ants[k].visited[start] = true;
        }

        for (auto& ant : ants) {
            while (ant.path.back() != end) {
                int current = ant.path.back();
                int next = selectNext(ant, current);

                if (next == -1) break;
                ant.path.push_back(next);
                ant.visited[next] = true;
                ant.pathLength += graph[current][next];
            }

            if (ant.path.back() == end) 
                foundAnyPath = true;
        }

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                pheromone[i][j] *= (1.0 - evaporation);
            }
        }

        bool improved = false;

        for (auto& ant : ants) {
            if (ant.path.back() == end) {
                double pheromoneAmount = Q / ant.pathLength;

                for (size_t i = 0; i + 1 < ant.path.size(); i++) {
                    int u = ant.path[i];
                    int v = ant.path[i + 1];
                    pheromone[u][v] += pheromoneAmount;
                    pheromone[v][u] += pheromoneAmount;
                }

                if (ant.pathLength < bestLength) {
                    bestLength = ant.pathLength;
                    bestPath = ant.path;
                    improved = true;
                    result.pathFound = true;
                }
            }
        }

//        cout << "Iteration " << it + 1 << " - Pheromone matrix:\n\t";
//        for (int j = 0; j < n; j++) cout << labels[j] << "\t";
//        cout << "\n";
//        for (int i = 0; i < n; i++) {
//            cout << labels[i] << "\t";
//            for (int j = 0; j < n; j++) {
//                cout << pheromone[i][j] << "\t";
//            }
//            cout << "\n";
//        }
//        cout << "---------------------------\n";
//
//        this_thread::sleep_for(chrono::seconds(1));
// 
////#ifdef _WIN32
////        system("cls");
////#else
////        system("clear");
////#endif

        if (improved) 
            noImprovement = 0;
        
        else 
            noImprovement++;
        

        if (it % 10 == 0) {
            cout << "Iteration " << it + 1;
            if (result.pathFound) {
                cout << " - Best: " << bestLength;
            }
            else {
                cout << " - No path found yet";
            }
            cout << endl;
        }

        if (noImprovement >= stagnationLimit) {
            cout << "Stopped early after " << it + 1 << " iterations due to stagnation." << endl;
            break;
        }
    }

    result.bestPath = bestPath;
    result.bestLength = bestLength;

    if (!result.pathFound) 
        result.bestLength = numeric_limits<double>::max();
    

    if (result.pathFound) {
        cout << "SUCCESS: Path found: ";
        for (size_t i = 0; i < result.bestPath.size(); i++) {
            string label = labels[result.bestPath[i]];
            result.bestPathLabels.append(label);

            cout << label;
            if (i < result.bestPath.size() - 1) 
                cout << " -> ";
        }
        cout << " (length: " << result.bestLength << ", iterations: " << result.iterations << ")" << endl;
    }
    else {
        cout << "FAIL: No path found from " << labels[start] << " to " << labels[end] << endl;
    }

    return result;
}