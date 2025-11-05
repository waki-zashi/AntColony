#pragma once
#include "GraphGenerator.h"
#include <fstream>
#include <string>

using namespace std;

void tryGenerateConnected(int countVertex, double density, const string graphType, GraphGenerator& generator);
void saveFilesList();
void generateConnectedTestSuite();
