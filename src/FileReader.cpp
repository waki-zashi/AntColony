#include "FileReader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

void readGraphFromFile(const string& filename, bool& fileLoaded, vector<vector<double>>& graph, vector<string>& labels, int& start, int& end) {
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
    bool firstLineChecked = false;
    bool hasLabels = false;
    char delimiter = ',';
    vector<string> allLines;

    // Сначала читаем все строки
    while (getline(file, line)) {
        allLines.push_back(line);
    }

    // Обрабатываем все строки кроме последней как матрицу смежности
    for (int i = 0; i < allLines.size() - 1; i++) {
        line = allLines[i];

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

        // Если это не первая строка с метками, обрабатываем как числа
        if (!(i == 0 && hasLabels)) {
            vector<double> row;
            for (const auto& t : tokens) {
                row.push_back(stod(t));
            }
            graph.push_back(row);
        }
    }

    // Обрабатываем последнюю строку как start и end
    if (!allLines.empty()) {
        string lastLine = allLines.back();
        stringstream ss(lastLine);
        vector<string> tokens;
        string token;

        // Определяем разделитель для последней строки
        if (lastLine.find(',') != string::npos) delimiter = ',';
        else if (lastLine.find(' ') != string::npos) delimiter = ' ';

        while (getline(ss, token, delimiter)) {
            if (token.empty()) continue;
            tokens.push_back(token);
        }

        if (tokens.size() >= 2) {
            // Ищем индексы вершин start и end в labels
            /*string startLabel = tokens[0];
            string endLabel = tokens[1];*/

            start = stoi(tokens[0]);
            end = stoi(tokens[1]);

            /*for (size_t i = 0; i < labels.size(); i++) {
                if (labels[i] == startLabel) 
                    start = i;
                if (labels[i] == endLabel) 
                    end = i;
            }*/

            // Если не нашли, устанавливаем по умолчанию
            if (start == -1) start = 0;
            if (end == -1) end = labels.size() - 1;
        }
    }

    // Если меток нет, создаем их
    int n = graph.size();
    if (!hasLabels && n > 0) {
        labels.resize(n);
        for (int i = 0; i < n; i++) {
            labels[i] = string(1, 'A' + i);
        }
    }
}