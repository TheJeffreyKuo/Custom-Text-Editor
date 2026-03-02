#include "diffviewer.hpp"
#include <algorithm>
#include <cstdio>

DiffViewer::DiffViewer(Terminal& terminal, const DiffResult& diff,
                       const std::string& leftName, const std::string& rightName)
    : terminal_(terminal), diff_(diff),
      leftName_(leftName), rightName_(rightName),
      screen_(terminal.getSize()) {
    screen_.rows -= 1;
}

void DiffViewer::run() {
    while (running_) {
        refreshScreen();
        int key = terminal_.readKey();
        processKeypress(key);
    }
    terminal_.flush("\x1b[2J\x1b[H");
}

int DiffViewer::maxScroll() const {
    int total = static_cast<int>(diff_.lines.size());
    return std::max(0, total - screen_.rows);
}

void DiffViewer::refreshScreen() {
    if (terminal_.wasResized()) {
        screen_ = terminal_.getSize();
        screen_.rows -= 1;
    }

    std::string buf;
    buf.append("\x1b[?25l");
    buf.append("\x1b[H");

    drawRows(buf);
    drawStatusBar(buf);

    buf.append("\x1b[?25h");
    terminal_.flush(buf);
}

void DiffViewer::drawRows(std::string& buf) {
    int leftWidth = (screen_.cols - 1) / 2;
    int rightWidth = screen_.cols - leftWidth - 1;
    int totalLines = static_cast<int>(diff_.lines.size());

    for (int y = 0; y < screen_.rows; y++) {
        int idx = y + scrollOffset_;

        if (idx >= totalLines) {
            buf.append("~");
            for (int p = 1; p < leftWidth; p++) buf.push_back(' ');
            buf.append("\x1b[90m|\x1b[m");
            buf.append("~");
            for (int p = 1; p < rightWidth; p++) buf.push_back(' ');
        } else {
            const DiffLine& line = diff_.lines[idx];

            // Left pane
            if (line.type == DiffLine::REMOVED) {
                buf.append("\x1b[41m");
                int len = std::min(static_cast<int>(line.text.size()), leftWidth);
                buf.append(line.text, 0, len);
                for (int p = len; p < leftWidth; p++) buf.push_back(' ');
                buf.append("\x1b[m");
            } else if (line.type == DiffLine::ADDED) {
                buf.append("\x1b[42m");
                for (int p = 0; p < leftWidth; p++) buf.push_back(' ');
                buf.append("\x1b[m");
            } else {
                int len = std::min(static_cast<int>(line.text.size()), leftWidth);
                buf.append(line.text, 0, len);
                for (int p = len; p < leftWidth; p++) buf.push_back(' ');
            }

            // Divider
            buf.append("\x1b[90m|\x1b[m");

            // Right pane
            if (line.type == DiffLine::ADDED) {
                buf.append("\x1b[42m");
                int len = std::min(static_cast<int>(line.text.size()), rightWidth);
                buf.append(line.text, 0, len);
                for (int p = len; p < rightWidth; p++) buf.push_back(' ');
                buf.append("\x1b[m");
            } else if (line.type == DiffLine::REMOVED) {
                buf.append("\x1b[41m");
                for (int p = 0; p < rightWidth; p++) buf.push_back(' ');
                buf.append("\x1b[m");
            } else {
                int len = std::min(static_cast<int>(line.text.size()), rightWidth);
                buf.append(line.text, 0, len);
                for (int p = len; p < rightWidth; p++) buf.push_back(' ');
            }
        }

        buf.append("\x1b[K");
        buf.append("\r\n");
    }
}

void DiffViewer::drawStatusBar(std::string& buf) {
    buf.append("\x1b[7m");

    char left[256];
    int leftLen = snprintf(left, sizeof(left), " %s | %s",
        leftName_.c_str(), rightName_.c_str());
    if (leftLen > screen_.cols) leftLen = screen_.cols;

    char right[256];
    int rightLen = snprintf(right, sizeof(right),
        "+%d / -%d / %d hunks | n/N = hunk | q = quit ",
        diff_.addedCount, diff_.removedCount, diff_.hunkCount);

    buf.append(left, leftLen);

    int padding = screen_.cols - leftLen - rightLen;
    if (padding > 0) {
        buf.append(padding, ' ');
        buf.append(right, rightLen);
    } else {
        int remaining = screen_.cols - leftLen;
        if (remaining > 0) buf.append(remaining, ' ');
    }

    buf.append("\x1b[m");
}

void DiffViewer::jumpToHunk(int index) {
    if (index < 0 || index >= diff_.hunkCount) return;
    currentHunk_ = index;
    scrollOffset_ = diff_.hunkStarts[index] - screen_.rows / 3;
    if (scrollOffset_ < 0) scrollOffset_ = 0;
    if (scrollOffset_ > maxScroll()) scrollOffset_ = maxScroll();
}

void DiffViewer::processKeypress(int key) {
    switch (key) {
        case 'q':
            running_ = false;
            break;

        case static_cast<int>(EditorKey::ARROW_UP):
            if (scrollOffset_ > 0) scrollOffset_--;
            break;

        case static_cast<int>(EditorKey::ARROW_DOWN):
            if (scrollOffset_ < maxScroll()) scrollOffset_++;
            break;

        case static_cast<int>(EditorKey::PAGE_UP):
            scrollOffset_ -= screen_.rows;
            if (scrollOffset_ < 0) scrollOffset_ = 0;
            break;

        case static_cast<int>(EditorKey::PAGE_DOWN):
            scrollOffset_ += screen_.rows;
            if (scrollOffset_ > maxScroll()) scrollOffset_ = maxScroll();
            break;

        case static_cast<int>(EditorKey::HOME):
            scrollOffset_ = 0;
            break;

        case static_cast<int>(EditorKey::END):
            scrollOffset_ = maxScroll();
            break;

        case 'n':
            if (currentHunk_ < diff_.hunkCount - 1)
                jumpToHunk(currentHunk_ + 1);
            break;

        case 'N':
            if (currentHunk_ > 0)
                jumpToHunk(currentHunk_ - 1);
            break;
    }
}
