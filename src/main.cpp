#include "AntColony.h"
#include "FileReader.h"
#include "TestRunner.h"
#include "Dijkstra.h"
#include "FloydWarshall.h"
#include "AStar.h"
#include "ResultAnalyzer.h"
#include "GenerateTestSuite.h"
#include <iostream>

using namespace std;

string testDirectory = "test_cases";

int main() {
    bool mainRunFlag = true;
    while (mainRunFlag) {
        string baseCommand;
        cout << "Enter program mode: autotests (a), initial (i) or recreate test cases (r): ";
        cin >> baseCommand;

        if (baseCommand == "a") {
            mainRunFlag = false;
            bool choiceTestFlag = true;

            while(choiceTestFlag) {
                string choiceTestCommand;
                cout << "What you want to test: ACO (aco), Dijkstra (dj), Floyd Warshall (fw), A-Star (a*) or all (all): ";
                cin >> choiceTestCommand;

                if (choiceTestCommand == "aco") {
                    choiceTestFlag = false;

                    cout << "=== ACO Algorithm Test Suite ===" << endl;

                    TestRunner runner;
                    runner.runTestSuite(testDirectory);

                    cout << "=== Testing complete ===" << endl;
                }
                else if (choiceTestCommand == "dj") {
                    choiceTestFlag = false;

                    cout << "=== Dijkstra Algorithm Test Suite ===" << endl;

                    DijkstraTestRunner runner;
                    runner.runTestSuite(testDirectory);

                    cout << "=== Dijkstra Testing complete ===" << endl;
                }
                else if (choiceTestCommand == "fw") {
                    choiceTestFlag = false;

                    cout << "=== Floyd-Warshall Algorithm Test Suite ===" << endl;

                    FloydWarshallTestRunner runner;
                    runner.runTestSuite(testDirectory);

                    cout << "=== Floyd-Warshall Testing complete ===" << endl;
                }
                else if (choiceTestCommand == "a*") {
                    choiceTestFlag = false;

                    cout << "=== A* Algorithm Test Suite ===" << endl;

                    AStarTestRunner runner;
                    runner.runTestSuite(testDirectory);

                    cout << "=== A* Testing complete ===" << endl;
                }
                else if (choiceTestCommand == "all") {
                    choiceTestFlag = false;
                    
                    cout << "=== ALGORITHM COMPARISON TOOL ===" << endl;

                    ResultsAnalyzer analyzer;

                    analyzer.loadResults(
                        "aco_results.csv",
                        "dijkstra_results.csv",
                        "astar_results.csv",
                        "fw_results.csv"
                    );

                    analyzer.generateComparativeAnalysis();

                    cout << "=== Analysis complete! ===" << endl;
                    cout << "Results saved to:" << endl;
                    cout << "- detailed_comparison.csv" << endl;
                    cout << "- summary_comparison.csv" << endl;
                }
                else {
                    cout << "Wrong chiose, please repeat correctly: 'aco', 'dj', 'fw', 'a*' or 'all'";
                }
            }
        }
        else if (baseCommand == "i") {
            mainRunFlag = false;

            string filename;
            cout << "Enter graph filename (txt or csv): ";
            cin >> filename;

            bool fileLoaded = false;
            vector<vector<double>> graph;
            vector<string> labels;
            int start = -1;
            int end = -1;

            readGraphFromFile(filename, fileLoaded, graph, labels, start, end);

            if (!fileLoaded) {
                graph = {
                    {0, 2, 0, 1, 0},
                    {2, 0, 3, 2, 0},
                    {0, 3, 0, 0, 2},
                    {1, 2, 0, 0, 3},
                    {0, 0, 2, 3, 0}
                };
                labels = { "A", "B", "C", "D", "E" };

                string startLabel, endLabel;
                cout << "Enter start vertex: ";
                cin >> startLabel;
                cout << "Enter end vertex: ";
                cin >> endLabel;

                for (size_t i = 0; i < labels.size(); i++) {
                    if (labels[i] == startLabel) start = i;
                    if (labels[i] == endLabel) end = i;
                }

                if (start == -1 || end == -1) {
                    cerr << "Invalid start or end vertex!\n";
                    return 1;
                }
            }

            cout << "Graph loaded with " << graph.size() << " vertices.\n";
            cout << "Vertices: ";
            for (auto& l : labels) 
                cout << l << " ";
            cout << "\n";

            AntColony colony(graph, labels, start, end);
            colony.run();
        }
        else if (baseCommand == "r") {
            mainRunFlag = false;

            generateConnectedTestSuite();
        }
        else {
            cout << "Wrong command, enter 'a' or 'i' to choose mode" << endl;
        }
    }
}