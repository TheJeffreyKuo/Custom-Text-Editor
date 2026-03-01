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
}

int Buffer::numRows() const { return static_cast<int>(rows_.size()); }

const Row& Buffer::getRow(int index) const { return rows_[index]; }

const std::string& Buffer::getFilename() const { return filename_; }

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
