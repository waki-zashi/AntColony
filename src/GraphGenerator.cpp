#include "GraphGenerator.h"
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <fstream>
#include <queue>
#include <algorithm>
#include <cmath>

using namespace std;

vector<vector<double>> GraphGenerator::generateConnectedRandomGraph(int n, double density) {
    if (n <= 0) return {};
    if (n == 1) return { {0.0} };

    vector<vector<double>> graph(n, vector<double>(n, 0.0));
    uniform_real_distribution<double> weightDist(1.0, 10.0);

    // Создаем остовное дерево для гарантии связности
    vector<bool> inTree(n, false);
    vector<int> treeVertices;

    // Начинаем со случайной вершины
    int startVertex = uniform_int_distribution<int>(0, n - 1)(gen);
    inTree[startVertex] = true;
    treeVertices.push_back(startVertex);

    while (treeVertices.size() < n) {
        // Выбираем случайную вершину из уже добавленных
        int randomIndex = uniform_int_distribution<int>(0, treeVertices.size() - 1)(gen);
        int u = treeVertices[randomIndex];

        // Выбираем случайную вершину еще не в дереве
        int v;
        do {
            v = uniform_int_distribution<int>(0, n - 1)(gen);
        } while (inTree[v]);

        // Добавляем ребро
        double weight = weightDist(gen);
        graph[u][v] = weight;
        graph[v][u] = weight;

        inTree[v] = true;
        treeVertices.push_back(v);
    }

    // Добавляем дополнительные ребра согласно density
    uniform_real_distribution<double> probDist(0.0, 1.0);

    // Уже есть n-1 ребро от остовного дерева
    int currentEdges = n - 1;
    int maxPossibleEdges = n * (n - 1) / 2;
    int targetEdges = max(currentEdges, static_cast<int>(density * maxPossibleEdges));

    // Пытаемся добавить ребра, пока не достигнем targetEdges
    while (currentEdges < targetEdges) {
        int u = uniform_int_distribution<int>(0, n - 1)(gen);
        int v = uniform_int_distribution<int>(0, n - 1)(gen);

        if (u != v && graph[u][v] == 0.0) {
            if (probDist(gen) < 0.3) { // Вероятность добавления
                double weight = weightDist(gen);
                graph[u][v] = weight;
                graph[v][u] = weight;
                currentEdges++;
            }
        }
    }

    return graph;
}

vector<vector<double>> GraphGenerator::generateGridGraph(int rows, int cols) {
    int n = rows * cols;
    vector<vector<double>> graph(n, vector<double>(n, 0.0));
    uniform_real_distribution<double> weightDist(1.0, 5.0);

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int current = i * cols + j;

            // Соединение с правой вершиной
            if (j < cols - 1) {
                int right = i * cols + (j + 1);
                double weight = weightDist(gen);
                graph[current][right] = weight;
                graph[right][current] = weight;
            }

            // Соединение с нижней вершиной
            if (i < rows - 1) {
                int down = (i + 1) * cols + j;
                double weight = weightDist(gen);
                graph[current][down] = weight;
                graph[down][current] = weight;
            }
        }
    }

    return graph;
}

vector<vector<double>> GraphGenerator::generateStarGraph(int n) {
    vector<vector<double>> graph(n, vector<double>(n, 0.0));
    uniform_real_distribution<double> weightDist(1.0, 5.0);

    // Центральная вершина 0 соединена со всеми остальными
    for (int i = 1; i < n; i++) {
        double weight = weightDist(gen);
        graph[0][i] = weight;
        graph[i][0] = weight;
    }

    return graph;
}

vector<vector<double>> GraphGenerator::generatePathGraph(int n) {
    vector<vector<double>> graph(n, vector<double>(n, 0.0));
    uniform_real_distribution<double> weightDist(1.0, 3.0);

    // Линейный путь: 0-1-2-3-...-(n-1)
    for (int i = 0; i < n - 1; i++) {
        double weight = weightDist(gen);
        graph[i][i + 1] = weight;
        graph[i + 1][i] = weight;
    }

    return graph;
}

vector<vector<double>> GraphGenerator::generateCompleteGraph(int n) {
    vector<vector<double>> graph(n, vector<double>(n, 0.0));
    uniform_real_distribution<double> weightDist(1.0, 10.0);

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            double weight = weightDist(gen);
            graph[i][j] = weight;
            graph[j][i] = weight;
        }
    }

    return graph;
}

bool GraphGenerator::isConnected(const vector<vector<double>>& graph) {
    int n = graph.size();
    if (n == 0) return true;

    vector<bool> visited(n, false);
    queue<int> q;
    int visitedCount = 0;

    q.push(0);
    visited[0] = true;
    visitedCount++;

    while (!q.empty()) {
        int current = q.front();
        q.pop();

        for (int neighbor = 0; neighbor < n; neighbor++) {
            if (graph[current][neighbor] > 0 && !visited[neighbor]) {
                visited[neighbor] = true;
                visitedCount++;
                q.push(neighbor);
            }
        }
    }

    return (visitedCount == n);
}

vector<string> GraphGenerator::generateLabels(int n) {
    vector<string> labels;
    for (int i = 0; i < n; i++) {
        if (i < 26) {
            labels.push_back(string(1, 'A' + i));
        }
        else {
            labels.push_back("V" + to_string(i + 1));
        }
    }
    return labels;
}

void GraphGenerator::saveGraphToFile(const vector<vector<double>>& graph,
    const vector<string>& labels,
    const string& filename) {
    ofstream file(filename);

    if (!file.is_open()) {
        cerr << "Cannot open file for writing: " << filename << endl;
        return;
    }

    // Записываем заголовок с метками
    for (size_t i = 0; i < labels.size(); i++) {
        file << labels[i];
        if (i < labels.size() - 1) file << ",";
    }
    file << "\n";

    // Записываем матрицу смежности
    for (const auto& row : graph) {
        for (size_t j = 0; j < row.size(); j++) {
            file << row[j];
            if (j < row.size() - 1) file << ",";
        }
        file << "\n";
    }
}