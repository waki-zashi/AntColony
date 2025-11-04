#pragma once
#include "TestRunner.h"
#include <vector>
#include <string>
#include <limits>
#include <algorithm>

using namespace std;

struct FloydWarshallResult {
    vector<int> bestPath;
    double bestLength;
    bool pathFound;

    FloydWarshallResult() : bestLength(numeric_limits<double>::max()), pathFound(false) {}
};

class FloydWarshall {
public:
    static FloydWarshallResult findShortestPath(const vector<vector<double>>& graph,
        const vector<string>& labels,
        int start, int end) {
        FloydWarshallResult result;
        int n = graph.size();

        // Матрицы расстояний и предков
        vector<vector<double>> dist(n, vector<double>(n, numeric_limits<double>::max()));
        vector<vector<int>> next(n, vector<int>(n, -1));

        // Инициализация матриц
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (i == j) {
                    dist[i][j] = 0;
                }
                else if (graph[i][j] > 0) {
                    dist[i][j] = graph[i][j];
                    next[i][j] = j;
                }
            }
        }

        // Алгоритм Флойда-Уоршелла
        for (int k = 0; k < n; k++) {
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    if (dist[i][k] < numeric_limits<double>::max() &&
                        dist[k][j] < numeric_limits<double>::max()) {
                        if (dist[i][j] > dist[i][k] + dist[k][j]) {
                            dist[i][j] = dist[i][k] + dist[k][j];
                            next[i][j] = next[i][k];
                        }
                    }
                }
            }
        }

        // Проверяем существование пути
        if (dist[start][end] < numeric_limits<double>::max()) {
            result.pathFound = true;
            result.bestLength = dist[start][end];
            result.bestPath = reconstructPath(next, start, end);
        }

        return result;
    }

private:
    static vector<int> reconstructPath(const vector<vector<int>>& next, int start, int end) {
        if (next[start][end] == -1) {
            return {};
        }

        vector<int> path;
        path.push_back(start);

        int current = start;
        while (current != end) {
            current = next[current][end];
            path.push_back(current);
        }

        return path;
    }
};

class FloydWarshallTestRunner : public TestRunner {
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

        // Запуск алгоритма Флойда-Уоршелла с замером времени
        auto startTime = chrono::high_resolution_clock::now();

        FloydWarshallResult result = FloydWarshall::findShortestPath(graph, labels, start, end);

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
        testResult.iterations = 1; // Флойд-Уоршелл всегда за 1 "итерацию"
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

    void runTestSuite(const string& testDirectory) override {
        cout << "=== Floyd Warshall Algorithm Test Suite ===" << endl;
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
        saveResultsToCSV("fw_results.csv");
    }
};