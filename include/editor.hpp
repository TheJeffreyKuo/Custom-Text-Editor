#pragma once
#include "buffer.hpp"
#include "filetype.hpp"
#include "terminal.hpp"
#include <ctime>
#include <string>
#include <vector>

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
    int quitConfirm_ = 0;

    std::string statusMsg_;
    time_t statusMsgTime_ = 0;

    DocumentStats stats_;
    bool statsDirty_ = true;

    std::vector<EditAction> undoStack_;

    void refreshScreen();
    void drawRows(std::string& buf);
    void drawStatusBar(std::string& buf);
    void drawMessageBar(std::string& buf);
    void processKeypress(int key);
    void moveCursor(int key);
    void scroll();
    void recordAction(EditAction action);
    void setStatusMessage(const std::string& msg);
    std::string prompt(const std::string& message);
};
