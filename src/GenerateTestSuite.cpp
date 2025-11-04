//#include "GraphGenerator.h"
//#include <iostream>
//#include <fstream>
//#include <vector>
//
//using namespace std;
//
//string testDir = "test_cases";
//const int maxAttempts = 10; // Максимум попыток на один граф
//int testCount = 0;
//vector<string> generatedTestFiles; // Глобальный массив для хранения имен файлов
//
//void tryGenerateConnected(int countVertex, double density, const string graphType, GraphGenerator& generator) {
//    bool connected = false;
//    vector<vector<double>> graph;
//    vector<string> labels;
//    int attempts = 0;
//
//    // Пытаемся сгенерировать связный граф
//    while (!connected && attempts < maxAttempts) {
//        graph = generator.generateConnectedRandomGraph(countVertex, density);
//        labels = generator.generateLabels(countVertex);
//        connected = generator.isConnected(graph);
//        attempts++;
//    }
//
//    if (connected) {
//        // Сохраняем только если граф связный
//        string filename = "test_" + to_string(testCount) +
//            graphType + to_string(countVertex) + "v_d" +
//            to_string((int)(density * 100)) + ".csv";
//        string fullPath = testDir + "/" + filename;
//
//        generator.saveGraphToFile(graph, labels, fullPath);
//        generatedTestFiles.push_back(filename); // Добавляем в список
//        testCount++;
//
//        cout << "  Created: " << filename;
//        if (attempts > 1) {
//            cout << " (attempts: " << attempts << ")";
//        }
//        cout << endl;
//    }
//    else {
//        cout << "  Failed to generate connected graph for n=" << countVertex << ", density=" << density << endl;
//    }
//}
//
//void saveFilesList() {
//    string listFilePath = testDir + "/test_files_list.txt";
//    ofstream listFile(listFilePath);
//
//    if (!listFile.is_open()) {
//        cerr << "Error: Cannot create file list: " << listFilePath << endl;
//        return;
//    }
//
//    // Записываем все имена файлов
//    for (const auto& filename : generatedTestFiles) {
//        listFile << filename << "\n";
//    }
//
//    listFile.close();
//    cout << "Test files list saved to: " << listFilePath << endl;
//    cout << "Total files in list: " << generatedTestFiles.size() << endl;
//}
//
//void generateConnectedTestSuite() {
//    GraphGenerator generator(42);
//
//    // Очищаем список файлов перед генерацией
//    generatedTestFiles.clear();
//
//    // Создаем директорию для тестов
//    system(("mkdir " + testDir).c_str());
//
//    cout << "Generating CONNECTED test graphs..." << endl;
//
//    // 1. Случайные связные графы разных размеров и плотностей
//    cout << "1. Random connected graphs..." << endl;
//
//    // Маленькие графы
//    for (int n = 5; n <= 15; n += 2) {
//        for (double density : {0.3, 0.6, 0.9}) {
//            tryGenerateConnected(n, density, "_small_", generator);
//        }
//    }
//
//    // Средние графы
//    for (int n = 16; n <= 50; n += 5) {
//        for (double density : {0.4, 0.8}) {
//            tryGenerateConnected(n, density, "_medium_", generator);
//        }
//    }
//
//    // Большие графы
//    for (int n = 51; n <= 100; n += 5) {
//        tryGenerateConnected(n, 0.4, "_big_", generator);
//    }
//
//    // 2. Специальные типы связных графов
//    cout << "2. Special connected graphs..." << endl;
//
//    // Звездные графы
//    for (int n = 6; n <= 20; n += 4) {
//        auto graph = generator.generateStarGraph(n);
//        auto labels = generator.generateLabels(n);
//
//        string filename = "test_" + to_string(testCount) + "_star_" + to_string(n) + "v.csv";
//        string fullPath = testDir + "/" + filename;
//
//        generator.saveGraphToFile(graph, labels, fullPath);
//        generatedTestFiles.push_back(filename);
//        testCount++;
//        cout << "  Created: " << filename << endl;
//    }
//
//    // Линейные графы (пути)
//    for (int n = 8; n <= 25; n += 5) {
//        auto graph = generator.generatePathGraph(n);
//        auto labels = generator.generateLabels(n);
//
//        string filename = "test_" + to_string(testCount) + "_path_" + to_string(n) + "v.csv";
//        string fullPath = testDir + "/" + filename;
//
//        generator.saveGraphToFile(graph, labels, fullPath);
//        generatedTestFiles.push_back(filename);
//        testCount++;
//        cout << "  Created: " << filename << endl;
//    }
//
//    // Сеточные графы
//    for (int rows = 3; rows <= 6; rows++) {
//        for (int cols = 3; cols <= 6; cols++) {
//            auto graph = generator.generateGridGraph(rows, cols);
//            auto labels = generator.generateLabels(rows * cols);
//
//            string filename = "test_" + to_string(testCount) + "_grid_" + to_string(rows) + "x" + to_string(cols) + ".csv";
//            string fullPath = testDir + "/" + filename;
//
//            generator.saveGraphToFile(graph, labels, fullPath);
//            generatedTestFiles.push_back(filename);
//            testCount++;
//            cout << "  Created: " << filename << endl;
//        }
//    }
//
//    // Полные графы
//    for (int n = 5; n <= 15; n += 3) {
//        auto graph = generator.generateCompleteGraph(n);
//        auto labels = generator.generateLabels(n);
//
//        string filename = "test_" + to_string(testCount) + "_complete_" + to_string(n) + "v.csv";
//        string fullPath = testDir + "/" + filename;
//
//        generator.saveGraphToFile(graph, labels, fullPath);
//        generatedTestFiles.push_back(filename);
//        testCount++;
//        cout << "  Created: " << filename << endl;
//    }
//
//    // Сохраняем список всех созданных файлов
//    saveFilesList();
//
//    cout << "\n=== GENERATION COMPLETE ===" << endl;
//    cout << "Total graphs generated: " << testCount << endl;
//    cout << "Files in list: " << generatedTestFiles.size() << endl;
//    cout << "Test directory: " << testDir << endl;
//    cout << "File list: " << testDir << "/test_files_list.txt" << endl;
//
//    // Выводим примеры созданных файлов
//    if (!generatedTestFiles.empty()) {
//        cout << "\nExample files:" << endl;
//        for (int i = 0; i < min(5, (int)generatedTestFiles.size()); i++) {
//            cout << "  " << generatedTestFiles[i] << endl;
//        }
//        if (generatedTestFiles.size() > 5) {
//            cout << "  ... and " << (generatedTestFiles.size() - 5) << " more" << endl;
//        }
//    }
//}
//
//int main() {
//    generateConnectedTestSuite();
//    return 0;
//}