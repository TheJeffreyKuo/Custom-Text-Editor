#pragma once
#include <termios.h>

class Terminal {
public:
    Terminal();
    ~Terminal();

    Terminal(const Terminal&) = delete;
    Terminal& operator=(const Terminal&) = delete;

private:
    termios original_termios_;

    void enableRawMode();
    void disableRawMode();
};
