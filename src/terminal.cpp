#include "terminal.hpp"
#include <stdexcept>
#include <unistd.h>

Terminal::Terminal() {
    enableRawMode();
}

Terminal::~Terminal() {
    disableRawMode();
}

void Terminal::enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &original_termios_) == -1)
        throw std::runtime_error("tcgetattr failed");

    termios raw = original_termios_;

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        throw std::runtime_error("tcsetattr failed");
}

void Terminal::disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios_);
}
