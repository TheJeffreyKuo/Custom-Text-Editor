#include "buffer.hpp"
#include <fstream>

void Buffer::open(const std::string& filename) {
    filename_ = filename;
    std::ifstream file(filename);
    if (!file) return;

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        Row row;
        row.chars = std::move(line);
        updateRender(row);
        rows_.push_back(std::move(row));
    }
    dirty_ = false;
}

int Buffer::numRows() const { return static_cast<int>(rows_.size()); }

const Row& Buffer::getRow(int index) const { return rows_[index]; }

const std::string& Buffer::getFilename() const { return filename_; }

bool Buffer::isDirty() const { return dirty_; }

void Buffer::insertRow(int at, const std::string& text) {
    if (at < 0 || at > numRows()) return;
    Row row;
    row.chars = text;
    updateRender(row);
    rows_.insert(rows_.begin() + at, std::move(row));
}

void Buffer::deleteRow(int at) {
    if (at < 0 || at >= numRows()) return;
    rows_.erase(rows_.begin() + at);
}

EditAction Buffer::insertChar(int row, int col, char c) {
    if (row >= numRows()) insertRow(numRows(), "");
    rows_[row].chars.insert(rows_[row].chars.begin() + col, c);
    updateRender(rows_[row]);
    dirty_ = true;
    return {EditAction::INSERT_CHAR, row, col, c, "", 0, 0};
}

EditAction Buffer::deleteChar(int row, int col) {
    char deleted = rows_[row].chars[col];
    rows_[row].chars.erase(rows_[row].chars.begin() + col);
    updateRender(rows_[row]);
    dirty_ = true;
    return {EditAction::DELETE_CHAR, row, col, deleted, "", 0, 0};
}

EditAction Buffer::splitLine(int row, int col) {
    if (row >= numRows()) insertRow(numRows(), "");
    std::string tail = rows_[row].chars.substr(col);
    rows_[row].chars.resize(col);
    updateRender(rows_[row]);
    insertRow(row + 1, tail);
    dirty_ = true;
    return {EditAction::SPLIT_LINE, row, col, '\0', "", 0, 0};
}

EditAction Buffer::joinLines(int row) {
    int joinCol = static_cast<int>(rows_[row].chars.size());
    std::string savedText = rows_[row + 1].chars;
    rows_[row].chars.append(savedText);
    updateRender(rows_[row]);
    deleteRow(row + 1);
    dirty_ = true;
    return {EditAction::JOIN_LINES, row, joinCol, '\0', savedText, 0, 0};
}

int Buffer::save() {
    if (filename_.empty()) return -1;

    std::string content;
    for (int i = 0; i < numRows(); i++) {
        content.append(rows_[i].chars);
        content.push_back('\n');
    }

    std::ofstream file(filename_, std::ios::trunc);
    if (!file) return -1;

    file.write(content.data(), content.size());
    if (!file) return -1;

    dirty_ = false;
    return static_cast<int>(content.size());
}

int Buffer::save(const std::string& filename) {
    filename_ = filename;
    return save();
}

void Buffer::updateRender(Row& row) {
    row.render.clear();
    for (char c : row.chars) {
        if (c == '\t') {
            int spaces = TAB_STOP - (static_cast<int>(row.render.size()) % TAB_STOP);
            row.render.append(spaces, ' ');
        } else {
            row.render.push_back(c);
        }
    }
}

int charsToRender(const Row& row, int cx) {
    int rx = 0;
    for (int i = 0; i < cx && i < static_cast<int>(row.chars.size()); i++) {
        if (row.chars[i] == '\t')
            rx += TAB_STOP - (rx % TAB_STOP);
        else
            rx++;
    }
    return rx;
}

int renderToChars(const Row& row, int rx) {
    int cur_rx = 0;
    int cx = 0;
    for (cx = 0; cx < static_cast<int>(row.chars.size()); cx++) {
        if (cur_rx >= rx) return cx;
        if (row.chars[cx] == '\t')
            cur_rx += TAB_STOP - (cur_rx % TAB_STOP);
        else
            cur_rx++;
    }
    return cx;
}
