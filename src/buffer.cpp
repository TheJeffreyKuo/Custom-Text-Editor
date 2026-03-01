#include "buffer.hpp"
#include "filetype.hpp"
#include <cctype>
#include <cstring>
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

    // Highlight all rows after loading
    FileType ft = detectFileType(filename_);
    for (int i = 0; i < numRows(); i++) {
        bool prev = (i > 0) ? rows_[i - 1].openComment : false;
        highlightRow(rows_[i], ft, prev);
    }

    dirty_ = false;
}

int Buffer::numRows() const { return static_cast<int>(rows_.size()); }

const Row& Buffer::getRow(int index) const { return rows_[index]; }

Row& Buffer::getRowMut(int index) { return rows_[index]; }

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
    updateHighlight(row);
    dirty_ = true;
    return {EditAction::INSERT_CHAR, row, col, c, "", 0, 0};
}

EditAction Buffer::deleteChar(int row, int col) {
    char deleted = rows_[row].chars[col];
    rows_[row].chars.erase(rows_[row].chars.begin() + col);
    updateRender(rows_[row]);
    updateHighlight(row);
    dirty_ = true;
    return {EditAction::DELETE_CHAR, row, col, deleted, "", 0, 0};
}

EditAction Buffer::splitLine(int row, int col) {
    if (row >= numRows()) insertRow(numRows(), "");
    std::string tail = rows_[row].chars.substr(col);
    rows_[row].chars.resize(col);
    updateRender(rows_[row]);
    insertRow(row + 1, tail);
    updateHighlight(row);
    dirty_ = true;
    return {EditAction::SPLIT_LINE, row, col, '\0', "", 0, 0};
}

EditAction Buffer::joinLines(int row) {
    int joinCol = static_cast<int>(rows_[row].chars.size());
    std::string savedText = rows_[row + 1].chars;
    rows_[row].chars.append(savedText);
    updateRender(rows_[row]);
    deleteRow(row + 1);
    updateHighlight(row);
    dirty_ = true;
    return {EditAction::JOIN_LINES, row, joinCol, '\0', savedText, 0, 0};
}

void Buffer::updateHighlight(int fromRow) {
    FileType ft = detectFileType(filename_);
    for (int i = fromRow; i < numRows(); i++) {
        bool prev = (i > 0) ? rows_[i - 1].openComment : false;
        bool oldOpen = rows_[i].openComment;
        highlightRow(rows_[i], ft, prev);
        if (rows_[i].openComment == oldOpen && i > fromRow)
            break;
    }
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

bool isSeparator(char c) {
    return std::isspace(static_cast<unsigned char>(c)) || c == '\0' ||
           std::strchr(",.()+-/*=~%<>[];:{}&|^!?#", c) != nullptr;
}

void highlightRow(Row& row, const FileType& syntax, bool prevOpenComment) {
    row.hl.assign(row.render.size(), HL_NORMAL);
    row.openComment = prevOpenComment;

    const std::string& r = row.render;
    int len = static_cast<int>(r.size());
    int i = 0;
    bool prevSep = true;

    while (i < len) {
        char c = r[i];

        // Multi-line comment continuation
        if (row.openComment) {
            row.hl[i] = HL_MLCOMMENT;
            if (!syntax.mlCommentEnd.empty() &&
                r.compare(i, syntax.mlCommentEnd.size(), syntax.mlCommentEnd) == 0) {
                for (int j = 0; j < static_cast<int>(syntax.mlCommentEnd.size()); j++)
                    row.hl[i + j] = HL_MLCOMMENT;
                i += static_cast<int>(syntax.mlCommentEnd.size());
                row.openComment = false;
                prevSep = true;
            } else {
                i++;
            }
            continue;
        }

        // Single-line comment
        if (!syntax.singleLineComment.empty() &&
            r.compare(i, syntax.singleLineComment.size(), syntax.singleLineComment) == 0) {
            for (int j = i; j < len; j++)
                row.hl[j] = HL_COMMENT;
            break;
        }

        // Multi-line comment start
        if (!syntax.mlCommentStart.empty() &&
            r.compare(i, syntax.mlCommentStart.size(), syntax.mlCommentStart) == 0) {
            for (int j = 0; j < static_cast<int>(syntax.mlCommentStart.size()); j++)
                row.hl[i + j] = HL_MLCOMMENT;
            i += static_cast<int>(syntax.mlCommentStart.size());
            row.openComment = true;
            prevSep = false;
            continue;
        }

        // Strings
        if (syntax.highlightStrings && (c == '"' || c == '\'')) {
            char quote = c;
            row.hl[i] = HL_STRING;
            i++;
            while (i < len) {
                row.hl[i] = HL_STRING;
                if (r[i] == '\\' && i + 1 < len) {
                    row.hl[i + 1] = HL_STRING;
                    i += 2;
                    continue;
                }
                if (r[i] == quote) { i++; break; }
                i++;
            }
            prevSep = true;
            continue;
        }

        // Numbers
        if (syntax.highlightNumbers &&
            ((std::isdigit(static_cast<unsigned char>(c)) && prevSep) ||
             (c == '.' && i > 0 && row.hl[i - 1] == HL_NUMBER))) {
            row.hl[i] = HL_NUMBER;
            i++;
            while (i < len && (std::isdigit(static_cast<unsigned char>(r[i])) ||
                               r[i] == '.')) {
                row.hl[i] = HL_NUMBER;
                i++;
            }
            prevSep = false;
            continue;
        }

        // Keywords
        if (prevSep) {
            bool matched = false;

            // Check keywords1 first
            for (const auto& kw : syntax.keywords1) {
                int kwLen = static_cast<int>(kw.size());
                if (r.compare(i, kwLen, kw) == 0 &&
                    (i + kwLen >= len || isSeparator(r[i + kwLen]))) {
                    for (int j = 0; j < kwLen; j++)
                        row.hl[i + j] = HL_KEYWORD1;
                    i += kwLen;
                    prevSep = false;
                    matched = true;
                    break;
                }
            }
            if (matched) continue;

            // Check keywords2
            for (const auto& kw : syntax.keywords2) {
                int kwLen = static_cast<int>(kw.size());
                if (r.compare(i, kwLen, kw) == 0 &&
                    (i + kwLen >= len || isSeparator(r[i + kwLen]))) {
                    for (int j = 0; j < kwLen; j++)
                        row.hl[i + j] = HL_KEYWORD2;
                    i += kwLen;
                    prevSep = false;
                    matched = true;
                    break;
                }
            }
            if (matched) continue;
        }

        prevSep = isSeparator(c);
        i++;
    }
}

int highlightToColor(uint8_t hl) {
    switch (hl) {
        case HL_NUMBER:    return 31;
        case HL_STRING:    return 35;
        case HL_COMMENT:   return 36;
        case HL_MLCOMMENT: return 36;
        case HL_KEYWORD1:  return 33;
        case HL_KEYWORD2:  return 32;
        case HL_MATCH:     return 34;
        default:           return 37;
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

DocumentStats computeStats(const Buffer& buffer) {
    DocumentStats stats;
    stats.totalLines = buffer.numRows();
    for (int i = 0; i < buffer.numRows(); i++) {
        const std::string& chars = buffer.getRow(i).chars;
        stats.totalChars += static_cast<int>(chars.size());

        bool inWord = false;
        for (char c : chars) {
            if (std::isspace(static_cast<unsigned char>(c))) {
                inWord = false;
            } else if (!inWord) {
                inWord = true;
                stats.totalWords++;
            }
        }
    }
    return stats;
}
