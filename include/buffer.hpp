#pragma once
#include <string>
#include <vector>

constexpr int TAB_STOP = 8;

struct Row {
    std::string chars;
    std::string render;
};

int charsToRender(const Row& row, int cx);
int renderToChars(const Row& row, int rx);

class Buffer {
public:
    void open(const std::string& filename);

    int numRows() const;
    const Row& getRow(int index) const;
    const std::string& getFilename() const;

private:
    std::vector<Row> rows_;
    std::string filename_;

    void updateRender(Row& row);
};
