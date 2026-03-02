#pragma once
#include "diff.hpp"
#include "terminal.hpp"
#include <string>

class DiffViewer {
public:
    DiffViewer(Terminal& terminal, const DiffResult& diff,
               const std::string& leftName, const std::string& rightName);
    void run();

private:
    Terminal& terminal_;
    const DiffResult& diff_;
    std::string leftName_;
    std::string rightName_;
    TerminalSize screen_;

    int scrollOffset_ = 0;
    int currentHunk_ = 0;
    bool running_ = true;

    void refreshScreen();
    void drawRows(std::string& buf);
    void drawStatusBar(std::string& buf);
    void processKeypress(int key);
    void jumpToHunk(int index);
    int maxScroll() const;
};
