#pragma once
#include "TestRunner.h"
#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <queue>
#include <algorithm>

using namespace std;

struct DijkstraResult {
    vector<int> bestPath;
    double bestLength;
    bool pathFound;

    DijkstraResult() : bestLength(numeric_limits<double>::max()), pathFound(false) {}
};

class Dijkstra {
public:
    static DijkstraResult findShortestPath(const vector<vector<double>>& graph,
        const vector<string>& labels,
        int start, int end) {
        DijkstraResult result;
        int n = graph.size();

        // Массивы для алгоритма Дейкстры
        vector<double> dist(n, numeric_limits<double>::max());
        vector<int> prev(n, -1);
        vector<bool> visited(n, false);

        // Приоритетная очередь: (расстояние, вершина)
        priority_queue<pair<double, int>,
            vector<pair<double, int>>,
            greater<pair<double, int>>> pq;

        // Инициализация
        dist[start] = 0.0;
        pq.push({ 0.0, start });

        while (!pq.empty()) {
            // Извлекаем вершину с минимальным расстоянием
            double currentDist = pq.top().first;
            int current = pq.top().second;
            pq.pop();

            // Если уже посетили эту вершину, пропускаем
            if (visited[current]) continue;
            visited[current] = true;

            // Если дошли до конечной вершины, можно выйти
            if (current == end) break;

            // Обходим всех соседей
            for (int neighbor = 0; neighbor < n; neighbor++) {
                if (graph[current][neighbor] > 0 && !visited[neighbor]) {
                    double newDist = currentDist + graph[current][neighbor];

                    if (newDist < dist[neighbor]) {
                        dist[neighbor] = newDist;
                        prev[neighbor] = current;
                        pq.push({ newDist, neighbor });
                    }
                }
            }
        }

        // Восстанавливаем путь
        if (dist[end] < numeric_limits<double>::max()) {
            result.pathFound = true;
            result.bestLength = dist[end];

            // Восстанавливаем путь от конца к началу
            vector<int> path;
            for (int v = end; v != -1; v = prev[v]) {
                path.push_back(v);
            }
            reverse(path.begin(), path.end());
            result.bestPath = path;
        }

        return result;
    }
};

class DijkstraTestRunner : public TestRunner {
public:
    void runSingleTest(const std::string& graphFile, const std::string& testName) override {
        // Загрузка графа
        bool fileLoaded;
        std::vector<std::vector<double>> graph;
        std::vector<std::string> labels;

        readGraphFromFile(graphFile, fileLoaded, graph, labels);

        if (!fileLoaded || graph.empty()) {
            std::cerr << "  Failed to load graph: " << graphFile << std::endl;
            return;
        }

        int n = labels.size();
        if (n == 0) {
            std::cerr << "  Empty graph: " << graphFile << std::endl;
            return;
        }

        // Выбор случайных start и end (разные вершины)
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, n - 1);

        int start = dist(gen);
        int end;
        do {
            end = dist(gen);
        } while (end == start);

        std::cout << "  Path: " << labels[start] << " -> " << labels[end];
        std::cout << " (vertices: " << n << ", edges: " << countEdges(graph) << ")" << std::endl;

        // Запуск алгоритма Дейкстры с замером времени
        auto startTime = std::chrono::high_resolution_clock::now();

        DijkstraResult result = Dijkstra::findShortestPath(graph, labels, start, end);

        auto endTime = std::chrono::high_resolution_clock::now();
        double executionTime = std::chrono::duration<double>(endTime - startTime).count();

        // Формируем строку с путем
        std::string pathSequence = "NO_PATH";
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
        testResult.iterations = 1; // Дейкстра всегда за 1 "итерацию"
        testResult.bestPathSequence = pathSequence;

        results.push_back(testResult);

        std::cout << "  Result: time=" << executionTime << "s, length=";

        if (result.pathFound) {
            std::cout << result.bestLength;
        }
        else {
            std::cout << "NO_PATH";
        }

        std::cout << ", found=" << (result.pathFound ? "yes" : "no") << std::endl;
    }

    void runTestSuite(const string& testDirectory) override {
        cout << "=== Dijkstra Algorithm Test Suite ===" << endl;
        cout << "Looking for test files in: " << testDirectory << endl;

        // Читаем список файлов из сгенерированного списка
        vector<string> testFiles = readTestFilesList(testDirectory);

        if (testFiles.empty()) {
            cout << "ERROR: No test files list found or list is empty!" << endl;
            cout << "Please run generate_test_suite first to create test graphs." << endl;
            return;
        }

        cout << "Found " << testFiles.size() << " test files in the list." << endl;

        int testCount = 0;
        const int maxTests = min(100, (int)testFiles.size()); // Ограничиваем 100 тестами

        // Запускаем тесты для каждого файла из списка
        for (size_t i = 0; i < testFiles.size() && testCount < maxTests; i++) {
            string filename = testFiles[i];
            string fullPath = testDirectory + "/" + filename;

            // Проверяем что файл действительно существует
            if (!fileExists(fullPath)) {
                cout << "Warning: File from list not found: " << fullPath << endl;
                continue;
            }

            // Извлекаем имя теста (убираем расширение .csv)
            string testName = filename;
            size_t dotPos = testName.find_last_of(".");
            if (dotPos != string::npos) {
                testName = testName.substr(0, dotPos);
            }

            cout << "[" << (testCount + 1) << "] Running: " << testName << endl;
            runSingleTest(fullPath, testName);
            testCount++;
        }

        if (testCount == 0) {
            cout << "ERROR: No valid test files found!" << endl;
            cout << "Files from list exist but cannot be loaded." << endl;
            return;
        }

        cout << "\n=== Completed " << testCount << " tests ===" << endl;
        printSummary();
        saveResultsToCSV("dijkstra_results.csv");
    }
};