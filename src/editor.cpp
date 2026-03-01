#include "editor.hpp"
#include <cstdio>

static const char* VERSION = "0.0.1";

Editor::Editor() : screen_(terminal_.getSize()) {}

void Editor::run() {
    while (running_) {
        refreshScreen();
        int key = terminal_.readKey();
        processKeypress(key);
    }
}

void Editor::refreshScreen() {
    if (terminal_.wasResized())
        screen_ = terminal_.getSize();

    std::string buf;
    buf.append("\x1b[?25l");
    buf.append("\x1b[H");

    drawRows(buf);

    char cursor[32];
    snprintf(cursor, sizeof(cursor), "\x1b[%d;%dH", cy_ + 1, cx_ + 1);
    buf.append(cursor);

    buf.append("\x1b[?25h");
    terminal_.flush(buf);
}

void Editor::drawRows(std::string& buf) {
    for (int y = 0; y < screen_.rows; y++) {
        if (y == screen_.rows / 3) {
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
    switch (key) {
        case static_cast<int>(EditorKey::ARROW_LEFT):
            if (cx_ > 0) cx_--;
            break;
        case static_cast<int>(EditorKey::ARROW_RIGHT):
            if (cx_ < screen_.cols - 1) cx_++;
            break;
        case static_cast<int>(EditorKey::ARROW_UP):
            if (cy_ > 0) cy_--;
            break;
        case static_cast<int>(EditorKey::ARROW_DOWN):
            if (cy_ < screen_.rows - 1) cy_++;
            break;
        case static_cast<int>(EditorKey::HOME):
            cx_ = 0;
            break;
        case static_cast<int>(EditorKey::END):
            cx_ = screen_.cols - 1;
            break;
        case static_cast<int>(EditorKey::PAGE_UP):
            cy_ = 0;
            break;
        case static_cast<int>(EditorKey::PAGE_DOWN):
            cy_ = screen_.rows - 1;
            break;
    }
}
