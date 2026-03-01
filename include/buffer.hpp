#pragma once
#include <cstdint>
#include <ctime>
#include <string>
#include <vector>

constexpr int TAB_STOP = 8;

enum Highlight : uint8_t {
    HL_NORMAL = 0,
    HL_NUMBER,
    HL_STRING,
    HL_COMMENT,
    HL_MLCOMMENT,
    HL_KEYWORD1,
    HL_KEYWORD2,
    HL_MATCH
};

struct FileType;

struct Row {
    std::string chars;
    std::string render;
    std::vector<uint8_t> hl;
    bool openComment = false;
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
void highlightRow(Row& row, const FileType& syntax, bool prevOpenComment);
int highlightToColor(uint8_t hl);
bool isSeparator(char c);

class Buffer {
public:
    void open(const std::string& filename);

    int numRows() const;
    const Row& getRow(int index) const;
    Row& getRowMut(int index);
    const std::string& getFilename() const;
    bool isDirty() const;

    EditAction insertChar(int row, int col, char c);
    EditAction deleteChar(int row, int col);
    EditAction splitLine(int row, int col);
    EditAction joinLines(int row);

    void updateHighlight(int fromRow);

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
