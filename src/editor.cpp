#include "editor.hpp"
#include <cstdio>

static const char* VERSION = "0.0.1";

Editor::Editor(const std::string& filename) : screen_(terminal_.getSize()) {
    screen_.rows -= 2;
    if (!filename.empty())
        buffer_.open(filename);
    setStatusMessage("HELP: Ctrl-S save | Ctrl-Q quit | Ctrl-Z undo | Ctrl-Y redo | Ctrl-F find | Ctrl-R replace");
}

void Editor::run() {
    while (running_) {
        refreshScreen();
        int key = terminal_.readKey();
        processKeypress(key);
    }
}

void Editor::setStatusMessage(const std::string& msg) {
    statusMsg_ = msg;
    statusMsgTime_ = std::time(nullptr);
}

void Editor::recordAction(EditAction action) {
    action.cursorRow = cy_;
    action.cursorCol = cx_;
    action.timestamp = std::time(nullptr);
    undoStack_.push_back(std::move(action));
    redoStack_.clear();
    statsDirty_ = true;
}

void Editor::undo() {
    if (undoStack_.empty()) {
        setStatusMessage("Nothing to undo");
        return;
    }

    auto undoOne = [&]() {
        EditAction action = std::move(undoStack_.back());
        undoStack_.pop_back();

        EditAction redoAction;
        switch (action.type) {
            case EditAction::INSERT_CHAR:
                redoAction = buffer_.deleteChar(action.row, action.col);
                break;
            case EditAction::DELETE_CHAR:
                redoAction = buffer_.insertChar(action.row, action.col, action.ch);
                break;
            case EditAction::SPLIT_LINE:
                redoAction = buffer_.joinLines(action.row);
                break;
            case EditAction::JOIN_LINES:
                redoAction = buffer_.splitLine(action.row, action.col);
                break;
        }

        redoAction.cursorRow = cy_;
        redoAction.cursorCol = cx_;
        redoAction.timestamp = action.timestamp;
        redoStack_.push_back(std::move(redoAction));

        cy_ = action.cursorRow;
        cx_ = action.cursorCol;

        return action;
    };

    EditAction last = undoOne();

    while (!undoStack_.empty()) {
        const EditAction& prev = undoStack_.back();
        if (prev.type != last.type) break;
        if (prev.type == EditAction::SPLIT_LINE || prev.type == EditAction::JOIN_LINES) break;
        if (last.timestamp - prev.timestamp > 1) break;
        last = undoOne();
    }

    statsDirty_ = true;
    setStatusMessage("Undo");
}

void Editor::redo() {
    if (redoStack_.empty()) {
        setStatusMessage("Nothing to redo");
        return;
    }

    auto redoOne = [&]() {
        EditAction action = std::move(redoStack_.back());
        redoStack_.pop_back();

        EditAction undoAction;
        switch (action.type) {
            case EditAction::INSERT_CHAR:
                undoAction = buffer_.deleteChar(action.row, action.col);
                break;
            case EditAction::DELETE_CHAR:
                undoAction = buffer_.insertChar(action.row, action.col, action.ch);
                break;
            case EditAction::SPLIT_LINE:
                undoAction = buffer_.joinLines(action.row);
                break;
            case EditAction::JOIN_LINES:
                undoAction = buffer_.splitLine(action.row, action.col);
                break;
        }

        undoAction.cursorRow = cy_;
        undoAction.cursorCol = cx_;
        undoAction.timestamp = action.timestamp;
        undoStack_.push_back(std::move(undoAction));

        cy_ = action.cursorRow;
        cx_ = action.cursorCol;

        return action;
    };

    EditAction last = redoOne();

    while (!redoStack_.empty()) {
        const EditAction& next = redoStack_.back();
        if (next.type != last.type) break;
        if (next.type == EditAction::SPLIT_LINE || next.type == EditAction::JOIN_LINES) break;
        if (next.timestamp - last.timestamp > 1) break;
        last = redoOne();
    }

    statsDirty_ = true;
    setStatusMessage("Redo");
}

void Editor::find() {
    int savedCx = cx_, savedCy = cy_;
    int savedRowOffset = rowOffset_, savedColOffset = colOffset_;
    int searchDirection = 1;
    int lastMatchRow = -1;

    auto callback = [&](const std::string& query, int key) {
        if (key == static_cast<int>(EditorKey::ARROW_DOWN) ||
            key == static_cast<int>(EditorKey::ARROW_RIGHT)) {
            searchDirection = 1;
        } else if (key == static_cast<int>(EditorKey::ARROW_UP) ||
                   key == static_cast<int>(EditorKey::ARROW_LEFT)) {
            searchDirection = -1;
        } else if (key == static_cast<int>(EditorKey::ESCAPE)) {
            cx_ = savedCx;
            cy_ = savedCy;
            rowOffset_ = savedRowOffset;
            colOffset_ = savedColOffset;
            return;
        } else {
            searchDirection = 1;
            lastMatchRow = -1;
        }

        if (query.empty()) return;

        int startRow = (lastMatchRow == -1) ? cy_ : lastMatchRow + searchDirection;

        for (int i = 0; i < buffer_.numRows(); i++) {
            int row = (startRow + i * searchDirection + buffer_.numRows())
                      % buffer_.numRows();
            const std::string& chars = buffer_.getRow(row).chars;
            auto pos = chars.find(query);
            if (pos != std::string::npos) {
                lastMatchRow = row;
                cy_ = row;
                cx_ = static_cast<int>(pos);
                break;
            }
        }

        // Count matches for display
        int total = 0, current = 0;
        for (int r = 0; r < buffer_.numRows(); r++) {
            size_t searchPos = 0;
            while ((searchPos = buffer_.getRow(r).chars.find(query, searchPos))
                    != std::string::npos) {
                total++;
                if (r < cy_ || (r == cy_ && static_cast<int>(searchPos) <= cx_))
                    current++;
                searchPos += query.size();
            }
        }

        if (total > 0)
            setStatusMessage("Search: " + query + " [" + std::to_string(current)
                            + "/" + std::to_string(total) + "]");
        else
            setStatusMessage("Search: " + query + " [no matches]");
    };

    std::string result = prompt("Search: ", callback);
    if (result.empty())
        setStatusMessage("");
}

void Editor::findAndReplace() {
    std::string searchTerm = prompt("Search: ");
    if (searchTerm.empty()) return;

    std::string replacement = prompt("Replace with: ");
    if (replacement.empty()) {
        setStatusMessage("Replace cancelled");
        return;
    }

    int replaced = 0;
    int total = 0;

    for (int r = 0; r < buffer_.numRows(); r++) {
        size_t pos = 0;
        while ((pos = buffer_.getRow(r).chars.find(searchTerm, pos))
                != std::string::npos) {
            total++;
            pos += searchTerm.size();
        }
    }

    if (total == 0) {
        setStatusMessage("No matches found");
        return;
    }

    bool done = false;
    bool replaceAll = false;

    for (int r = 0; r < buffer_.numRows() && !done; r++) {
        size_t pos = 0;
        while ((pos = buffer_.getRow(r).chars.find(searchTerm, pos))
                != std::string::npos) {
            cy_ = r;
            cx_ = static_cast<int>(pos);

            if (!replaceAll) {
                setStatusMessage("Replace? y = yes, n = skip, a = all, Esc = stop ["
                    + std::to_string(replaced) + "/" + std::to_string(total) + "]");
                refreshScreen();
                int key = terminal_.readKey();

                if (key == 'n') {
                    pos += searchTerm.size();
                    continue;
                } else if (key == 'a') {
                    replaceAll = true;
                } else if (key == static_cast<int>(EditorKey::ESCAPE)) {
                    done = true;
                    break;
                } else if (key != 'y') {
                    pos += searchTerm.size();
                    continue;
                }
            }

            for (int i = 0; i < static_cast<int>(searchTerm.size()); i++)
                recordAction(buffer_.deleteChar(r, static_cast<int>(pos)));

            for (int i = 0; i < static_cast<int>(replacement.size()); i++)
                recordAction(buffer_.insertChar(r, static_cast<int>(pos) + i,
                             replacement[i]));

            replaced++;
            pos += replacement.size();
        }
    }

    setStatusMessage("Replaced " + std::to_string(replaced) + " of "
                    + std::to_string(total) + " occurrences");
}

void Editor::scroll() {
    rx_ = 0;
    if (cy_ < buffer_.numRows())
        rx_ = charsToRender(buffer_.getRow(cy_), cx_);

    if (cy_ < rowOffset_)
        rowOffset_ = cy_;
    if (cy_ >= rowOffset_ + screen_.rows)
        rowOffset_ = cy_ - screen_.rows + 1;

    if (rx_ < colOffset_)
        colOffset_ = rx_;
    if (rx_ >= colOffset_ + screen_.cols)
        colOffset_ = rx_ - screen_.cols + 1;
}

void Editor::refreshScreen() {
    if (terminal_.wasResized()) {
        screen_ = terminal_.getSize();
        screen_.rows -= 2;
    }

    int rowLen = (cy_ < buffer_.numRows())
        ? static_cast<int>(buffer_.getRow(cy_).chars.size()) : 0;
    if (cx_ > rowLen) cx_ = rowLen;

    scroll();

    std::string buf;
    buf.append("\x1b[?25l");
    buf.append("\x1b[H");

    drawRows(buf);
    drawStatusBar(buf);
    drawMessageBar(buf);

    char cursor[32];
    snprintf(cursor, sizeof(cursor), "\x1b[%d;%dH",
        cy_ - rowOffset_ + 1, rx_ - colOffset_ + 1);
    buf.append(cursor);

    buf.append("\x1b[?25h");
    terminal_.flush(buf);
}

void Editor::drawRows(std::string& buf) {
    for (int y = 0; y < screen_.rows; y++) {
        int fileRow = y + rowOffset_;

        if (fileRow >= buffer_.numRows()) {
            if (buffer_.numRows() == 0 && y == screen_.rows / 3) {
                char welcome[80];
                int len = snprintf(welcome, sizeof(welcome),
                    "Custom Text Editor -- version %s", VERSION);
                if (len > screen_.cols) len = screen_.cols;

                int padding = (screen_.cols - len) / 2;
                if (padding > 0) {
                    buf.append("~");
                    padding--;
                }
                while (padding-- > 0) buf.append(" ");
                buf.append(welcome, len);
            } else {
                buf.append("~");
            }
        } else {
            const std::string& render = buffer_.getRow(fileRow).render;
            int len = static_cast<int>(render.size()) - colOffset_;
            if (len < 0) len = 0;
            if (len > screen_.cols) len = screen_.cols;
            if (len > 0)
                buf.append(render, colOffset_, len);
        }

        buf.append("\x1b[K");
        buf.append("\r\n");
    }
}

void Editor::drawStatusBar(std::string& buf) {
    if (statsDirty_) {
        stats_ = computeStats(buffer_);
        statsDirty_ = false;
    }

    buf.append("\x1b[7m");

    const std::string& fname = buffer_.getFilename();
    FileType ft = detectFileType(fname);

    char left[256];
    int leftLen = snprintf(left, sizeof(left), " %.40s%s -- %s",
        fname.empty() ? "[No Name]" : fname.c_str(),
        buffer_.isDirty() ? " (modified)" : "",
        ft.name.c_str());
    if (leftLen > screen_.cols) leftLen = screen_.cols;

    char right[256];
    int rightLen = snprintf(right, sizeof(right),
        "Ln %d, Col %d | %d lines | %d chars | %d words ",
        cy_ + 1, rx_ + 1,
        stats_.totalLines, stats_.totalChars, stats_.totalWords);

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
    buf.append("\r\n");
}

void Editor::drawMessageBar(std::string& buf) {
    buf.append("\x1b[K");
    if (!statusMsg_.empty() && std::time(nullptr) - statusMsgTime_ < 5) {
        int len = static_cast<int>(statusMsg_.size());
        if (len > screen_.cols) len = screen_.cols;
        buf.append(statusMsg_, 0, len);
    }
}

std::string Editor::prompt(const std::string& message,
                           PromptCallback callback) {
    std::string input;

    setStatusMessage(message);
    refreshScreen();

    while (true) {
        int key = terminal_.readKey();

        if (key == '\r' && !input.empty()) {
            setStatusMessage("");
            if (callback) callback(input, key);
            return input;
        } else if (key == static_cast<int>(EditorKey::ESCAPE)) {
            setStatusMessage("");
            if (callback) callback(input, key);
            return "";
        } else if (key == 127 || key == ctrlKey('h')) {
            if (!input.empty()) input.pop_back();
        } else if (key >= 32 && key <= 126) {
            input.push_back(static_cast<char>(key));
        }

        setStatusMessage(message + input);
        if (callback) callback(input, key);
        refreshScreen();
    }
}

void Editor::processKeypress(int key) {
    if (key != ctrlKey('q')) quitConfirm_ = 0;

    switch (key) {
        case ctrlKey('q'):
            if (buffer_.isDirty() && quitConfirm_ < 1) {
                setStatusMessage("File has unsaved changes. Ctrl-Q again to quit.");
                quitConfirm_++;
                break;
            }
            running_ = false;
            terminal_.flush("\x1b[2J\x1b[H");
            break;

        case ctrlKey('s'): {
            int bytes = buffer_.save();
            if (bytes == -1 && buffer_.getFilename().empty()) {
                std::string filename = prompt("Save as: ");
                if (filename.empty()) {
                    setStatusMessage("Save aborted");
                    break;
                }
                bytes = buffer_.save(filename);
            }
            if (bytes >= 0)
                setStatusMessage(std::to_string(bytes) + " bytes written to "
                                + buffer_.getFilename());
            else
                setStatusMessage("Save failed!");
            statsDirty_ = true;
            break;
        }

        case ctrlKey('z'):
            undo();
            break;

        case ctrlKey('y'):
            redo();
            break;

        case ctrlKey('f'):
            find();
            break;

        case ctrlKey('r'):
            findAndReplace();
            break;

        case '\r':
            recordAction(buffer_.splitLine(cy_, cx_));
            cy_++;
            cx_ = 0;
            break;

        case 127:
        case ctrlKey('h'):
            if (cx_ > 0) {
                recordAction(buffer_.deleteChar(cy_, cx_ - 1));
                cx_--;
            } else if (cy_ > 0) {
                cx_ = static_cast<int>(buffer_.getRow(cy_ - 1).chars.size());
                recordAction(buffer_.joinLines(cy_ - 1));
                cy_--;
            }
            break;

        case static_cast<int>(EditorKey::DEL):
            if (cy_ < buffer_.numRows()) {
                int rowLen = static_cast<int>(buffer_.getRow(cy_).chars.size());
                if (cx_ < rowLen) {
                    recordAction(buffer_.deleteChar(cy_, cx_));
                } else if (cy_ < buffer_.numRows() - 1) {
                    recordAction(buffer_.joinLines(cy_));
                }
            }
            break;

        case '\t':
            recordAction(buffer_.insertChar(cy_, cx_, '\t'));
            cx_++;
            break;

        case static_cast<int>(EditorKey::ARROW_UP):
        case static_cast<int>(EditorKey::ARROW_DOWN):
        case static_cast<int>(EditorKey::ARROW_LEFT):
        case static_cast<int>(EditorKey::ARROW_RIGHT):
        case static_cast<int>(EditorKey::HOME):
        case static_cast<int>(EditorKey::END):
        case static_cast<int>(EditorKey::PAGE_UP):
        case static_cast<int>(EditorKey::PAGE_DOWN):
            moveCursor(key);
            break;

        default:
            if (key >= 32 && key <= 126) {
                recordAction(buffer_.insertChar(cy_, cx_, key));
                cx_++;
            }
            break;
    }
}

void Editor::moveCursor(int key) {
    int numRows = buffer_.numRows();
    int rowLen = (cy_ < numRows)
        ? static_cast<int>(buffer_.getRow(cy_).chars.size()) : 0;

    switch (key) {
        case static_cast<int>(EditorKey::ARROW_LEFT):
            if (cx_ > 0) {
                cx_--;
            } else if (cy_ > 0) {
                cy_--;
                cx_ = static_cast<int>(buffer_.getRow(cy_).chars.size());
            }
            break;
        case static_cast<int>(EditorKey::ARROW_RIGHT):
            if (cx_ < rowLen) {
                cx_++;
            } else if (cy_ < numRows - 1) {
                cy_++;
                cx_ = 0;
            }
            break;
        case static_cast<int>(EditorKey::ARROW_UP):
            if (cy_ > 0) cy_--;
            break;
        case static_cast<int>(EditorKey::ARROW_DOWN):
            if (cy_ < numRows) cy_++;
            break;
        case static_cast<int>(EditorKey::HOME):
            cx_ = 0;
            break;
        case static_cast<int>(EditorKey::END):
            if (cy_ < numRows)
                cx_ = static_cast<int>(buffer_.getRow(cy_).chars.size());
            break;
        case static_cast<int>(EditorKey::PAGE_UP):
            cy_ = rowOffset_;
            for (int i = 0; i < screen_.rows; i++)
                if (cy_ > 0) cy_--;
            break;
        case static_cast<int>(EditorKey::PAGE_DOWN):
            cy_ = rowOffset_ + screen_.rows - 1;
            for (int i = 0; i < screen_.rows; i++)
                if (cy_ < numRows) cy_++;
            break;
    }
}
