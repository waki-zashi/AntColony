#include "GenerateTestSuite.h"
#include "GraphGenerator.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

static const int maxAttempts = 10;

void tryGenerateConnected(int countVertex,
                          double density,
                          const string& graphType,
                          GraphGenerator& generator,
                          const string& testDir,
                          int& testCount,
                          vector<string>& generatedTestFiles) {
    bool connected = false;
    vector<vector<double>> graph;
    vector<string> labels;
    int attempts = 0;

    while (!connected && attempts < maxAttempts) {
        graph = generator.generateConnectedRandomGraph(countVertex, density);
        labels = generator.generateLabels(countVertex);
        connected = generator.isConnected(graph);
        attempts++;
    }

    if (connected) {
        string filename = "test_" + to_string(testCount) +
                          graphType + to_string(countVertex) + "v_d" +
                          to_string(static_cast<int>(density * 100)) + ".csv";
        string fullPath = testDir + "/" + filename;

        generator.saveGraphToFile(graph, labels, fullPath);
        generatedTestFiles.push_back(filename);
        testCount++;

        cout << "  Created: " << filename;
        if (attempts > 1) {
            cout << " (attempts: " << attempts << ")";
        }
        cout << endl;
    } else {
        cout << "  Failed to generate connected graph for n=" << countVertex
             << ", density=" << density << endl;
    }
}

void saveFilesList(const string& testDir,
                   const vector<string>& generatedTestFiles) {
    const string listFilePath = testDir + "/test_files_list.txt";
    ofstream listFile(listFilePath);

    if (!listFile.is_open()) {
        cerr << "Error: Cannot create file list: " << listFilePath << endl;
        return;
    }

    for (const auto& filename : generatedTestFiles) {
        listFile << filename << "\n";
    }

    listFile.close();
    cout << "Test files list saved to: " << listFilePath << endl;
    cout << "Total files in list: " << generatedTestFiles.size() << endl;
}

static void generateBase70Graphs(const string& testDir,
                                 GraphGenerator& generator,
                                 int& testCount,
                                 vector<string>& generatedTestFiles) {
    cout << "1. Random connected graphs..." << endl;

    for (int n = 5; n <= 15; n += 2) {
        for (double density : {0.3, 0.6, 0.9}) {
            tryGenerateConnected(n, density, "_small_", generator, testDir, testCount, generatedTestFiles);
        }
    }

    for (int n = 16; n <= 50; n += 5) {
        for (double density : {0.4, 0.8}) {
            tryGenerateConnected(n, density, "_medium_", generator, testDir, testCount, generatedTestFiles);
        }
    }

    for (int n = 51; n <= 100; n += 5) {
        tryGenerateConnected(n, 0.4, "_big_", generator, testDir, testCount, generatedTestFiles);
    }

    cout << "2. Special connected graphs..." << endl;

    for (int n = 6; n <= 20; n += 4) {
        auto graph = generator.generateStarGraph(n);
        auto labels = generator.generateLabels(n);

        string filename = "test_" + to_string(testCount) + "_star_" + to_string(n) + "v.csv";
        string fullPath = testDir + "/" + filename;

        generator.saveGraphToFile(graph, labels, fullPath);
        generatedTestFiles.push_back(filename);
        testCount++;
        cout << "  Created: " << filename << endl;
    }

    for (int n = 8; n <= 25; n += 5) {
        auto graph = generator.generatePathGraph(n);
        auto labels = generator.generateLabels(n);

        string filename = "test_" + to_string(testCount) + "_path_" + to_string(n) + "v.csv";
        string fullPath = testDir + "/" + filename;

        generator.saveGraphToFile(graph, labels, fullPath);
        generatedTestFiles.push_back(filename);
        testCount++;
        cout << "  Created: " << filename << endl;
    }

    for (int rows = 3; rows <= 6; rows++) {
        for (int cols = 3; cols <= 6; cols++) {
            auto graph = generator.generateGridGraph(rows, cols);
            auto labels = generator.generateLabels(rows * cols);

            string filename = "test_" + to_string(testCount) + "_grid_" +
                              to_string(rows) + "x" + to_string(cols) + ".csv";
            string fullPath = testDir + "/" + filename;

            generator.saveGraphToFile(graph, labels, fullPath);
            generatedTestFiles.push_back(filename);
            testCount++;
            cout << "  Created: " << filename << endl;
        }
    }

    for (int n = 5; n <= 15; n += 3) {
        auto graph = generator.generateCompleteGraph(n);
        auto labels = generator.generateLabels(n);

        string filename = "test_" + to_string(testCount) + "_complete_" + to_string(n) + "v.csv";
        string fullPath = testDir + "/" + filename;

        generator.saveGraphToFile(graph, labels, fullPath);
        generatedTestFiles.push_back(filename);
        testCount++;
        cout << "  Created: " << filename << endl;
    }
}

static void generateExtra30LargeGraphs(const string& testDir,
                                       GraphGenerator& generator,
                                       int& testCount,
                                       vector<string>& generatedTestFiles) {
    cout << "3. Extra large connected graphs..." << endl;

    const vector<int> sizes = {
        110, 120, 130, 140, 150,
        160, 170, 180, 190, 200
    };

    const vector<double> densities = {0.20, 0.35, 0.50};

    for (int n : sizes) {
        for (double density : densities) {
            tryGenerateConnected(n, density, "_large_", generator, testDir, testCount, generatedTestFiles);
        }
    }
}

void generateConnectedTestSuite(const string& testDir) {
    GraphGenerator generator(42);

    int testCount = 0;
    vector<string> generatedTestFiles;

    fs::create_directories(testDir);

    cout << "Generating CONNECTED test graphs..." << endl;
    cout << "Output directory: " << testDir << endl;

    generateBase70Graphs(testDir, generator, testCount, generatedTestFiles);

    saveFilesList(testDir, generatedTestFiles);

    cout << "\n=== GENERATION COMPLETE ===" << endl;
    cout << "Total graphs generated: " << testCount << endl;
    cout << "Files in list: " << generatedTestFiles.size() << endl;
    cout << "Test directory: " << testDir << endl;
    cout << "File list: " << testDir << "/test_files_list.txt" << endl;
}

void generateLargeTestSuite(const string& testDir) {
    GraphGenerator generator(42);

    int testCount = 0;
    vector<string> generatedTestFiles;

    fs::create_directories(testDir);

    cout << "Generating LARGE connected test graphs..." << endl;
    cout << "Output directory: " << testDir << endl;
    cout << "Target: exactly 30 graphs" << endl;

    generateExtra30LargeGraphs(testDir, generator, testCount, generatedTestFiles);

    saveFilesList(testDir, generatedTestFiles);

    cout << "\n=== LARGE GENERATION COMPLETE ===" << endl;
    cout << "Total graphs generated: " << testCount << endl;
    cout << "Files in list: " << generatedTestFiles.size() << endl;
    cout << "Test directory: " << testDir << endl;
    cout << "File list: " << testDir << "/test_files_list.txt" << endl;

    if (testCount != 30) {
        cout << "Warning: expected 30 graphs, but generated " << testCount << endl;
    }
}

void generateFullTestSuite(const string& testDir) {
    GraphGenerator generator(42);

    int testCount = 0;
    vector<string> generatedTestFiles;

    fs::create_directories(testDir);

    cout << "Generating FULL test suite..." << endl;
    cout << "Output directory: " << testDir << endl;
    cout << "Target: 70 base graphs + 30 large graphs = 100 total" << endl;

    generateBase70Graphs(testDir, generator, testCount, generatedTestFiles);
    generateExtra30LargeGraphs(testDir, generator, testCount, generatedTestFiles);

    saveFilesList(testDir, generatedTestFiles);

    cout << "\n=== FULL GENERATION COMPLETE ===" << endl;
    cout << "Total graphs generated: " << testCount << endl;
    cout << "Files in list: " << generatedTestFiles.size() << endl;
    cout << "Test directory: " << testDir << endl;
    cout << "File list: " << testDir << "/test_files_list.txt" << endl;

    if (testCount != 100) {
        cout << "Warning: expected 100 graphs, but generated " << testCount << endl;
    }

    if (!generatedTestFiles.empty()) {
        cout << "\nExample files:" << endl;
        for (int i = 0; i < min(5, static_cast<int>(generatedTestFiles.size())); i++) {
            cout << "  " << generatedTestFiles[i] << endl;
        }
        if (generatedTestFiles.size() > 5) {
            cout << "  ... and " << (generatedTestFiles.size() - 5) << " more" << endl;
        }
    }
}