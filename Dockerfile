FROM gcc:14

WORKDIR /app
COPY src ./src

RUN mkdir -p /runtime/bin /runtime/data/test_cases /runtime/results && \
    g++ -std=c++17 -O2 -Wall -Wextra \
        src/*.cpp \
        -o /runtime/bin/aco

WORKDIR /runtime
CMD ["./bin/aco", "--interactive"]