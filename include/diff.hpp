#pragma once
#include <string>
#include <vector>

struct DiffLine {
    enum Type { SAME, ADDED, REMOVED };
    Type type;
    std::string text;
    int leftLineNum;
    int rightLineNum;
};

struct DiffResult {
    std::vector<DiffLine> lines;
    int addedCount = 0;
    int removedCount = 0;
    int hunkCount = 0;
    std::vector<int> hunkStarts;
};

std::vector<std::string> readFileLines(const std::string& filename);
DiffResult computeDiff(const std::vector<std::string>& left,
                       const std::vector<std::string>& right);
