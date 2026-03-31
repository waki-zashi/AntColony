#pragma once

#include "GraphGenerator.h"
#include <string>
#include <vector>

void tryGenerateConnected(int countVertex,
                          double density,
                          const std::string& graphType,
                          GraphGenerator& generator,
                          const std::string& testDir,
                          int& testCount,
                          std::vector<std::string>& generatedTestFiles);

void saveFilesList(const std::string& testDir,
                   const std::vector<std::string>& generatedTestFiles);

void generateConnectedTestSuite(const std::string& testDir = "data/test_cases");