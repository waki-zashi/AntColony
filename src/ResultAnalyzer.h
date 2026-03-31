#pragma once

#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <filesystem>
#include <limits>

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

struct NumericStats {
    int count = 0;
    double mean = 0.0;
    double median = 0.0;
    double min = 0.0;
    double max = 0.0;
    double stddev = 0.0;
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
    double hugeGraphsSuccess = 0.0;

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
        double avgRelativeErrorPct = 0.0;
        double maxRelativeErrorPct = 0.0;
        double avgExcessLength = 0.0;
        double avgTimeRatioVsDijkstra = 0.0;
        double medianTimeRatioVsDijkstra = 0.0;
    };

    struct AggregateRow {
        string scopeType;
        string scopeValue;
        string algorithm;

        int tests = 0;
        int successful = 0;
        double successRate = 0.0;

        NumericStats timeStats;
        NumericStats pathStats;
        NumericStats iterationStats;

        int comparableTests = 0;
        int optimalMatches = 0;
        double optimalityRate = 0.0;
        double avgRelativeErrorPct = 0.0;
        double maxRelativeErrorPct = 0.0;
        double avgExcessLength = 0.0;
        double avgTimeRatioVsDijkstra = 0.0;
        double medianTimeRatioVsDijkstra = 0.0;
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

        algorithmNames = {"ACO", "Dijkstra", "A*", "Floyd-Warshall"};
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
        savePerTestComparisonCSV(joinPath(resultsDir, "per_test_comparison.csv"));
        saveCategoryComparisonCSV(joinPath(resultsDir, "category_comparison.csv"));
        saveRankingsCSV(joinPath(resultsDir, "algorithm_rankings.csv"));
    }

private:
    static string joinPath(const string& left, const string& right) {
        namespace fs = std::filesystem;
        return (fs::path(left) / fs::path(right)).string();
    }

    static bool almostEqual(double a, double b, double eps = 1e-6) {
        return fabs(a - b) < eps;
    }

    static double safePercent(double part, double total) {
        if (total <= 0.0) return 0.0;
        return 100.0 * part / total;
    }

    static double safeRatio(double num, double den) {
        if (fabs(den) < 1e-12) return 0.0;
        return num / den;
    }

    static string csvAlgorithmKey(const string& algoName) {
        if (algoName == "ACO") return "ACO";
        if (algoName == "Dijkstra") return "Dijkstra";
        if (algoName == "A*") return "AStar";
        if (algoName == "Floyd-Warshall") return "FloydWarshall";
        return algoName;
    }

    static string detectSizeCategory(const TestResults& r) {
        if (r.vertices <= 15) return "Small(5-15)";
        if (r.vertices <= 50) return "Medium(16-50)";
        if (r.vertices <= 100) return "Large(51-100)";
        return "Huge(101+)";
    }

    static string detectDensityCategory(const TestResults& r) {
        if (r.vertices <= 1) return "Unknown";
        const double density = (2.0 * r.edges) / (r.vertices * (r.vertices - 1.0));
        return (density < 0.3) ? "Sparse" : "Dense";
    }

    static string detectFamilyCategory(const TestResults& r) {
        if (r.testName.find("_star_") != string::npos) return "Star";
        if (r.testName.find("_path_") != string::npos) return "Path";
        if (r.testName.find("_grid_") != string::npos) return "Grid";
        if (r.testName.find("_complete_") != string::npos) return "Complete";
        return "Random";
    }

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

    NumericStats computeNumericStats(vector<double> values) const {
        NumericStats stats;
        stats.count = static_cast<int>(values.size());

        if (values.empty()) {
            return stats;
        }

        sort(values.begin(), values.end());

        stats.min = values.front();
        stats.max = values.back();

        double sum = 0.0;
        for (double v : values) {
            sum += v;
        }
        stats.mean = sum / values.size();

        if (values.size() % 2 == 1) {
            stats.median = values[values.size() / 2];
        } else {
            const size_t i = values.size() / 2;
            stats.median = (values[i - 1] + values[i]) / 2.0;
        }

        double var = 0.0;
        for (double v : values) {
            const double d = v - stats.mean;
            var += d * d;
        }
        var /= values.size();
        stats.stddev = sqrt(var);

        return stats;
    }

    map<string, TestResults> buildResultMap(const vector<TestResults>& results) const {
        map<string, TestResults> m;
        for (const auto& r : results) {
            m[r.testName] = r;
        }
        return m;
    }

    vector<string> getAllTestNames() const {
        set<string> names;
        for (const auto& [algo, results] : algorithmResults) {
            for (const auto& r : results) {
                names.insert(r.testName);
            }
        }
        return vector<string>(names.begin(), names.end());
    }

    ComparisonResult compareWithDijkstra(const vector<TestResults>& algoResults,
                                         const vector<TestResults>& dijkstraResults) {
        ComparisonResult result;

        map<string, TestResults> dijkstraMap = buildResultMap(dijkstraResults);

        double totalPathLength = 0.0;
        int validComparisons = 0;

        vector<double> relErrors;
        vector<double> excessLengths;
        vector<double> timeRatios;

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

                if (almostEqual(ar.bestPathLength, dr.bestPathLength)) {
                    result.samePathCount++;
                }

                const double excess = ar.bestPathLength - dr.bestPathLength;
                excessLengths.push_back(excess);

                double relError = 0.0;
                if (dr.bestPathLength > 1e-12) {
                    relError = (excess / dr.bestPathLength) * 100.0;
                    if (relError < 0.0) relError = 0.0;
                }
                relErrors.push_back(relError);

                if (dr.executionTime > 1e-12) {
                    timeRatios.push_back(ar.executionTime / dr.executionTime);
                }
            }
        }

        result.avgPathLength = validComparisons > 0 ? totalPathLength / validComparisons : 0.0;
        result.optimalityRate = result.totalComparisons > 0
            ? (100.0 * result.samePathCount) / result.totalComparisons
            : 0.0;

        if (!relErrors.empty()) {
            double sumRel = 0.0;
            double maxRel = 0.0;
            for (double v : relErrors) {
                sumRel += v;
                maxRel = max(maxRel, v);
            }
            result.avgRelativeErrorPct = sumRel / relErrors.size();
            result.maxRelativeErrorPct = maxRel;
        }

        if (!excessLengths.empty()) {
            double sumExcess = 0.0;
            for (double v : excessLengths) {
                sumExcess += v;
            }
            result.avgExcessLength = sumExcess / excessLengths.size();
        }

        if (!timeRatios.empty()) {
            double sum = 0.0;
            for (double v : timeRatios) sum += v;
            result.avgTimeRatioVsDijkstra = sum / timeRatios.size();

            sort(timeRatios.begin(), timeRatios.end());
            if (timeRatios.size() % 2 == 1) {
                result.medianTimeRatioVsDijkstra = timeRatios[timeRatios.size() / 2];
            } else {
                const size_t i = timeRatios.size() / 2;
                result.medianTimeRatioVsDijkstra = (timeRatios[i - 1] + timeRatios[i]) / 2.0;
            }
        }

        return result;
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
        int hugeSuccess = 0, hugeTotal = 0;

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

            const string sizeCat = detectSizeCategory(result);
            if (sizeCat == "Small(5-15)") {
                smallTotal++;
                if (result.foundPath) smallSuccess++;
            } else if (sizeCat == "Medium(16-50)") {
                mediumTotal++;
                if (result.foundPath) mediumSuccess++;
            } else if (sizeCat == "Large(51-100)") {
                largeTotal++;
                if (result.foundPath) largeSuccess++;
            } else {
                hugeTotal++;
                if (result.foundPath) hugeSuccess++;
            }

            const string densityCat = detectDensityCategory(result);
            if (densityCat == "Sparse") {
                sparseTotal++;
                if (result.foundPath) sparseSuccess++;
            } else if (densityCat == "Dense") {
                denseTotal++;
                if (result.foundPath) denseSuccess++;
            }

            const string familyCat = detectFamilyCategory(result);
            if (familyCat == "Star") {
                starTotal++;
                if (result.foundPath) starSuccess++;
            } else if (familyCat == "Path") {
                pathTotal++;
                if (result.foundPath) pathSuccess++;
            } else if (familyCat == "Grid") {
                gridTotal++;
                if (result.foundPath) gridSuccess++;
            } else if (familyCat == "Complete") {
                completeTotal++;
                if (result.foundPath) completeSuccess++;
            }
        }

        stats.totalTime = totalTime;
        stats.successfulTests = successful;
        stats.avgTime = stats.totalTests > 0 ? totalTime / stats.totalTests : 0.0;
        stats.successRate = stats.totalTests > 0 ? (100.0 * successful) / stats.totalTests : 0.0;

        stats.smallGraphsSuccess = smallTotal > 0 ? (100.0 * smallSuccess) / smallTotal : 0.0;
        stats.mediumGraphsSuccess = mediumTotal > 0 ? (100.0 * mediumSuccess) / mediumTotal : 0.0;
        stats.largeGraphsSuccess = largeTotal > 0 ? (100.0 * largeSuccess) / largeTotal : 0.0;
        stats.hugeGraphsSuccess = hugeTotal > 0 ? (100.0 * hugeSuccess) / hugeTotal : 0.0;

        stats.sparseGraphsSuccess = sparseTotal > 0 ? (100.0 * sparseSuccess) / sparseTotal : 0.0;
        stats.denseGraphsSuccess = denseTotal > 0 ? (100.0 * denseSuccess) / denseTotal : 0.0;

        stats.starGraphsSuccess = starTotal > 0 ? (100.0 * starSuccess) / starTotal : 0.0;
        stats.pathGraphsSuccess = pathTotal > 0 ? (100.0 * pathSuccess) / pathTotal : 0.0;
        stats.gridGraphsSuccess = gridTotal > 0 ? (100.0 * gridSuccess) / gridTotal : 0.0;
        stats.completeGraphsSuccess = completeTotal > 0 ? (100.0 * completeSuccess) / completeTotal : 0.0;

        return stats;
    }

    AggregateRow buildAggregateRow(const string& algorithm,
                                   const vector<TestResults>& filteredAlgo,
                                   const vector<TestResults>& filteredDijkstra,
                                   const string& scopeType,
                                   const string& scopeValue) {
        AggregateRow row;
        row.scopeType = scopeType;
        row.scopeValue = scopeValue;
        row.algorithm = algorithm;
        row.tests = static_cast<int>(filteredAlgo.size());

        vector<double> times;
        vector<double> pathLengths;
        vector<double> iterations;

        for (const auto& r : filteredAlgo) {
            times.push_back(r.executionTime);
            iterations.push_back(static_cast<double>(r.iterations));
            if (r.foundPath) {
                row.successful++;
                pathLengths.push_back(r.bestPathLength);
            }
        }

        row.successRate = row.tests > 0 ? (100.0 * row.successful) / row.tests : 0.0;
        row.timeStats = computeNumericStats(times);
        row.pathStats = computeNumericStats(pathLengths);
        row.iterationStats = computeNumericStats(iterations);

        if (algorithm == "Dijkstra") {
            row.comparableTests = row.successful;
            row.optimalMatches = row.successful;
            row.optimalityRate = row.successRate;
            row.avgRelativeErrorPct = 0.0;
            row.maxRelativeErrorPct = 0.0;
            row.avgExcessLength = 0.0;
            row.avgTimeRatioVsDijkstra = 1.0;
            row.medianTimeRatioVsDijkstra = 1.0;
            return row;
        }

        const ComparisonResult cmp = compareWithDijkstra(filteredAlgo, filteredDijkstra);
        row.comparableTests = cmp.totalComparisons;
        row.optimalMatches = cmp.samePathCount;
        row.optimalityRate = cmp.optimalityRate;
        row.avgRelativeErrorPct = cmp.avgRelativeErrorPct;
        row.maxRelativeErrorPct = cmp.maxRelativeErrorPct;
        row.avgExcessLength = cmp.avgExcessLength;
        row.avgTimeRatioVsDijkstra = cmp.avgTimeRatioVsDijkstra;
        row.medianTimeRatioVsDijkstra = cmp.medianTimeRatioVsDijkstra;
        return row;
    }

    template <typename Predicate>
    vector<TestResults> filterResults(const vector<TestResults>& results, Predicate pred) const {
        vector<TestResults> out;
        for (const auto& r : results) {
            if (pred(r)) out.push_back(r);
        }
        return out;
    }

    vector<AggregateRow> buildAllAggregateRows() {
        vector<AggregateRow> rows;
        const auto& dijkstraAll = algorithmResults["Dijkstra"];

        struct ScopeDef {
            string type;
            string value;
        };

        vector<ScopeDef> scopes = {
            {"Overall", "All"},
            {"Size", "Small(5-15)"},
            {"Size", "Medium(16-50)"},
            {"Size", "Large(51-100)"},
            {"Size", "Huge(101+)"},
            {"Density", "Sparse"},
            {"Density", "Dense"},
            {"Family", "Random"},
            {"Family", "Star"},
            {"Family", "Path"},
            {"Family", "Grid"},
            {"Family", "Complete"}
        };

        for (const auto& scope : scopes) {
            for (const auto& algo : algorithmNames) {
                const auto& algoResults = algorithmResults[algo];

                auto pred = [&](const TestResults& r) -> bool {
                    if (scope.type == "Overall") return true;
                    if (scope.type == "Size") return detectSizeCategory(r) == scope.value;
                    if (scope.type == "Density") return detectDensityCategory(r) == scope.value;
                    if (scope.type == "Family") return detectFamilyCategory(r) == scope.value;
                    return false;
                };

                vector<TestResults> filteredAlgo = filterResults(algoResults, pred);
                vector<TestResults> filteredDijkstra = filterResults(dijkstraAll, pred);

                rows.push_back(buildAggregateRow(algo, filteredAlgo, filteredDijkstra, scope.type, scope.value));
            }
        }

        return rows;
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
        cout << "===============================================================" << endl;

        const auto& dijkstraResults = algorithmResults["Dijkstra"];

        cout << setw(15) << "Algorithm"
             << setw(15) << "Avg Length"
             << setw(18) << "Same as Dijkstra"
             << setw(15) << "Optimality %"
             << setw(14) << "Avg RelErr%" << endl;
        cout << string(77, '-') << endl;

        for (const auto& algoName : algorithmNames) {
            if (algoName == "Dijkstra") {
                continue;
            }

            const auto& results = algorithmResults[algoName];
            const ComparisonResult comparison = compareWithDijkstra(results, dijkstraResults);

            cout << setw(15) << algoName
                 << setw(15) << fixed << setprecision(3) << comparison.avgPathLength
                 << setw(18) << (to_string(comparison.samePathCount) + "/" + to_string(comparison.totalComparisons))
                 << setw(15) << fixed << setprecision(1) << comparison.optimalityRate << "%"
                 << setw(14) << fixed << setprecision(3) << comparison.avgRelativeErrorPct << endl;
        }
    }

    void printGraphTypeAnalysis() {
        cout << "ANALYSIS BY GRAPH TYPES" << endl;
        cout << "===============================================" << endl;

        cout << setw(15) << "Algorithm"
             << setw(12) << "Small(5-15)"
             << setw(14) << "Medium(16-50)"
             << setw(14) << "Large(51-100)"
             << setw(12) << "Huge(101+)"
             << setw(12) << "Sparse"
             << setw(12) << "Dense" << endl;
        cout << string(91, '-') << endl;

        for (const auto& algoName : algorithmNames) {
            const AlgorithmStats stats = calculateAlgorithmStats(algoName, algorithmResults[algoName]);

            cout << setw(15) << algoName
                 << setw(12) << fixed << setprecision(1) << stats.smallGraphsSuccess << "%"
                 << setw(14) << fixed << setprecision(1) << stats.mediumGraphsSuccess << "%"
                 << setw(14) << fixed << setprecision(1) << stats.largeGraphsSuccess << "%"
                 << setw(12) << fixed << setprecision(1) << stats.hugeGraphsSuccess << "%"
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
        vector<pair<string, double>> errorScores;

        for (const auto& algoName : algorithmNames) {
            const auto& results = algorithmResults[algoName];
            const AlgorithmStats stats = calculateAlgorithmStats(algoName, results);

            speedScores.push_back({algoName, 1.0 / (stats.avgTime + 1e-9)});
            reliabilityScores.push_back({algoName, stats.successRate});

            if (algoName == "Dijkstra") {
                accuracyScores.push_back({algoName, 100.0});
                errorScores.push_back({algoName, 0.0});
            } else {
                const auto comparison = compareWithDijkstra(results, algorithmResults["Dijkstra"]);
                accuracyScores.push_back({algoName, comparison.optimalityRate});
                errorScores.push_back({algoName, -comparison.avgRelativeErrorPct});
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
        sort(errorScores.begin(), errorScores.end(), sortDesc);

        auto printOrderedList = [](const string& label,
                                const vector<pair<string, double>>& scores,
                                bool inverseTimeMetric = false,
                                bool negateValue = false,
                                const string& suffix = "") {
            cout << left << setw(13) << (label + ":");

            for (size_t i = 0; i < scores.size(); ++i) {
                double shownValue = scores[i].second;

                if (inverseTimeMetric) {
                    shownValue = 1.0 / scores[i].second;
                }
                if (negateValue) {
                    shownValue = -scores[i].second;
                }

                cout << scores[i].first
                    << " (" << fixed << setprecision(4) << shownValue << suffix << ")";

                if (i + 1 < scores.size()) {
                    cout << ", ";
                }
            }
            cout << endl;
        };

        printOrderedList("Speed", speedScores, true, false, "s");
        printOrderedList("Reliability", reliabilityScores, false, false, "%");
        printOrderedList("Accuracy", accuracyScores, false, false, "%");
        printOrderedList("Error", errorScores, false, true, "%");
    }

    void saveDetailedCSV(const string& filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "Cannot open file for writing: " << filename << endl;
            return;
        }

        file << "ScopeType,ScopeValue,Algorithm,Tests,Successful,SuccessRate,"
             << "AvgTime,MedianTime,StdTime,MinTime,MaxTime,"
             << "AvgPathLength,MedianPathLength,StdPathLength,MinPathLength,MaxPathLength,"
             << "AvgIterations,MedianIterations,StdIterations,MinIterations,MaxIterations,"
             << "ComparableTests,OptimalMatches,OptimalityRate,"
             << "AvgRelativeErrorPct,MaxRelativeErrorPct,AvgExcessLength,"
             << "AvgTimeRatioVsDijkstra,MedianTimeRatioVsDijkstra\n";

        const vector<AggregateRow> rows = buildAllAggregateRows();
        for (const auto& row : rows) {
            file << row.scopeType << ","
                 << row.scopeValue << ","
                 << row.algorithm << ","
                 << row.tests << ","
                 << row.successful << ","
                 << row.successRate << ","
                 << row.timeStats.mean << ","
                 << row.timeStats.median << ","
                 << row.timeStats.stddev << ","
                 << row.timeStats.min << ","
                 << row.timeStats.max << ","
                 << row.pathStats.mean << ","
                 << row.pathStats.median << ","
                 << row.pathStats.stddev << ","
                 << row.pathStats.min << ","
                 << row.pathStats.max << ","
                 << row.iterationStats.mean << ","
                 << row.iterationStats.median << ","
                 << row.iterationStats.stddev << ","
                 << row.iterationStats.min << ","
                 << row.iterationStats.max << ","
                 << row.comparableTests << ","
                 << row.optimalMatches << ","
                 << row.optimalityRate << ","
                 << row.avgRelativeErrorPct << ","
                 << row.maxRelativeErrorPct << ","
                 << row.avgExcessLength << ","
                 << row.avgTimeRatioVsDijkstra << ","
                 << row.medianTimeRatioVsDijkstra << "\n";
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

        auto cmpACO = compareWithDijkstra(algorithmResults["ACO"], algorithmResults["Dijkstra"]);
        auto cmpASTAR = compareWithDijkstra(algorithmResults["A*"], algorithmResults["Dijkstra"]);
        auto cmpFW = compareWithDijkstra(algorithmResults["Floyd-Warshall"], algorithmResults["Dijkstra"]);

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

        file << "OptimalityRate,"
             << cmpACO.optimalityRate << ","
             << 100.0 << ","
             << cmpASTAR.optimalityRate << ","
             << cmpFW.optimalityRate << "\n";

        file << "AvgRelativeErrorPct,"
             << cmpACO.avgRelativeErrorPct << ","
             << 0.0 << ","
             << cmpASTAR.avgRelativeErrorPct << ","
             << cmpFW.avgRelativeErrorPct << "\n";

        file << "MaxRelativeErrorPct,"
             << cmpACO.maxRelativeErrorPct << ","
             << 0.0 << ","
             << cmpASTAR.maxRelativeErrorPct << ","
             << cmpFW.maxRelativeErrorPct << "\n";

        file << "AvgTimeRatioVsDijkstra,"
             << cmpACO.avgTimeRatioVsDijkstra << ","
             << 1.0 << ","
             << cmpASTAR.avgTimeRatioVsDijkstra << ","
             << cmpFW.avgTimeRatioVsDijkstra << "\n";
    }

    void savePerTestComparisonCSV(const string& filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "Cannot open file for writing: " << filename << endl;
            return;
        }

        file << "TestName,Vertices,Edges,SizeCategory,DensityCategory,FamilyCategory,"
             << "DijkstraFound,DijkstraLength,DijkstraTime,DijkstraIterations,"
             << "ACOFound,ACOLength,ACOTime,ACOIterations,ACOOptimal,ACORelativeErrorPct,"
             << "AStarFound,AStarLength,AStarTime,AStarIterations,AStarOptimal,AStarRelativeErrorPct,"
             << "FloydWarshallFound,FloydWarshallLength,FloydWarshallTime,FloydWarshallIterations,FloydWarshallOptimal,FloydWarshallRelativeErrorPct\n";

        const auto djMap = buildResultMap(algorithmResults["Dijkstra"]);
        const auto acoMap = buildResultMap(algorithmResults["ACO"]);
        const auto astarMap = buildResultMap(algorithmResults["A*"]);
        const auto fwMap = buildResultMap(algorithmResults["Floyd-Warshall"]);

        vector<string> allTests = getAllTestNames();

        for (const auto& testName : allTests) {
            auto itd = djMap.find(testName);
            if (itd == djMap.end()) continue;

            const auto& dr = itd->second;
            const string sizeCat = detectSizeCategory(dr);
            const string densityCat = detectDensityCategory(dr);
            const string familyCat = detectFamilyCategory(dr);

            auto writeAlgo = [&](const map<string, TestResults>& m) {
                auto it = m.find(testName);
                if (it == m.end()) {
                    file << "false,0,0,0,false,0";
                    return;
                }

                const auto& r = it->second;
                bool optimal = false;
                double relErr = 0.0;

                if (r.foundPath && dr.foundPath) {
                    optimal = almostEqual(r.bestPathLength, dr.bestPathLength);
                    if (dr.bestPathLength > 1e-12) {
                        relErr = max(0.0, (r.bestPathLength - dr.bestPathLength) / dr.bestPathLength * 100.0);
                    }
                }

                file << (r.foundPath ? "true" : "false") << ","
                     << r.bestPathLength << ","
                     << r.executionTime << ","
                     << r.iterations << ","
                     << (optimal ? "true" : "false") << ","
                     << relErr;
            };

            file << testName << ","
                 << dr.vertices << ","
                 << dr.edges << ","
                 << sizeCat << ","
                 << densityCat << ","
                 << familyCat << ","
                 << (dr.foundPath ? "true" : "false") << ","
                 << dr.bestPathLength << ","
                 << dr.executionTime << ","
                 << dr.iterations << ",";

            writeAlgo(acoMap);
            file << ",";
            writeAlgo(astarMap);
            file << ",";
            writeAlgo(fwMap);
            file << "\n";
        }
    }

    void saveCategoryComparisonCSV(const string& filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "Cannot open file for writing: " << filename << endl;
            return;
        }

        file << "ScopeType,ScopeValue,Metric,ACO,Dijkstra,A*,Floyd-Warshall\n";

        const vector<AggregateRow> rows = buildAllAggregateRows();

        auto findRow = [&](const string& scopeType, const string& scopeValue, const string& algo) -> AggregateRow {
            for (const auto& row : rows) {
                if (row.scopeType == scopeType &&
                    row.scopeValue == scopeValue &&
                    row.algorithm == algo) {
                    return row;
                }
            }
            return AggregateRow{};
        };

        vector<pair<string, string>> scopes = {
            {"Overall", "All"},
            {"Size", "Small(5-15)"},
            {"Size", "Medium(16-50)"},
            {"Size", "Large(51-100)"},
            {"Size", "Huge(101+)"},
            {"Density", "Sparse"},
            {"Density", "Dense"},
            {"Family", "Random"},
            {"Family", "Star"},
            {"Family", "Path"},
            {"Family", "Grid"},
            {"Family", "Complete"}
        };

        for (const auto& scope : scopes) {
            const auto aco = findRow(scope.first, scope.second, "ACO");
            const auto dj = findRow(scope.first, scope.second, "Dijkstra");
            const auto astar = findRow(scope.first, scope.second, "A*");
            const auto fw = findRow(scope.first, scope.second, "Floyd-Warshall");

            file << scope.first << "," << scope.second << ",SuccessRate,"
                 << aco.successRate << "," << dj.successRate << "," << astar.successRate << "," << fw.successRate << "\n";

            file << scope.first << "," << scope.second << ",AvgTime,"
                 << aco.timeStats.mean << "," << dj.timeStats.mean << "," << astar.timeStats.mean << "," << fw.timeStats.mean << "\n";

            file << scope.first << "," << scope.second << ",MedianTime,"
                 << aco.timeStats.median << "," << dj.timeStats.median << "," << astar.timeStats.median << "," << fw.timeStats.median << "\n";

            file << scope.first << "," << scope.second << ",OptimalityRate,"
                 << aco.optimalityRate << "," << dj.optimalityRate << "," << astar.optimalityRate << "," << fw.optimalityRate << "\n";

            file << scope.first << "," << scope.second << ",AvgRelativeErrorPct,"
                 << aco.avgRelativeErrorPct << "," << dj.avgRelativeErrorPct << "," << astar.avgRelativeErrorPct << "," << fw.avgRelativeErrorPct << "\n";

            file << scope.first << "," << scope.second << ",AvgIterations,"
                 << aco.iterationStats.mean << "," << dj.iterationStats.mean << "," << astar.iterationStats.mean << "," << fw.iterationStats.mean << "\n";
        }
    }

    void saveRankingsCSV(const string& filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "Cannot open file for writing: " << filename << endl;
            return;
        }

        vector<pair<string, double>> speedScores;
        vector<pair<string, double>> reliabilityScores;
        vector<pair<string, double>> accuracyScores;
        vector<pair<string, double>> errorScores;

        for (const auto& algoName : algorithmNames) {
            const auto stats = calculateAlgorithmStats(algoName, algorithmResults[algoName]);
            speedScores.push_back({algoName, 1.0 / (stats.avgTime + 1e-9)});
            reliabilityScores.push_back({algoName, stats.successRate});

            if (algoName == "Dijkstra") {
                accuracyScores.push_back({algoName, 100.0});
                errorScores.push_back({algoName, 0.0});
            } else {
                const auto cmp = compareWithDijkstra(algorithmResults[algoName], algorithmResults["Dijkstra"]);
                accuracyScores.push_back({algoName, cmp.optimalityRate});
                errorScores.push_back({algoName, -cmp.avgRelativeErrorPct});
            }
        }

        auto sortDesc = [](const auto& a, const auto& b) {
            if (fabs(a.second - b.second) > 1e-9) return a.second > b.second;
            return a.first < b.first;
        };

        sort(speedScores.begin(), speedScores.end(), sortDesc);
        sort(reliabilityScores.begin(), reliabilityScores.end(), sortDesc);
        sort(accuracyScores.begin(), accuracyScores.end(), sortDesc);
        sort(errorScores.begin(), errorScores.end(), sortDesc);

        file << "Metric,OrderIndex,Algorithm,DisplayedValue,Unit\n";

        auto writeMetric = [&](const string& metric,
                            const vector<pair<string, double>>& scores,
                            bool inverseTimeMetric = false,
                            bool negateValue = false,
                            const string& unit = "") {
            for (size_t i = 0; i < scores.size(); ++i) {
                double shownValue = scores[i].second;

                if (inverseTimeMetric) {
                    shownValue = 1.0 / scores[i].second;
                }
                if (negateValue) {
                    shownValue = -scores[i].second;
                }

                file << metric << ","
                    << (i + 1) << ","
                    << scores[i].first << ","
                    << shownValue << ","
                    << unit << "\n";
            }
        };

        writeMetric("Speed", speedScores, true, false, "s");
        writeMetric("Reliability", reliabilityScores, false, false, "%");
        writeMetric("Accuracy", accuracyScores, false, false, "%");
        writeMetric("Error", errorScores, false, true, "%");
    }
};