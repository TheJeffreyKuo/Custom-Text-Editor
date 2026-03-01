#pragma once
#include "buffer.hpp"
#include "terminal.hpp"
#include <string>

constexpr int ctrlKey(char c) { return c & 0x1f; }

class Editor {
public:
    Editor(const std::string& filename = "");
    void run();

private:
    Terminal terminal_;
    Buffer buffer_;
    TerminalSize screen_;

    int cx_ = 0;
    int cy_ = 0;
    int rx_ = 0;
    int rowOffset_ = 0;
    int colOffset_ = 0;
    bool running_ = true;

    void refreshScreen();
    void drawRows(std::string& buf);
    void processKeypress(int key);
    void moveCursor(int key);
    void scroll();
};
