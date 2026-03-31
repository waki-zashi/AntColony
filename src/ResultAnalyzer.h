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
#include <filesystem>

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
    double avgTime = 0.0;
    double totalTime = 0.0;
    double successRate = 0.0;
    int totalTests = 0;
    int successfulTests = 0;

    double smallGraphsSuccess = 0.0;
    double mediumGraphsSuccess = 0.0;
    double largeGraphsSuccess = 0.0;

    double sparseGraphsSuccess = 0.0;
    double denseGraphsSuccess = 0.0;

    double starGraphsSuccess = 0.0;
    double pathGraphsSuccess = 0.0;
    double gridGraphsSuccess = 0.0;
    double completeGraphsSuccess = 0.0;
};

class ResultsAnalyzer {
private:
    struct ComparisonResult {
        double avgPathLength = 0.0;
        int samePathCount = 0;
        int totalComparisons = 0;
        double optimalityRate = 0.0;
    };

    map<string, vector<TestResults>> algorithmResults;
    vector<string> algorithmNames;
    string resultsDir = "results";

public:
    explicit ResultsAnalyzer(const string& resultsDirectory = "results")
        : resultsDir(resultsDirectory) {}

    void loadResults(const string& acoFile,
                     const string& dijkstraFile,
                     const string& astarFile,
                     const string& floydFile) {
        algorithmResults.clear();

        algorithmResults["ACO"] = loadCSV(joinPath(resultsDir, acoFile));
        algorithmResults["Dijkstra"] = loadCSV(joinPath(resultsDir, dijkstraFile));
        algorithmResults["A*"] = loadCSV(joinPath(resultsDir, astarFile));
        algorithmResults["Floyd-Warshall"] = loadCSV(joinPath(resultsDir, floydFile));

        algorithmNames = { "ACO", "Dijkstra", "A*", "Floyd-Warshall" };
    }

    bool hasAllRequiredResults() const {
        for (const auto& algoName : algorithmNames) {
            auto it = algorithmResults.find(algoName);
            if (it == algorithmResults.end() || it->second.empty()) {
                return false;
            }
        }
        return true;
    }

    void generateComparativeAnalysis() {
        cout << "=== COMPREHENSIVE ALGORITHM COMPARISON ===" << endl << endl;

        if (!hasAllRequiredResults()) {
            cerr << "Error: not all result files were loaded successfully." << endl;
            cerr << "Expected non-empty results for ACO, Dijkstra, A*, and Floyd-Warshall." << endl;
            return;
        }

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

        saveDetailedCSV(joinPath(resultsDir, "detailed_comparison.csv"));
        saveSummaryCSV(joinPath(resultsDir, "summary_comparison.csv"));
    }

private:
    static string joinPath(const string& left, const string& right) {
        namespace fs = std::filesystem;
        return (fs::path(left) / fs::path(right)).string();
    }

    vector<TestResults> loadCSV(const string& filename) {
        vector<TestResults> results;
        ifstream file(filename);
        string line;

        if (!file.is_open()) {
            cerr << "Cannot open file: " << filename << endl;
            return results;
        }

        getline(file, line); // header

        while (getline(file, line)) {
            if (line.empty()) {
                continue;
            }

            stringstream ss(line);
            TestResults result;
            string token;

            getline(ss, result.testName, ',');

            getline(ss, token, ',');
            result.vertices = token.empty() ? 0 : stoi(token);

            getline(ss, token, ',');
            result.edges = token.empty() ? 0 : stoi(token);

            getline(ss, token, ',');
            result.executionTime = token.empty() ? 0.0 : stod(token);

            getline(ss, token, ',');
            result.bestPathLength = token.empty() ? 0.0 : stod(token);

            string foundStr;
            getline(ss, foundStr, ',');
            result.foundPath = (foundStr == "true");

            getline(ss, token, ',');
            result.iterations = token.empty() ? 0 : stoi(token);

            getline(ss, result.bestPathSequence);

            if (!result.bestPathSequence.empty() &&
                result.bestPathSequence.front() == '"' &&
                result.bestPathSequence.back() == '"') {
                result.bestPathSequence =
                    result.bestPathSequence.substr(1, result.bestPathSequence.size() - 2);
            }

            results.push_back(result);
        }

        return results;
    }

    void printPerformanceStatistics() {
        cout << "GENERAL PERFORMANCE STATISTICS" << endl;
        cout << "===========================================" << endl;

        cout << setw(15) << "Algorithm"
             << setw(12) << "Avg Time"
             << setw(12) << "Total Time"
             << setw(10) << "Success%"
             << setw(10) << "Tests"
             << setw(12) << "Successful" << endl;
        cout << string(70, '-') << endl;

        for (const auto& algoName : algorithmNames) {
            const auto& results = algorithmResults[algoName];
            const AlgorithmStats stats = calculateAlgorithmStats(algoName, results);

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

        cout << setw(15) << "Algorithm"
             << setw(15) << "Avg Path Length"
             << setw(18) << "Same as Dijkstra"
             << setw(15) << "Optimality %" << endl;
        cout << string(68, '-') << endl;

        for (const auto& algoName : algorithmNames) {
            if (algoName == "Dijkstra") {
                continue;
            }

            const auto& results = algorithmResults[algoName];
            const ComparisonResult comparison = compareWithDijkstra(results, dijkstraResults);

            cout << setw(15) << algoName
                 << setw(15) << fixed << setprecision(3) << comparison.avgPathLength
                 << setw(18) << (to_string(comparison.samePathCount) + "/" + to_string(comparison.totalComparisons))
                 << setw(15) << fixed << setprecision(1) << comparison.optimalityRate << "%" << endl;
        }
    }

    void printGraphTypeAnalysis() {
        cout << "ANALYSIS BY GRAPH TYPES" << endl;
        cout << "=============================" << endl;

        cout << setw(15) << "Algorithm"
             << setw(12) << "Small(5-15)"
             << setw(14) << "Medium(16-50)"
             << setw(12) << "Large(51+)"
             << setw(12) << "Sparse"
             << setw(12) << "Dense" << endl;
        cout << string(77, '-') << endl;

        for (const auto& algoName : algorithmNames) {
            const AlgorithmStats stats = calculateAlgorithmStats(algoName, algorithmResults[algoName]);

            cout << setw(15) << algoName
                 << setw(12) << fixed << setprecision(1) << stats.smallGraphsSuccess << "%"
                 << setw(14) << fixed << setprecision(1) << stats.mediumGraphsSuccess << "%"
                 << setw(12) << fixed << setprecision(1) << stats.largeGraphsSuccess << "%"
                 << setw(12) << fixed << setprecision(1) << stats.sparseGraphsSuccess << "%"
                 << setw(12) << fixed << setprecision(1) << stats.denseGraphsSuccess << "%" << endl;
        }
    }

    void printSpecialGraphsAnalysis() {
        cout << "=====================================" << endl;

        cout << setw(15) << "Algorithm"
             << setw(12) << "Star"
             << setw(12) << "Path"
             << setw(12) << "Grid"
             << setw(12) << "Complete" << endl;
        cout << string(63, '-') << endl;

        for (const auto& algoName : algorithmNames) {
            const AlgorithmStats stats = calculateAlgorithmStats(algoName, algorithmResults[algoName]);

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
            const AlgorithmStats stats = calculateAlgorithmStats(algoName, results);

            speedScores.push_back({ algoName, 1.0 / (stats.avgTime + 1e-9) });
            reliabilityScores.push_back({ algoName, stats.successRate });

            if (algoName == "Dijkstra") {
                accuracyScores.push_back({ algoName, 100.0 });
            } else {
                const auto comparison = compareWithDijkstra(results, algorithmResults["Dijkstra"]);
                accuracyScores.push_back({ algoName, comparison.optimalityRate });
            }
        }

        auto sortDesc = [](const auto& a, const auto& b) {
            if (fabs(a.second - b.second) > 1e-9) {
                return a.second > b.second;
            }
            return a.first < b.first;
        };

        sort(speedScores.begin(), speedScores.end(), sortDesc);
        sort(reliabilityScores.begin(), reliabilityScores.end(), sortDesc);
        sort(accuracyScores.begin(), accuracyScores.end(), sortDesc);

        cout << "Speed:       ";
        for (size_t i = 0; i < speedScores.size(); ++i) {
            cout << (i + 1) << ". " << speedScores[i].first;
            if (i + 1 < speedScores.size()) cout << ", ";
        }
        cout << endl;

        cout << "Reliability: ";
        for (size_t i = 0; i < reliabilityScores.size(); ++i) {
            cout << (i + 1) << ". " << reliabilityScores[i].first;
            if (i + 1 < reliabilityScores.size()) cout << ", ";
        }
        cout << endl;

        cout << "Accuracy:    ";
        for (size_t i = 0; i < accuracyScores.size(); ++i) {
            cout << (i + 1) << ". " << accuracyScores[i].first;
            if (i + 1 < accuracyScores.size()) cout << ", ";
        }
        cout << endl;
    }

    AlgorithmStats calculateAlgorithmStats(const string& algoName,
                                           const vector<TestResults>& results) {
        AlgorithmStats stats;
        stats.name = algoName;
        stats.totalTests = static_cast<int>(results.size());

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
            } else if (result.vertices <= 50) {
                mediumTotal++;
                if (result.foundPath) mediumSuccess++;
            } else {
                largeTotal++;
                if (result.foundPath) largeSuccess++;
            }

            if (result.vertices > 1) {
                const double density =
                    (2.0 * result.edges) / (result.vertices * (result.vertices - 1));
                if (density < 0.3) {
                    sparseTotal++;
                    if (result.foundPath) sparseSuccess++;
                } else {
                    denseTotal++;
                    if (result.foundPath) denseSuccess++;
                }
            }

            if (result.testName.find("_star_") != string::npos) {
                starTotal++;
                if (result.foundPath) starSuccess++;
            } else if (result.testName.find("_path_") != string::npos) {
                pathTotal++;
                if (result.foundPath) pathSuccess++;
            } else if (result.testName.find("_grid_") != string::npos) {
                gridTotal++;
                if (result.foundPath) gridSuccess++;
            } else if (result.testName.find("_complete_") != string::npos) {
                completeTotal++;
                if (result.foundPath) completeSuccess++;
            }
        }

        stats.totalTime = totalTime;
        stats.successfulTests = successful;
        stats.avgTime = stats.totalTests > 0 ? totalTime / stats.totalTests : 0.0;
        stats.successRate = stats.totalTests > 0
            ? (100.0 * successful) / stats.totalTests
            : 0.0;

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

    ComparisonResult compareWithDijkstra(const vector<TestResults>& algoResults,
                                         const vector<TestResults>& dijkstraResults) {
        ComparisonResult result;

        map<string, TestResults> dijkstraMap;
        for (const auto& dr : dijkstraResults) {
            dijkstraMap[dr.testName] = dr;
        }

        double totalPathLength = 0.0;
        int validComparisons = 0;

        for (const auto& ar : algoResults) {
            auto it = dijkstraMap.find(ar.testName);
            if (it == dijkstraMap.end()) {
                continue;
            }

            const auto& dr = it->second;

            if (ar.foundPath && dr.foundPath) {
                result.totalComparisons++;
                totalPathLength += ar.bestPathLength;
                validComparisons++;

                if (fabs(ar.bestPathLength - dr.bestPathLength) < 1e-6) {
                    result.samePathCount++;
                }
            }
        }

        result.avgPathLength = validComparisons > 0
            ? totalPathLength / validComparisons
            : 0.0;

        result.optimalityRate = result.totalComparisons > 0
            ? (100.0 * result.samePathCount) / result.totalComparisons
            : 0.0;

        return result;
    }

    void saveDetailedCSV(const string& filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "Cannot open file for writing: " << filename << endl;
            return;
        }

        file << "Algorithm,TestType,SuccessRate,AvgTime,TotalTime,TotalTests,SuccessfulTests\n";

        for (const auto& algoName : algorithmNames) {
            const auto& results = algorithmResults[algoName];
            const AlgorithmStats stats = calculateAlgorithmStats(algoName, results);

            file << algoName << ",Overall,"
                 << stats.successRate << ","
                 << stats.avgTime << ","
                 << stats.totalTime << ","
                 << stats.totalTests << ","
                 << stats.successfulTests << "\n";
        }
    }

    void saveSummaryCSV(const string& filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "Cannot open file for writing: " << filename << endl;
            return;
        }

        file << "Metric,ACO,Dijkstra,A*,Floyd-Warshall\n";

        vector<AlgorithmStats> allStats;
        for (const auto& algoName : algorithmNames) {
            allStats.push_back(calculateAlgorithmStats(algoName, algorithmResults[algoName]));
        }

        file << "SuccessRate,"
             << allStats[0].successRate << ","
             << allStats[1].successRate << ","
             << allStats[2].successRate << ","
             << allStats[3].successRate << "\n";

        file << "AvgTime,"
             << allStats[0].avgTime << ","
             << allStats[1].avgTime << ","
             << allStats[2].avgTime << ","
             << allStats[3].avgTime << "\n";

        file << "TotalTime,"
             << allStats[0].totalTime << ","
             << allStats[1].totalTime << ","
             << allStats[2].totalTime << ","
             << allStats[3].totalTime << "\n";
    }
};