FROM gcc:14

WORKDIR /app
COPY src ./src

RUN mkdir -p /runtime/bin /runtime/test_cases /runtime/results && \
    g++ -std=c++17 -O2 \
    src/main.cpp \
    src/AntColony.cpp \
    src/FileReader.cpp \
    src/TestRunner.cpp \
    src/GraphGenerator.cpp \
    src/GenerateTestSuite.cpp \
    -o /runtime/bin/aco    

WORKDIR /runtime
CMD ["./bin/aco"]

