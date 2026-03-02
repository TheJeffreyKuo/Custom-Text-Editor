#include "diff.hpp"
#include <algorithm>
#include <fstream>

std::vector<std::string> readFileLines(const std::string& filename) {
    std::vector<std::string> lines;
    std::ifstream file(filename);
    if (!file) return lines;

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        lines.push_back(std::move(line));
    }
    return lines;
}

DiffResult computeDiff(const std::vector<std::string>& left,
                       const std::vector<std::string>& right) {
    int m = static_cast<int>(left.size());
    int n = static_cast<int>(right.size());

    // Build LCS table (O(m*n) space — fine for files under ~5K lines)
    std::vector<std::vector<int>> lcs(m + 1, std::vector<int>(n + 1, 0));
    for (int i = m - 1; i >= 0; i--) {
        for (int j = n - 1; j >= 0; j--) {
            if (left[i] == right[j])
                lcs[i][j] = lcs[i + 1][j + 1] + 1;
            else
                lcs[i][j] = std::max(lcs[i + 1][j], lcs[i][j + 1]);
        }
    }

    // Backtrack to produce diff
    DiffResult result;
    int i = 0, j = 0;
    int leftNum = 1, rightNum = 1;

    while (i < m && j < n) {
        if (left[i] == right[j]) {
            result.lines.push_back({DiffLine::SAME, left[i], leftNum++, rightNum++});
            i++;
            j++;
        } else if (lcs[i + 1][j] >= lcs[i][j + 1]) {
            result.lines.push_back({DiffLine::REMOVED, left[i], leftNum++, -1});
            result.removedCount++;
            i++;
        } else {
            result.lines.push_back({DiffLine::ADDED, right[j], -1, rightNum++});
            result.addedCount++;
            j++;
        }
    }

    while (i < m) {
        result.lines.push_back({DiffLine::REMOVED, left[i], leftNum++, -1});
        result.removedCount++;
        i++;
    }

    while (j < n) {
        result.lines.push_back({DiffLine::ADDED, right[j], -1, rightNum++});
        result.addedCount++;
        j++;
    }

    // Compute hunk metadata
    bool inHunk = false;
    for (int k = 0; k < static_cast<int>(result.lines.size()); k++) {
        if (result.lines[k].type != DiffLine::SAME) {
            if (!inHunk) {
                result.hunkStarts.push_back(k);
                result.hunkCount++;
                inHunk = true;
            }
        } else {
            inHunk = false;
        }
    }

    return result;
}
