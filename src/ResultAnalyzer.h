#pragma once
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace std;

struct TestResults {
    string testName;
    double bestPathLength;
    double executionTime;
    int vertices;
    int edges;
    bool foundPath;
    int iterations;
    string bestPathSequence;
};

struct AlgorithmStats {
    string name;
    double avgTime;
    double totalTime;
    double successRate;
    int totalTests;
    int successfulTests;

    double smallGraphsSuccess;
    double mediumGraphsSuccess;
    double largeGraphsSuccess;

    double sparseGraphsSuccess;
    double denseGraphsSuccess;

    double starGraphsSuccess;
    double pathGraphsSuccess;
    double gridGraphsSuccess;
    double completeGraphsSuccess;
};

class ResultsAnalyzer {
private:
    map<string, vector<TestResults>> algorithmResults;
    vector<string> algorithmNames;

public:
    void loadResults(const string& acoFile, const string& dijkstraFile,
        const string& astarFile, const string& floydFile) {
        algorithmResults["ACO"] = loadCSV(acoFile);
        algorithmResults["Dijkstra"] = loadCSV(dijkstraFile);
        algorithmResults["A*"] = loadCSV(astarFile);
        algorithmResults["Floyd-Warshall"] = loadCSV(floydFile);

        algorithmNames = { "ACO", "Dijkstra", "A*", "Floyd-Warshall" };
    }

    void generateComparativeAnalysis() {
        cout << "=== COMPREHENSIVE ALGORITHM COMPARISON ===" << endl << endl;

        printPerformanceStatistics();
        cout << endl;

        printSolutionQuality();
        cout << endl;

        printGraphTypeAnalysis();
        cout << endl;

        printSpecialGraphsAnalysis();
        cout << endl;

        printAlgorithmRankings();
        cout << endl;

        saveDetailedCSV("detailed_comparison.csv");
        saveSummaryCSV("summary_comparison.csv");
    }

private:
    vector<TestResults> loadCSV(const string& filename) {
        vector<TestResults> results;
        ifstream file(filename);
        string line;

        if (!file.is_open()) {
            cerr << "Cannot open file: " << filename << endl;
            return results;
        }

        getline(file, line);

        while (getline(file, line)) {
            stringstream ss(line);
            TestResults result;
            string token;

            getline(ss, result.testName, ',');
            getline(ss, token, ','); result.vertices = stoi(token);
            getline(ss, token, ','); result.edges = stoi(token);
            getline(ss, token, ','); result.executionTime = stod(token);
            getline(ss, token, ','); result.bestPathLength = stod(token);

            string foundStr;
            getline(ss, foundStr, ',');
            result.foundPath = (foundStr == "true");

            getline(ss, token, ','); result.iterations = stoi(token);
            getline(ss, result.bestPathSequence, ',');

            if (!result.bestPathSequence.empty() && result.bestPathSequence[0] == '"') {
                result.bestPathSequence = result.bestPathSequence.substr(1, result.bestPathSequence.length() - 2);
            }

            results.push_back(result);
        }

        file.close();
        return results;
    }

    void printPerformanceStatistics() {
        cout << "GENERAL PERFORMANCE STATISTICS" << endl;
        cout << "===========================================" << endl;

        cout << setw(15) << "Algorithm" << setw(12) << "Avg Time"
            << setw(12) << "Total Time" << setw(10) << "Success%"
            << setw(10) << "Tests" << setw(12) << "Successful" << endl;
        cout << string(70, '-') << endl;

        for (const auto& algoName : algorithmNames) {
            const auto& results = algorithmResults[algoName];
            AlgorithmStats stats = calculateAlgorithmStats(algoName, results);

            cout << setw(15) << algoName
                << setw(12) << fixed << setprecision(4) << stats.avgTime << "s"
                << setw(12) << fixed << setprecision(3) << stats.totalTime << "s"
                << setw(10) << fixed << setprecision(1) << stats.successRate << "%"
                << setw(10) << stats.totalTests
                << setw(12) << stats.successfulTests << endl;
        }
    }

    void printSolutionQuality() {
        cout << "COMPARISON OF QUALITY" << endl;
        cout << "=================================" << endl;

        const auto& dijkstraResults = algorithmResults["Dijkstra"];

        cout << setw(15) << "Algorithm" << setw(15) << "Avg Path Length"
            << setw(15) << "Same as Dijkstra" << setw(15) << "Optimality %" << endl;
        cout << string(60, '-') << endl;

        for (const auto& algoName : algorithmNames) {
            if (algoName == "Dijkstra") continue;

            const auto& results = algorithmResults[algoName];
            auto comparison = compareWithDijkstra(results, dijkstraResults);

            cout << setw(15) << algoName
                << setw(15) << fixed << setprecision(3) << comparison.avgPathLength
                << setw(15) << comparison.samePathCount << "/" << comparison.totalComparisons
                << setw(15) << fixed << setprecision(1) << comparison.optimalityRate << "%" << endl;
        }
    }

    void printGraphTypeAnalysis() {
        cout << "ANALYSIS BY GRAPH TYPES" << endl;
        cout << "=============================" << endl;

        cout << setw(15) << "Algorithm" << setw(12) << "Small(5-15)"
            << setw(12) << "Medium(16-50)" << setw(12) << "Large(51+)"
            << setw(12) << "Sparse" << setw(12) << "Dense" << endl;
        cout << string(75, '-') << endl;

        for (const auto& algoName : algorithmNames) {
            const auto& results = algorithmResults[algoName];
            AlgorithmStats stats = calculateAlgorithmStats(algoName, results);

            cout << setw(15) << algoName
                << setw(12) << fixed << setprecision(1) << stats.smallGraphsSuccess << "%"
                << setw(12) << fixed << setprecision(1) << stats.mediumGraphsSuccess << "%"
                << setw(12) << fixed << setprecision(1) << stats.largeGraphsSuccess << "%"
                << setw(12) << fixed << setprecision(1) << stats.sparseGraphsSuccess << "%"
                << setw(12) << fixed << setprecision(1) << stats.denseGraphsSuccess << "%" << endl;
        }
    }

    void printSpecialGraphsAnalysis() {
        cout << "=====================================" << endl;

        cout << setw(15) << "Algorithm" << setw(12) << "Star"
            << setw(12) << "Path" << setw(12) << "Grid"
            << setw(12) << "Complete" << endl;
        cout << string(63, '-') << endl;

        for (const auto& algoName : algorithmNames) {
            const auto& results = algorithmResults[algoName];
            AlgorithmStats stats = calculateAlgorithmStats(algoName, results);

            cout << setw(15) << algoName
                << setw(12) << fixed << setprecision(1) << stats.starGraphsSuccess << "%"
                << setw(12) << fixed << setprecision(1) << stats.pathGraphsSuccess << "%"
                << setw(12) << fixed << setprecision(1) << stats.gridGraphsSuccess << "%"
                << setw(12) << fixed << setprecision(1) << stats.completeGraphsSuccess << "%" << endl;
        }
    }

    void printAlgorithmRankings() {
        cout << "5. ALGORITHM RANKING" << endl;
        cout << "==============================" << endl;

        vector<pair<string, double>> speedScores;
        vector<pair<string, double>> reliabilityScores;
        vector<pair<string, double>> accuracyScores;

        for (const auto& algoName : algorithmNames) {
            const auto& results = algorithmResults[algoName];
            AlgorithmStats stats = calculateAlgorithmStats(algoName, results);

            speedScores.push_back({ algoName, 1.0 / (stats.avgTime + 0.001) });

            reliabilityScores.push_back({ algoName, stats.successRate });

            if (algoName == "Dijkstra") {
                accuracyScores.push_back({ algoName, 100.0 });
            }
            else {
                const auto& dijkstraResults = algorithmResults["Dijkstra"];
                auto comparison = compareWithDijkstra(results, dijkstraResults);
                accuracyScores.push_back({ algoName, comparison.optimalityRate });
            }
        }

        auto sortDesc = [](const auto& a, const auto& b) { return a.second > b.second; };
        sort(speedScores.begin(), speedScores.end(), sortDesc);
        sort(reliabilityScores.begin(), reliabilityScores.end(), sortDesc);
        sort(accuracyScores.begin(), accuracyScores.end(), sortDesc);

        cout << "Speed:    ";
        for (int i = 0; i < 4; i++) {
            cout << (i + 1) << ". " << speedScores[i].first;
            if (i < 3) cout << ", ";
        }
        cout << endl;

        cout << "Reliability:  ";
        for (int i = 0; i < 4; i++) {
            cout << (i + 1) << ". " << reliabilityScores[i].first;
            if (i < 3) cout << ", ";
        }
        cout << endl;

        cout << "Accuracy:    ";
        for (int i = 0; i < 4; i++) {
            cout << (i + 1) << ". " << accuracyScores[i].first;
            if (i < 3) cout << ", ";
        }
        cout << endl;
    }

    AlgorithmStats calculateAlgorithmStats(const string& algoName, const vector<TestResults>& results) {
        AlgorithmStats stats;
        stats.name = algoName;
        stats.totalTests = results.size();

        double totalTime = 0.0;
        int successful = 0;

        int smallSuccess = 0, smallTotal = 0;
        int mediumSuccess = 0, mediumTotal = 0;
        int largeSuccess = 0, largeTotal = 0;
        int sparseSuccess = 0, sparseTotal = 0;
        int denseSuccess = 0, denseTotal = 0;
        int starSuccess = 0, starTotal = 0;
        int pathSuccess = 0, pathTotal = 0;
        int gridSuccess = 0, gridTotal = 0;
        int completeSuccess = 0, completeTotal = 0;

        for (const auto& result : results) {
            totalTime += result.executionTime;

            if (result.foundPath) {
                successful++;
            }

            if (result.vertices <= 15) {
                smallTotal++;
                if (result.foundPath) smallSuccess++;
            }
            else if (result.vertices <= 50) {
                mediumTotal++;
                if (result.foundPath) mediumSuccess++;
            }
            else {
                largeTotal++;
                if (result.foundPath) largeSuccess++;
            }

            double density = (2.0 * result.edges) / (result.vertices * (result.vertices - 1));
            if (density < 0.3) {
                sparseTotal++;
                if (result.foundPath) sparseSuccess++;
            }
            else {
                denseTotal++;
                if (result.foundPath) denseSuccess++;
            }

            if (result.testName.find("_star_") != string::npos) {
                starTotal++;
                if (result.foundPath) starSuccess++;
            }
            else if (result.testName.find("_path_") != string::npos) {
                pathTotal++;
                if (result.foundPath) pathSuccess++;
            }
            else if (result.testName.find("_grid_") != string::npos) {
                gridTotal++;
                if (result.foundPath) gridSuccess++;
            }
            else if (result.testName.find("_complete_") != string::npos) {
                completeTotal++;
                if (result.foundPath) completeSuccess++;
            }
        }

        stats.avgTime = stats.totalTests > 0 ? totalTime / stats.totalTests : 0.0;
        stats.totalTime = totalTime;
        stats.successfulTests = successful;
        stats.successRate = stats.totalTests > 0 ? (100.0 * successful) / stats.totalTests : 0.0;

        stats.smallGraphsSuccess = smallTotal > 0 ? (100.0 * smallSuccess) / smallTotal : 0.0;
        stats.mediumGraphsSuccess = mediumTotal > 0 ? (100.0 * mediumSuccess) / mediumTotal : 0.0;
        stats.largeGraphsSuccess = largeTotal > 0 ? (100.0 * largeSuccess) / largeTotal : 0.0;

        stats.sparseGraphsSuccess = sparseTotal > 0 ? (100.0 * sparseSuccess) / sparseTotal : 0.0;
        stats.denseGraphsSuccess = denseTotal > 0 ? (100.0 * denseSuccess) / denseTotal : 0.0;

        stats.starGraphsSuccess = starTotal > 0 ? (100.0 * starSuccess) / starTotal : 0.0;
        stats.pathGraphsSuccess = pathTotal > 0 ? (100.0 * pathSuccess) / pathTotal : 0.0;
        stats.gridGraphsSuccess = gridTotal > 0 ? (100.0 * gridSuccess) / gridTotal : 0.0;
        stats.completeGraphsSuccess = completeTotal > 0 ? (100.0 * completeSuccess) / completeTotal : 0.0;

        return stats;
    }

    struct ComparisonResult {
        double avgPathLength;
        int samePathCount;
        int totalComparisons;
        double optimalityRate;
    };

    ComparisonResult compareWithDijkstra(const vector<TestResults>& algoResults,
        const vector<TestResults>& dijkstraResults) {
        ComparisonResult result;
        result.totalComparisons = 0;
        result.samePathCount = 0;
        double totalPathLength = 0.0;
        int validComparisons = 0;

        map<string, TestResults> dijkstraMap;
        for (const auto& dr : dijkstraResults) {
            dijkstraMap[dr.testName] = dr;
        }

        for (const auto& ar : algoResults) {
            if (dijkstraMap.count(ar.testName)) {
                const auto& dr = dijkstraMap[ar.testName];

                if (ar.foundPath && dr.foundPath) {
                    result.totalComparisons++;
                    totalPathLength += ar.bestPathLength;

                    if (abs(ar.bestPathLength - dr.bestPathLength) < 1e-6) {
                        result.samePathCount++;
                    }
                }
            }
        }

        result.avgPathLength = validComparisons > 0 ? totalPathLength / validComparisons : 0.0;
        result.optimalityRate = result.totalComparisons > 0 ?
            (100.0 * result.samePathCount) / result.totalComparisons : 0.0;

        return result;
    }

    void saveDetailedCSV(const string& filename) {
        ofstream file(filename);
        file << "Algorithm,TestType,SuccessRate,AvgTime,TotalTime,TotalTests,SuccessfulTests" << endl;

        for (const auto& algoName : algorithmNames) {
            const auto& results = algorithmResults[algoName];
            AlgorithmStats stats = calculateAlgorithmStats(algoName, results);

            file << algoName << ",Overall," << stats.successRate << ","
                << stats.avgTime << "," << stats.totalTime << ","
                << stats.totalTests << "," << stats.successfulTests << endl;
        }

        file.close();
    }

    void saveSummaryCSV(const string& filename) {
        ofstream file(filename);
        file << "Metric,ACO,Dijkstra,A*,Floyd-Warshall" << endl;

        vector<AlgorithmStats> allStats;
        for (const auto& algoName : algorithmNames) {
            allStats.push_back(calculateAlgorithmStats(algoName, algorithmResults[algoName]));
        }

        file << "SuccessRate," << allStats[0].successRate << "," << allStats[1].successRate
            << "," << allStats[2].successRate << "," << allStats[3].successRate << endl;
        file << "AvgTime," << allStats[0].avgTime << "," << allStats[1].avgTime
            << "," << allStats[2].avgTime << "," << allStats[3].avgTime << endl;
        file << "TotalTime," << allStats[0].totalTime << "," << allStats[1].totalTime
            << "," << allStats[2].totalTime << "," << allStats[3].totalTime << endl;

        file.close();
    }
};