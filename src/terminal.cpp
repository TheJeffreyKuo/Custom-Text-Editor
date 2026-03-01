#include "terminal.hpp"
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <stdexcept>
#include <sys/ioctl.h>
#include <unistd.h>

static volatile sig_atomic_t g_winch_received = 0;

static void sigwinchHandler(int) {
    g_winch_received = 1;
}

Terminal::Terminal() {
    enableRawMode();
    installSignalHandlers();
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

void Terminal::installSignalHandlers() {
    struct sigaction sa {};
    sa.sa_handler = sigwinchHandler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGWINCH, &sa, nullptr);
}

bool Terminal::wasResized() {
    if (g_winch_received) {
        g_winch_received = 0;
        return true;
    }
    return false;
}

TerminalSize Terminal::getSize() const {
    winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1 && ws.ws_col != 0)
        return {ws.ws_row, ws.ws_col};

    // Fallback: move cursor to bottom-right and query position
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B\x1b[6n", 18) != 18)
        throw std::runtime_error("getSize fallback failed");

    char buf[32];
    int i = 0;
    while (i < (int)sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    int rows, cols;
    if (buf[0] != '\x1b' || buf[1] != '[') throw std::runtime_error("getSize parse failed");
    if (sscanf(&buf[2], "%d;%d", &rows, &cols) != 2) throw std::runtime_error("getSize parse failed");

    return {rows, cols};
}

void Terminal::flush(const std::string& buf) {
    const char* data = buf.data();
    size_t remaining = buf.size();
    while (remaining > 0) {
        ssize_t written = write(STDOUT_FILENO, data, remaining);
        if (written == -1) {
            if (errno == EAGAIN) continue;
            throw std::runtime_error("write failed");
        }
        data += written;
        remaining -= written;
    }
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
        if (n == -1 && errno == EINTR) return 0;
        if (n == -1 && errno != EAGAIN)
            throw std::runtime_error("read failed");
    }

    if (c != '\x1b') return c;

    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1) return static_cast<int>(EditorKey::ESCAPE);
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return static_cast<int>(EditorKey::ESCAPE);

    int len = 2;
    if (seq[0] == '[' && seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) == 1) len = 3;
    }

    return parseEscapeSequence(seq, len);
}
