#pragma once
#include <ctime>
#include <string>
#include <vector>

constexpr int TAB_STOP = 8;

struct Row {
    std::string chars;
    std::string render;
};

struct EditAction {
    enum Type { INSERT_CHAR, DELETE_CHAR, SPLIT_LINE, JOIN_LINES };
    Type type;
    int row, col;
    char ch;
    std::string text;
    int cursorRow, cursorCol;
    time_t timestamp = 0;
};

struct DocumentStats {
    int totalLines = 0;
    int totalChars = 0;
    int totalWords = 0;
};

int charsToRender(const Row& row, int cx);
int renderToChars(const Row& row, int rx);

class Buffer {
public:
    void open(const std::string& filename);

    int numRows() const;
    const Row& getRow(int index) const;
    const std::string& getFilename() const;
    bool isDirty() const;

    EditAction insertChar(int row, int col, char c);
    EditAction deleteChar(int row, int col);
    EditAction splitLine(int row, int col);
    EditAction joinLines(int row);

    int save();
    int save(const std::string& filename);

private:
    std::vector<Row> rows_;
    std::string filename_;
    bool dirty_ = false;

    void insertRow(int at, const std::string& text);
    void deleteRow(int at);
    void updateRender(Row& row);
};

DocumentStats computeStats(const Buffer& buffer);
