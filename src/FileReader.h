#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

void readGraphFromFile(const string& filename, bool& fileLoaded, vector<vector<double>>& graph, vector<string>& labels);