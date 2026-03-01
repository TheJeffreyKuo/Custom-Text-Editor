#pragma once
#include <string>
#include <termios.h>

enum class EditorKey : int {
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    HOME,
    END,
    PAGE_UP,
    PAGE_DOWN,
    DEL,
    ESCAPE
};

struct TerminalSize {
    int rows;
    int cols;
};

int parseEscapeSequence(const char* seq, int len);

class Terminal {
public:
    Terminal();
    ~Terminal();

    int readKey();
    TerminalSize getSize() const;
    void flush(const std::string& buf);
    bool wasResized();

    Terminal(const Terminal&) = delete;
    Terminal& operator=(const Terminal&) = delete;

private:
    termios original_termios_;

    void enableRawMode();
    void disableRawMode();
    void installSignalHandlers();
};
