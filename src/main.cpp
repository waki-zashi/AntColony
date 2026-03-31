#include "AntColony.h"
#include "FileReader.h"
#include "PipelineRunner.h"
#include "GenerateTestSuite.h"

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

namespace {
    const string DEFAULT_TEST_DIR = "data/test_cases";
    const string DEFAULT_RESULTS_DIR = "results";

    void printHelp() {
        cout << "Usage:\n";
        cout << "  ./aco --help\n";
        // cout << "  ./aco --interactive\n";
        cout << "  ./aco --single <graph_file>\n";
        cout << "  ./aco --generate-tests [--test-dir <dir>]\n";
        cout << "  ./aco --run <aco|dj|fw|astar> [--test-dir <dir>]\n";
        // cout << "  ./aco --run-all [--test-dir <dir>]\n";
        cout << "  ./aco --analyze\n";
        cout << "  ./aco --full-pipeline [--test-dir <dir>]\n";
    }

    string getOptionValue(const vector<string>& args, const string& key, const string& defaultValue) {
        for (size_t i = 0; i + 1 < args.size(); ++i) {
            if (args[i] == key) {
                return args[i + 1];
            }
        }
        return defaultValue;
    }

    bool hasFlag(const vector<string>& args, const string& flag) {
        for (const auto& arg : args) {
            if (arg == flag) {
                return true;
            }
        }
        return false;
    }

    int runSingleGraphMode(const string& filename) {
        bool fileLoaded = false;
        vector<vector<double>> graph;
        vector<string> labels;
        int start = -1;
        int end = -1;

        readGraphFromFile(filename, fileLoaded, graph, labels, start, end);

        if (!fileLoaded) {
            cerr << "Error: failed to load graph file: " << filename << endl;
            return 1;
        }

        if (graph.empty() || labels.empty()) {
            cerr << "Error: graph is empty or malformed." << endl;
            return 1;
        }

        if (start < 0 || end < 0 || start >= static_cast<int>(labels.size()) || end >= static_cast<int>(labels.size())) {
            cerr << "Error: invalid start/end vertices in graph file." << endl;
            return 1;
        }

        cout << "Graph loaded with " << graph.size() << " vertices.\n";
        cout << "Vertices: ";
        for (const auto& l : labels) {
            cout << l << " ";
        }
        cout << "\n";

        AntColony colony(graph, labels, start, end);
        colony.run();

        return 0;
    }

    int runInteractiveMode() {
        string testDirectory = DEFAULT_TEST_DIR;
        PipelineRunner pipeline(testDirectory);

        bool mainRunFlag = true;
        while (mainRunFlag) {
            string baseCommand;
            cout << "Enter program mode: autotests (a), single graph (i), recreate test cases (r), full pipeline (p): ";
            cin >> baseCommand;

            if (baseCommand == "a") {
                mainRunFlag = false;
                bool choiceTestFlag = true;

                while (choiceTestFlag) {
                    string choiceTestCommand;
                    cout << "What you want to test: ACO (aco), Dijkstra (dj), Floyd Warshall (fw), A-Star (a*), run all (runall) or analyze (all): ";
                    cin >> choiceTestCommand;

                    if (choiceTestCommand == "aco") {
                        choiceTestFlag = false;
                        pipeline.runACO();
                    } else if (choiceTestCommand == "dj") {
                        choiceTestFlag = false;
                        pipeline.runDijkstra();
                    } else if (choiceTestCommand == "fw") {
                        choiceTestFlag = false;
                        pipeline.runFloydWarshall();
                    } else if (choiceTestCommand == "a*") {
                        choiceTestFlag = false;
                        pipeline.runAStar();
                    } else if (choiceTestCommand == "runall") {
                        choiceTestFlag = false;
                        pipeline.runAllAlgorithms();
                    } else if (choiceTestCommand == "all") {
                        choiceTestFlag = false;
                        pipeline.analyzeResults();
                    } else {
                        cout << "Wrong choice, please repeat correctly: "
                             << "'aco', 'dj', 'fw', 'a*', 'runall' or 'all'\n";
                    }
                }
            } else if (baseCommand == "i") {
                mainRunFlag = false;

                string filename;
                cout << "Enter graph filename (txt or csv): ";
                cin >> filename;

                return runSingleGraphMode(filename);
            } else if (baseCommand == "r") {
                mainRunFlag = false;
                pipeline.generateTests();
            } else if (baseCommand == "p") {
                mainRunFlag = false;
                pipeline.fullPipeline(true);
            } else {
                cout << "Wrong command. Enter 'a', 'i', 'r' or 'p'." << endl;
            }
        }

        return 0;
    }
}

int main(int argc, char* argv[]) {
    fs::create_directories(DEFAULT_RESULTS_DIR);

    vector<string> args;
    for (int i = 1; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    if (args.empty() || hasFlag(args, "--interactive")) {
        return runInteractiveMode();
    }

    if (hasFlag(args, "--help")) {
        printHelp();
        return 0;
    }

    const string testDirectory = getOptionValue(args, "--test-dir", DEFAULT_TEST_DIR);
    PipelineRunner pipeline(testDirectory);

    if (hasFlag(args, "--generate-tests")) {
        pipeline.generateTests();
        return 0;
    }

    if (hasFlag(args, "--run-all")) {
        if (!pipeline.ensureTestSuiteExists()) {
            cerr << "Error: test suite not found in " << testDirectory << endl;
            cerr << "Run --generate-tests first, or use --full-pipeline." << endl;
            return 1;
        }

        pipeline.runAllAlgorithms();
        return 0;
    }

    if (hasFlag(args, "--analyze")) {
        pipeline.analyzeResults();
        return 0;
    }

    if (hasFlag(args, "--full-pipeline")) {
        pipeline.fullPipeline(true);
        return 0;
    }

    for (size_t i = 0; i + 1 < args.size(); ++i) {
        if (args[i] == "--single") {
            return runSingleGraphMode(args[i + 1]);
        }

        if (args[i] == "--run") {
            if (!pipeline.ensureTestSuiteExists()) {
                cerr << "Error: test suite not found in " << testDirectory << endl;
                cerr << "Run --generate-tests first, or use --full-pipeline." << endl;
                return 1;
            }

            const string algo = args[i + 1];

            if (algo == "aco") {
                pipeline.runACO();
                return 0;
            }
            if (algo == "dj") {
                pipeline.runDijkstra();
                return 0;
            }
            if (algo == "fw") {
                pipeline.runFloydWarshall();
                return 0;
            }
            if (algo == "astar") {
                pipeline.runAStar();
                return 0;
            }

            cerr << "Unknown algorithm: " << algo << endl;
            cerr << "Allowed: aco, dj, fw, astar" << endl;
            return 1;
        }
    }

    cerr << "Unknown command.\n";
    printHelp();
    return 1;
}