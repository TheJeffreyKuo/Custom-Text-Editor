#include "editor.hpp"
#include <cstdio>

static const char* VERSION = "0.0.1";

Editor::Editor(const std::string& filename) : screen_(terminal_.getSize()) {
    if (!filename.empty())
        buffer_.open(filename);
}

void Editor::run() {
    while (running_) {
        refreshScreen();
        int key = terminal_.readKey();
        processKeypress(key);
    }
}

void Editor::scroll() {
    rx_ = 0;
    if (cy_ < buffer_.numRows())
        rx_ = charsToRender(buffer_.getRow(cy_), cx_);

    if (cy_ < rowOffset_)
        rowOffset_ = cy_;
    if (cy_ >= rowOffset_ + screen_.rows)
        rowOffset_ = cy_ - screen_.rows + 1;

    if (rx_ < colOffset_)
        colOffset_ = rx_;
    if (rx_ >= colOffset_ + screen_.cols)
        colOffset_ = rx_ - screen_.cols + 1;
}

void Editor::refreshScreen() {
    if (terminal_.wasResized())
        screen_ = terminal_.getSize();

    // Snap cx_ to row length
    int rowLen = (cy_ < buffer_.numRows())
        ? static_cast<int>(buffer_.getRow(cy_).chars.size()) : 0;
    if (cx_ > rowLen) cx_ = rowLen;

    scroll();

    std::string buf;
    buf.append("\x1b[?25l");
    buf.append("\x1b[H");

    drawRows(buf);

    char cursor[32];
    snprintf(cursor, sizeof(cursor), "\x1b[%d;%dH",
        cy_ - rowOffset_ + 1, rx_ - colOffset_ + 1);
    buf.append(cursor);

    buf.append("\x1b[?25h");
    terminal_.flush(buf);
}

void Editor::drawRows(std::string& buf) {
    for (int y = 0; y < screen_.rows; y++) {
        int fileRow = y + rowOffset_;

        if (fileRow >= buffer_.numRows()) {
            if (buffer_.numRows() == 0 && y == screen_.rows / 3) {
                char welcome[80];
                int len = snprintf(welcome, sizeof(welcome),
                    "Custom Text Editor -- version %s", VERSION);
                if (len > screen_.cols) len = screen_.cols;

                int padding = (screen_.cols - len) / 2;
                if (padding > 0) {
                    buf.append("~");
                    padding--;
                }
                while (padding-- > 0) buf.append(" ");
                buf.append(welcome, len);
            } else {
                buf.append("~");
            }
        } else {
            const std::string& render = buffer_.getRow(fileRow).render;
            int len = static_cast<int>(render.size()) - colOffset_;
            if (len < 0) len = 0;
            if (len > screen_.cols) len = screen_.cols;
            if (len > 0)
                buf.append(render, colOffset_, len);
        }

        buf.append("\x1b[K");
        if (y < screen_.rows - 1)
            buf.append("\r\n");
    }
}

void Editor::processKeypress(int key) {
    switch (key) {
        case ctrlKey('q'):
            running_ = false;
            break;

        case static_cast<int>(EditorKey::ARROW_UP):
        case static_cast<int>(EditorKey::ARROW_DOWN):
        case static_cast<int>(EditorKey::ARROW_LEFT):
        case static_cast<int>(EditorKey::ARROW_RIGHT):
        case static_cast<int>(EditorKey::HOME):
        case static_cast<int>(EditorKey::END):
        case static_cast<int>(EditorKey::PAGE_UP):
        case static_cast<int>(EditorKey::PAGE_DOWN):
            moveCursor(key);
            break;
    }
}

void Editor::moveCursor(int key) {
    int numRows = buffer_.numRows();
    int rowLen = (cy_ < numRows)
        ? static_cast<int>(buffer_.getRow(cy_).chars.size()) : 0;

    switch (key) {
        case static_cast<int>(EditorKey::ARROW_LEFT):
            if (cx_ > 0) {
                cx_--;
            } else if (cy_ > 0) {
                cy_--;
                cx_ = static_cast<int>(buffer_.getRow(cy_).chars.size());
            }
            break;
        case static_cast<int>(EditorKey::ARROW_RIGHT):
            if (cx_ < rowLen) {
                cx_++;
            } else if (cy_ < numRows - 1) {
                cy_++;
                cx_ = 0;
            }
            break;
        case static_cast<int>(EditorKey::ARROW_UP):
            if (cy_ > 0) cy_--;
            break;
        case static_cast<int>(EditorKey::ARROW_DOWN):
            if (cy_ < numRows) cy_++;
            break;
        case static_cast<int>(EditorKey::HOME):
            cx_ = 0;
            break;
        case static_cast<int>(EditorKey::END):
            if (cy_ < numRows)
                cx_ = static_cast<int>(buffer_.getRow(cy_).chars.size());
            break;
        case static_cast<int>(EditorKey::PAGE_UP):
            cy_ = rowOffset_;
            for (int i = 0; i < screen_.rows; i++)
                if (cy_ > 0) cy_--;
            break;
        case static_cast<int>(EditorKey::PAGE_DOWN):
            cy_ = rowOffset_ + screen_.rows - 1;
            for (int i = 0; i < screen_.rows; i++)
                if (cy_ < numRows) cy_++;
            break;
    }
}
