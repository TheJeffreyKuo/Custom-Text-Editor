#include "terminal.hpp"
#include <cerrno>
#include <cstdio>
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

int parseEscapeSequence(const char* seq, int len) {
    if (len < 2) return static_cast<int>(EditorKey::ESCAPE);

    if (seq[0] == '[') {
        if (len == 3 && seq[2] == '~') {
            switch (seq[1]) {
                case '1': return static_cast<int>(EditorKey::HOME);
                case '3': return static_cast<int>(EditorKey::DEL);
                case '4': return static_cast<int>(EditorKey::END);
                case '5': return static_cast<int>(EditorKey::PAGE_UP);
                case '6': return static_cast<int>(EditorKey::PAGE_DOWN);
                case '7': return static_cast<int>(EditorKey::HOME);
                case '8': return static_cast<int>(EditorKey::END);
            }
        } else {
            switch (seq[1]) {
                case 'A': return static_cast<int>(EditorKey::ARROW_UP);
                case 'B': return static_cast<int>(EditorKey::ARROW_DOWN);
                case 'C': return static_cast<int>(EditorKey::ARROW_RIGHT);
                case 'D': return static_cast<int>(EditorKey::ARROW_LEFT);
                case 'H': return static_cast<int>(EditorKey::HOME);
                case 'F': return static_cast<int>(EditorKey::END);
            }
        }
    } else if (seq[0] == 'O') {
        switch (seq[1]) {
            case 'H': return static_cast<int>(EditorKey::HOME);
            case 'F': return static_cast<int>(EditorKey::END);
        }
    }

    return static_cast<int>(EditorKey::ESCAPE);
}

int Terminal::readKey() {
    char c;
    while (true) {
        int n = read(STDIN_FILENO, &c, 1);
        if (n == 1) break;
        if (n == -1 && errno != EAGAIN)
            throw std::runtime_error("read failed");
    }

    if (c != '\x1b') return c;

    // Try to read escape sequence
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1) return static_cast<int>(EditorKey::ESCAPE);
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return static_cast<int>(EditorKey::ESCAPE);

    int len = 2;

    // Tilde sequences have one more byte
    if (seq[0] == '[' && seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) == 1) len = 3;
    }

    return parseEscapeSequence(seq, len);
}
