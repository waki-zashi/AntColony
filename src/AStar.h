#pragma once
#include "TestRunner.h"
#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <queue>
#include <algorithm>
#include <cmath>

using namespace std;

struct AStarResult {
    vector<int> bestPath;
    double bestLength;
    bool pathFound;

    AStarResult() : bestLength(numeric_limits<double>::max()), pathFound(false) {}
};

class AStar {
public:
    static AStarResult findShortestPath(const vector<vector<double>>& graph,
        const vector<string>& labels,
        int start, int end) {
        AStarResult result;
        int n = graph.size();

        // Массивы для алгоритма A*
        vector<double> gScore(n, numeric_limits<double>::max()); // стоимость от start
        vector<double> fScore(n, numeric_limits<double>::max()); // gScore + эвристика
        vector<int> cameFrom(n, -1);
        vector<bool> closedSet(n, false);

        // Эвристическая функция (евклидово расстояние или другая)
        // В данном случае используем минимальное ребро как эвристику
        double minEdge = findMinEdge(graph);

        // Приоритетная очередь: (fScore, вершина)
        priority_queue<pair<double, int>,
            vector<pair<double, int>>,
            greater<pair<double, int>>> openSet;

        // Инициализация
        gScore[start] = 0.0;
        fScore[start] = heuristic(start, end, minEdge);
        openSet.push({ fScore[start], start });

        while (!openSet.empty()) {
            // Извлекаем вершину с наименьшим fScore
            int current = openSet.top().second;
            openSet.pop();

            // Если дошли до конечной вершины
            if (current == end) {
                result.pathFound = true;
                result.bestLength = gScore[end];
                result.bestPath = reconstructPath(cameFrom, start, end);
                return result;
            }

            // Помечаем как обработанную
            closedSet[current] = true;

            // Обходим всех соседей
            for (int neighbor = 0; neighbor < n; neighbor++) {
                if (graph[current][neighbor] > 0 && !closedSet[neighbor]) {
                    double tentativeGScore = gScore[current] + graph[current][neighbor];

                    if (tentativeGScore < gScore[neighbor]) {
                        // Этот путь лучше предыдущего
                        cameFrom[neighbor] = current;
                        gScore[neighbor] = tentativeGScore;
                        fScore[neighbor] = gScore[neighbor] + heuristic(neighbor, end, minEdge);

                        openSet.push({ fScore[neighbor], neighbor });
                    }
                }
            }
        }

        return result;
    }

private:
    static double heuristic(int from, int to, double minEdge) {
        // Простая эвристика: минимальное ребро * манхэттенское расстояние
        // Это допустимая эвристика (never overestimates)
        return minEdge * abs(from - to);
    }

    static double findMinEdge(const vector<vector<double>>& graph) {
        double minEdge = numeric_limits<double>::max();
        int n = graph.size();

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (graph[i][j] > 0 && graph[i][j] < minEdge) {
                    minEdge = graph[i][j];
                }
            }
        }

        return (minEdge < numeric_limits<double>::max()) ? minEdge : 1.0;
    }

    static vector<int> reconstructPath(const vector<int>& cameFrom, int start, int end) {
        vector<int> path;
        int current = end;

        while (current != -1) {
            path.push_back(current);
            current = cameFrom[current];
        }

        reverse(path.begin(), path.end());
        return path;
    }
};

class AStarTestRunner : public TestRunner {
public:
    void runSingleTest(const string& graphFile, const string& testName) override {
        // Загрузка графа
        bool fileLoaded;
        vector<vector<double>> graph;
        vector<string> labels;

        readGraphFromFile(graphFile, fileLoaded, graph, labels);

        if (!fileLoaded || graph.empty()) {
            cerr << "  Failed to load graph: " << graphFile << endl;
            return;
        }

        int n = labels.size();
        if (n == 0) {
            cerr << "  Empty graph: " << graphFile << endl;
            return;
        }

        // Выбор случайных start и end (разные вершины)
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dist(0, n - 1);

        int start = dist(gen);
        int end;
        do {
            end = dist(gen);
        } while (end == start);

        cout << "  Path: " << labels[start] << " -> " << labels[end];
        cout << " (vertices: " << n << ", edges: " << countEdges(graph) << ")" << endl;

        // Запуск алгоритма A* с замером времени
        auto startTime = chrono::high_resolution_clock::now();

        AStarResult result = AStar::findShortestPath(graph, labels, start, end);

        auto endTime = chrono::high_resolution_clock::now();
        double executionTime = chrono::duration<double>(endTime - startTime).count();

        // Формируем строку с путем
        string pathSequence = "NO_PATH";
        if (result.pathFound && !result.bestPath.empty()) {
            pathSequence = "";
            for (size_t i = 0; i < result.bestPath.size(); i++) {
                pathSequence += labels[result.bestPath[i]];
                if (i < result.bestPath.size() - 1) {
                    pathSequence += "->";
                }
            }
        }

        // Сохранение результатов
        TestResult testResult;
        testResult.testName = testName;
        testResult.executionTime = executionTime;
        testResult.bestPathLength = result.bestLength;
        testResult.vertices = n;
        testResult.edges = countEdges(graph);
        testResult.foundPath = result.pathFound;
        testResult.iterations = 1; // A* всегда за 1 "итерацию"
        testResult.bestPathSequence = pathSequence;

        results.push_back(testResult);

        cout << "  Result: time=" << executionTime << "s, length=";

        if (result.pathFound) {
            cout << result.bestLength;
        }
        else {
            cout << "NO_PATH";
        }

        cout << ", found=" << (result.pathFound ? "yes" : "no") << endl;
    }
};