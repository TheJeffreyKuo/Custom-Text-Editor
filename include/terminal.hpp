#pragma once
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

// Pure byte-to-key mapping, no terminal dependency.
int parseEscapeSequence(const char* seq, int len);

class Terminal {
public:
    Terminal();
    ~Terminal();

    int readKey();

    Terminal(const Terminal&) = delete;
    Terminal& operator=(const Terminal&) = delete;

private:
    termios original_termios_;

    void enableRawMode();
    void disableRawMode();
};
