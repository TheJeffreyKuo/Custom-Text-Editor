#pragma once
#include "terminal.hpp"
#include <string>

constexpr int ctrlKey(char c) { return c & 0x1f; }

class Editor {
public:
    Editor();
    void run();

private:
    Terminal terminal_;
    TerminalSize screen_;

    int cx_ = 0;
    int cy_ = 0;
    bool running_ = true;

    void refreshScreen();
    void drawRows(std::string& buf);
    void processKeypress(int key);
    void moveCursor(int key);
};
