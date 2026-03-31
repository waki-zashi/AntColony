#include "FileReader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>
#include <algorithm>

using namespace std;

namespace {
    string trim(const string& s) {
        size_t start = 0;
        while (start < s.size() && isspace(static_cast<unsigned char>(s[start]))) {
            start++;
        }

        size_t end = s.size();
        while (end > start && isspace(static_cast<unsigned char>(s[end - 1]))) {
            end--;
        }

        return s.substr(start, end - start);
    }

    bool isNumericToken(const string& s) {
        if (s.empty()) {
            return false;
        }

        try {
            size_t pos = 0;
            stod(s, &pos);
            return pos == s.size();
        } catch (...) {
            return false;
        }
    }

    vector<string> splitLine(const string& line, char delimiter) {
        vector<string> tokens;
        string token;
        stringstream ss(line);

        while (getline(ss, token, delimiter)) {
            tokens.push_back(trim(token));
        }

        return tokens;
    }
}

void readGraphFromFile(const string& filename,
                       bool& fileLoaded,
                       vector<vector<double>>& graph,
                       vector<string>& labels,
                       int& start,
                       int& end) {
    ifstream file(filename);
    string line;

    graph.clear();
    labels.clear();
    start = -1;
    end = -1;

    if (!file.is_open()) {
        cerr << "Error: cannot open file " << filename << ". Using default example.\n";
        fileLoaded = false;
        return;
    }

    fileLoaded = true;

    vector<string> allLines;
    while (getline(file, line)) {
        line = trim(line);
        if (!line.empty()) {
            allLines.push_back(line);
        }
    }

    if (allLines.size() < 2) {
        cerr << "Error: file format is invalid or too short: " << filename << endl;
        fileLoaded = false;
        return;
    }

    char delimiter = ',';
    if (allLines[0].find(',') != string::npos) {
        delimiter = ',';
    } else if (allLines[0].find(' ') != string::npos) {
        delimiter = ' ';
    }

    // Last line is start,end
    const string lastLine = allLines.back();
    const vector<string> lastTokens = splitLine(lastLine, delimiter);

    if (lastTokens.size() < 2 || !isNumericToken(lastTokens[0]) || !isNumericToken(lastTokens[1])) {
        cerr << "Error: last line must contain numeric start and end indices: " << filename << endl;
        fileLoaded = false;
        return;
    }

    start = stoi(lastTokens[0]);
    end = stoi(lastTokens[1]);

    // All lines except the last are matrix-related
    vector<string> matrixLines(allLines.begin(), allLines.end() - 1);
    if (matrixLines.empty()) {
        cerr << "Error: adjacency matrix is missing: " << filename << endl;
        fileLoaded = false;
        return;
    }

    // Detect whether first line is a header with labels
    vector<string> firstTokens = splitLine(matrixLines[0], delimiter);
    bool hasHeaderLabels = false;

    if (!firstTokens.empty()) {
        bool allNumeric = true;
        for (const auto& token : firstTokens) {
            if (token.empty()) {
                continue;
            }
            if (!isNumericToken(token)) {
                allNumeric = false;
                break;
            }
        }
        hasHeaderLabels = !allNumeric;
    }

    size_t matrixStartLine = 0;
    bool rowLabelsPresent = false;

    if (hasHeaderLabels) {
        // Header may look like: ,A,B,C   or   A,B,C
        // If first token is empty, skip it; otherwise use all tokens as labels.
        if (!firstTokens.empty() && firstTokens[0].empty()) {
            labels.assign(firstTokens.begin() + 1, firstTokens.end());
        } else {
            labels = firstTokens;
        }

        matrixStartLine = 1;
        rowLabelsPresent = true;
    }

    for (size_t i = matrixStartLine; i < matrixLines.size(); ++i) {
        vector<string> tokens = splitLine(matrixLines[i], delimiter);

        if (tokens.empty()) {
            continue;
        }

        // If rows start with a vertex label, drop the first token.
        if (rowLabelsPresent && !tokens.empty() && !isNumericToken(tokens[0])) {
            tokens.erase(tokens.begin());
        }

        vector<double> row;
        row.reserve(tokens.size());

        for (const auto& token : tokens) {
            if (token.empty()) {
                continue;
            }
            if (!isNumericToken(token)) {
                cerr << "Error: non-numeric token in matrix row: '" << token
                     << "' in file " << filename << endl;
                fileLoaded = false;
                graph.clear();
                labels.clear();
                start = -1;
                end = -1;
                return;
            }
            row.push_back(stod(token));
        }

        if (!row.empty()) {
            graph.push_back(row);
        }
    }

    const int n = static_cast<int>(graph.size());
    if (n == 0) {
        cerr << "Error: empty graph matrix in file " << filename << endl;
        fileLoaded = false;
        return;
    }

    // Validate square matrix
    for (const auto& row : graph) {
        if (static_cast<int>(row.size()) != n) {
            cerr << "Error: adjacency matrix is not square in file " << filename << endl;
            fileLoaded = false;
            graph.clear();
            labels.clear();
            start = -1;
            end = -1;
            return;
        }
    }

    // Generate labels if missing or mismatched
    if (labels.size() != static_cast<size_t>(n)) {
        labels.clear();
        labels.reserve(n);
        for (int i = 0; i < n; ++i) {
            if (i < 26) {
                labels.push_back(string(1, static_cast<char>('A' + i)));
            } else {
                labels.push_back("V" + to_string(i + 1));
            }
        }
    }

    if (start < 0 || start >= n) {
        start = 0;
    }
    if (end < 0 || end >= n) {
        end = n - 1;
    }
}