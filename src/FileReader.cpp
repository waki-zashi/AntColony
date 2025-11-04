#include "FileReader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

void readGraphFromFile(const string& filename, bool& fileLoaded, vector<vector<double>>& graph, vector<string>& labels) {
    ifstream file(filename);
    string line;
    graph.clear();
    labels.clear();

    if (!file.is_open()) {
        cerr << "Error: cannot open file " << filename << ". Using default example.\n";
        fileLoaded = false;
        return;
    }

    fileLoaded = true;
    bool firstLineChecked = false;
    bool hasLabels = false;
    char delimiter = ',';

    while (getline(file, line)) {
        if (!firstLineChecked) {
            if (line.find(',') != string::npos) delimiter = ',';
            else if (line.find(' ') != string::npos) delimiter = ' ';
        }

        stringstream ss(line);
        vector<string> tokens;
        string token;
        while (getline(ss, token, delimiter)) {
            if (token.empty()) continue;
            tokens.push_back(token);
        }

        if (!firstLineChecked) {
            firstLineChecked = true;
            bool allNumbers = true;
            for (const auto& t : tokens) {
                try {
                    stod(t);
                }
                catch (...) {
                    allNumbers = false;
                    break;
                }
            }
            if (!allNumbers) {
                hasLabels = true;
                labels = tokens;
                continue;
            }
        }

        vector<double> row;
        for (const auto& t : tokens) {
            row.push_back(stod(t));
        }
        graph.push_back(row);
    }

    int n = graph.size();
    if (!hasLabels) {
        labels.resize(n);
        for (int i = 0; i < n; i++) {
            labels[i] = string(1, 'A' + i);
        }
    }
}