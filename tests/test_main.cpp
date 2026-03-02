#include "terminal.hpp"
#include "buffer.hpp"
#include "diff.hpp"
#include "filetype.hpp"
#include <cassert>
#include <iostream>

// --- Escape sequence tests ---

void test_arrow_keys() {
    assert(parseEscapeSequence("[A", 2) == (int)EditorKey::ARROW_UP);
    assert(parseEscapeSequence("[B", 2) == (int)EditorKey::ARROW_DOWN);
    assert(parseEscapeSequence("[C", 2) == (int)EditorKey::ARROW_RIGHT);
    assert(parseEscapeSequence("[D", 2) == (int)EditorKey::ARROW_LEFT);
}

void test_home_end_variants() {
    assert(parseEscapeSequence("[H", 2) == (int)EditorKey::HOME);
    assert(parseEscapeSequence("[F", 2) == (int)EditorKey::END);
    assert(parseEscapeSequence("[1~", 3) == (int)EditorKey::HOME);
    assert(parseEscapeSequence("[4~", 3) == (int)EditorKey::END);
    assert(parseEscapeSequence("[7~", 3) == (int)EditorKey::HOME);
    assert(parseEscapeSequence("[8~", 3) == (int)EditorKey::END);
    assert(parseEscapeSequence("OH", 2) == (int)EditorKey::HOME);
    assert(parseEscapeSequence("OF", 2) == (int)EditorKey::END);
}

void test_page_and_delete() {
    assert(parseEscapeSequence("[5~", 3) == (int)EditorKey::PAGE_UP);
    assert(parseEscapeSequence("[6~", 3) == (int)EditorKey::PAGE_DOWN);
    assert(parseEscapeSequence("[3~", 3) == (int)EditorKey::DEL);
}

void test_unrecognized() {
    assert(parseEscapeSequence("[X", 2) == (int)EditorKey::ESCAPE);
    assert(parseEscapeSequence("[9~", 3) == (int)EditorKey::ESCAPE);
}

// --- Tab rendering tests ---

void test_chars_to_render() {
    Row row;
    row.chars = "ab\tcd";

    assert(charsToRender(row, 0) == 0);
    assert(charsToRender(row, 1) == 1);
    assert(charsToRender(row, 2) == 2);
    assert(charsToRender(row, 3) == 8);
    assert(charsToRender(row, 4) == 9);
    assert(charsToRender(row, 5) == 10);
}

void test_render_to_chars() {
    Row row;
    row.chars = "ab\tcd";

    assert(renderToChars(row, 0) == 0);
    assert(renderToChars(row, 1) == 1);
    assert(renderToChars(row, 2) == 2);
    assert(renderToChars(row, 8) == 3);
    assert(renderToChars(row, 9) == 4);
}

void test_no_tabs() {
    Row row;
    row.chars = "hello";

    for (int i = 0; i <= 5; i++)
        assert(charsToRender(row, i) == i);
}

void test_multiple_tabs() {
    Row row;
    row.chars = "\t\t";

    assert(charsToRender(row, 0) == 0);
    assert(charsToRender(row, 1) == 8);
    assert(charsToRender(row, 2) == 16);
}

// --- Edit operation tests ---

void test_insert_char() {
    Buffer buf;
    buf.insertChar(0, 0, 'H');
    assert(buf.numRows() == 1);
    assert(buf.getRow(0).chars == "H");

    buf.insertChar(0, 1, 'i');
    assert(buf.getRow(0).chars == "Hi");

    buf.insertChar(0, 0, '!');
    assert(buf.getRow(0).chars == "!Hi");
}

void test_delete_char() {
    Buffer buf;
    buf.insertChar(0, 0, 'a');
    buf.insertChar(0, 1, 'b');
    buf.insertChar(0, 2, 'c');

    auto action = buf.deleteChar(0, 2);
    assert(buf.getRow(0).chars == "ab");
    assert(action.ch == 'c');

    action = buf.deleteChar(0, 0);
    assert(buf.getRow(0).chars == "b");
    assert(action.ch == 'a');
}

void test_split_line() {
    Buffer buf;
    buf.insertChar(0, 0, 'a');
    buf.insertChar(0, 1, 'b');
    buf.insertChar(0, 2, 'c');
    buf.insertChar(0, 3, 'd');

    buf.splitLine(0, 2);
    assert(buf.numRows() == 2);
    assert(buf.getRow(0).chars == "ab");
    assert(buf.getRow(1).chars == "cd");
}

void test_join_lines() {
    Buffer buf;
    buf.insertChar(0, 0, 'a');
    buf.insertChar(0, 1, 'b');
    buf.splitLine(0, 2);
    buf.insertChar(1, 0, 'c');
    buf.insertChar(1, 1, 'd');

    auto action = buf.joinLines(0);
    assert(buf.numRows() == 1);
    assert(buf.getRow(0).chars == "abcd");
    assert(action.text == "cd");
}

void test_dirty_flag() {
    Buffer buf;
    assert(!buf.isDirty());
    buf.insertChar(0, 0, 'x');
    assert(buf.isDirty());
}

void test_action_types() {
    Buffer buf;
    auto a1 = buf.insertChar(0, 0, 'x');
    assert(a1.type == EditAction::INSERT_CHAR);
    assert(a1.ch == 'x');

    auto a2 = buf.deleteChar(0, 0);
    assert(a2.type == EditAction::DELETE_CHAR);

    buf.insertChar(0, 0, 'a');
    auto a3 = buf.splitLine(0, 1);
    assert(a3.type == EditAction::SPLIT_LINE);

    auto a4 = buf.joinLines(0);
    assert(a4.type == EditAction::JOIN_LINES);
}

// --- File type detection tests ---

void test_filetype_detection() {
    assert(detectFileType("main.cpp").name == "C++");
    assert(detectFileType("header.hpp").name == "C++");
    assert(detectFileType("main.c").name == "C");
    assert(detectFileType("script.py").name == "Python");
    assert(detectFileType("Makefile").name == "Makefile");
    assert(detectFileType("CMakeLists.txt").name == "Makefile");
    assert(detectFileType("readme.md").name == "Markdown");
    assert(detectFileType("unknown.xyz").name == "Plain Text");
    assert(detectFileType("").name == "Plain Text");
}

// --- Document stats tests ---

void test_word_count_basic() {
    Buffer buf;
    buf.insertChar(0, 0, 'h');
    buf.insertChar(0, 1, 'i');

    auto stats = computeStats(buf);
    assert(stats.totalWords == 1);
    assert(stats.totalChars == 2);
    assert(stats.totalLines == 1);
}

void test_word_count_multiple() {
    Buffer buf;
    const char* text = "hello world";
    for (int i = 0; text[i]; i++)
        buf.insertChar(0, i, text[i]);

    auto stats = computeStats(buf);
    assert(stats.totalWords == 2);
}

void test_word_count_edge_cases() {
    Buffer buf;
    auto stats = computeStats(buf);
    assert(stats.totalWords == 0);
    assert(stats.totalChars == 0);

    buf.insertChar(0, 0, ' ');
    buf.insertChar(0, 1, ' ');
    buf.insertChar(0, 2, ' ');
    stats = computeStats(buf);
    assert(stats.totalWords == 0);

    buf.deleteChar(0, 0);
    buf.deleteChar(0, 0);
    buf.deleteChar(0, 0);
    buf.insertChar(0, 0, 'a');
    buf.insertChar(0, 1, ' ');
    buf.insertChar(0, 2, ' ');
    buf.insertChar(0, 3, 'b');
    stats = computeStats(buf);
    assert(stats.totalWords == 2);
}

// --- Undo reversal tests ---

void test_undo_insert() {
    Buffer buf;
    buf.insertChar(0, 0, 'a');
    buf.insertChar(0, 1, 'b');
    buf.insertChar(0, 2, 'c');

    buf.deleteChar(0, 2);
    assert(buf.getRow(0).chars == "ab");

    buf.deleteChar(0, 1);
    assert(buf.getRow(0).chars == "a");
}

void test_undo_delete() {
    Buffer buf;
    buf.insertChar(0, 0, 'a');
    buf.insertChar(0, 1, 'b');
    buf.insertChar(0, 2, 'c');

    auto action = buf.deleteChar(0, 1);

    buf.insertChar(action.row, action.col, action.ch);
    assert(buf.getRow(0).chars == "abc");
}

void test_undo_split() {
    Buffer buf;
    buf.insertChar(0, 0, 'a');
    buf.insertChar(0, 1, 'b');
    buf.insertChar(0, 2, 'c');
    buf.insertChar(0, 3, 'd');

    auto action = buf.splitLine(0, 2);
    assert(buf.numRows() == 2);

    buf.joinLines(action.row);
    assert(buf.numRows() == 1);
    assert(buf.getRow(0).chars == "abcd");
}

void test_undo_join() {
    Buffer buf;
    buf.insertChar(0, 0, 'a');
    buf.insertChar(0, 1, 'b');
    buf.splitLine(0, 2);
    buf.insertChar(1, 0, 'c');
    buf.insertChar(1, 1, 'd');

    auto action = buf.joinLines(0);
    assert(buf.numRows() == 1);

    buf.splitLine(action.row, action.col);
    assert(buf.numRows() == 2);
    assert(buf.getRow(0).chars == "ab");
    assert(buf.getRow(1).chars == "cd");
}

// --- Syntax highlighting tests ---

void test_highlight_numbers() {
    Row row;
    row.chars = "x = 42;";
    row.render = row.chars;
    row.hl.resize(row.render.size(), HL_NORMAL);

    FileType ft = cppSyntax();
    highlightRow(row, ft, false);

    assert(row.hl[4] == HL_NUMBER);
    assert(row.hl[5] == HL_NUMBER);
    assert(row.hl[0] == HL_NORMAL);
}

void test_highlight_string() {
    Row row;
    row.chars = R"(char* s = "hello";)";
    row.render = row.chars;
    row.hl.resize(row.render.size(), HL_NORMAL);

    FileType ft = cppSyntax();
    highlightRow(row, ft, false);

    int qStart = static_cast<int>(row.render.find('"'));
    int qEnd = static_cast<int>(row.render.rfind('"'));
    for (int i = qStart; i <= qEnd; i++)
        assert(row.hl[i] == HL_STRING);
}

void test_highlight_comment() {
    Row row;
    row.chars = "int x; // comment";
    row.render = row.chars;
    row.hl.resize(row.render.size(), HL_NORMAL);

    FileType ft = cppSyntax();
    highlightRow(row, ft, false);

    int commentStart = static_cast<int>(row.render.find("//"));
    for (int i = commentStart; i < static_cast<int>(row.render.size()); i++)
        assert(row.hl[i] == HL_COMMENT);

    assert(row.hl[0] == HL_KEYWORD2);
}

void test_highlight_multiline_comment() {
    Row row1, row2, row3;
    row1.chars = "/* start";
    row1.render = row1.chars;
    row1.hl.resize(row1.render.size(), HL_NORMAL);

    row2.chars = "middle";
    row2.render = row2.chars;
    row2.hl.resize(row2.render.size(), HL_NORMAL);

    row3.chars = "end */";
    row3.render = row3.chars;
    row3.hl.resize(row3.render.size(), HL_NORMAL);

    FileType ft = cppSyntax();
    highlightRow(row1, ft, false);
    assert(row1.openComment == true);

    highlightRow(row2, ft, row1.openComment);
    assert(row2.openComment == true);
    for (auto h : row2.hl) assert(h == HL_MLCOMMENT);

    highlightRow(row3, ft, row2.openComment);
    assert(row3.openComment == false);
}

void test_highlight_invalidation() {
    Row row1, row2;
    row1.chars = "/* hello";
    row1.render = row1.chars;
    row1.hl.resize(row1.render.size(), HL_NORMAL);

    row2.chars = "world";
    row2.render = row2.chars;
    row2.hl.resize(row2.render.size(), HL_NORMAL);

    FileType ft = cppSyntax();
    highlightRow(row1, ft, false);
    highlightRow(row2, ft, row1.openComment);

    for (auto h : row2.hl) assert(h == HL_MLCOMMENT);

    row1.chars = "/* hello */";
    row1.render = row1.chars;
    row1.hl.resize(row1.render.size(), HL_NORMAL);
    highlightRow(row1, ft, false);
    assert(row1.openComment == false);

    highlightRow(row2, ft, row1.openComment);
    for (auto h : row2.hl) assert(h == HL_NORMAL);
}

// --- Replace offset tests ---

void test_replace_shorter() {
    Buffer buf;
    const char* text = "hello hello";
    for (int i = 0; text[i]; i++)
        buf.insertChar(0, i, text[i]);

    for (int i = 0; i < 5; i++) buf.deleteChar(0, 0);
    buf.insertChar(0, 0, 'h');
    buf.insertChar(0, 1, 'i');
    assert(buf.getRow(0).chars == "hi hello");

    auto pos = buf.getRow(0).chars.find("hello", 2);
    assert(pos == 3);
}

void test_replace_longer() {
    Buffer buf;
    const char* text = "hi hi";
    for (int i = 0; text[i]; i++)
        buf.insertChar(0, i, text[i]);

    for (int i = 0; i < 2; i++) buf.deleteChar(0, 0);
    const char* rep = "hello";
    for (int i = 0; rep[i]; i++)
        buf.insertChar(0, i, rep[i]);
    assert(buf.getRow(0).chars == "hello hi");

    auto pos = buf.getRow(0).chars.find("hi", 5);
    assert(pos == 6);
}

void test_replace_same_length() {
    Buffer buf;
    const char* text = "cat cat";
    for (int i = 0; text[i]; i++)
        buf.insertChar(0, i, text[i]);

    for (int i = 0; i < 3; i++) buf.deleteChar(0, 0);
    buf.insertChar(0, 0, 'd');
    buf.insertChar(0, 1, 'o');
    buf.insertChar(0, 2, 'g');
    assert(buf.getRow(0).chars == "dog cat");
}

// --- Diff algorithm tests ---

void test_diff_identical() {
    std::vector<std::string> a = {"hello", "world"};
    std::vector<std::string> b = {"hello", "world"};
    auto result = computeDiff(a, b);

    assert(result.lines.size() == 2);
    assert(result.lines[0].type == DiffLine::SAME);
    assert(result.lines[1].type == DiffLine::SAME);
    assert(result.addedCount == 0);
    assert(result.removedCount == 0);
    assert(result.hunkCount == 0);
}

void test_diff_completely_different() {
    std::vector<std::string> a = {"aaa", "bbb"};
    std::vector<std::string> b = {"ccc", "ddd"};
    auto result = computeDiff(a, b);

    assert(result.addedCount == 2);
    assert(result.removedCount == 2);
}

void test_diff_addition() {
    std::vector<std::string> a = {"hello", "world"};
    std::vector<std::string> b = {"hello", "new", "world"};
    auto result = computeDiff(a, b);

    assert(result.addedCount == 1);
    assert(result.removedCount == 0);

    bool foundAdded = false;
    for (const auto& line : result.lines) {
        if (line.type == DiffLine::ADDED && line.text == "new")
            foundAdded = true;
    }
    assert(foundAdded);
}

void test_diff_removal() {
    std::vector<std::string> a = {"hello", "old", "world"};
    std::vector<std::string> b = {"hello", "world"};
    auto result = computeDiff(a, b);

    assert(result.addedCount == 0);
    assert(result.removedCount == 1);
}

void test_diff_empty_left() {
    std::vector<std::string> a = {};
    std::vector<std::string> b = {"hello", "world"};
    auto result = computeDiff(a, b);

    assert(result.addedCount == 2);
    assert(result.removedCount == 0);
}

void test_diff_empty_right() {
    std::vector<std::string> a = {"hello", "world"};
    std::vector<std::string> b = {};
    auto result = computeDiff(a, b);

    assert(result.addedCount == 0);
    assert(result.removedCount == 2);
}

void test_diff_both_empty() {
    std::vector<std::string> a = {};
    std::vector<std::string> b = {};
    auto result = computeDiff(a, b);

    assert(result.lines.empty());
    assert(result.hunkCount == 0);
}

void test_diff_hunks() {
    std::vector<std::string> a = {"a", "b", "c", "d", "e"};
    std::vector<std::string> b = {"a", "X", "c", "Y", "e"};
    auto result = computeDiff(a, b);

    assert(result.hunkCount == 2);
    assert(result.hunkStarts.size() == 2);
}

int main() {
    test_arrow_keys();
    test_home_end_variants();
    test_page_and_delete();
    test_unrecognized();
    test_chars_to_render();
    test_render_to_chars();
    test_no_tabs();
    test_multiple_tabs();
    test_insert_char();
    test_delete_char();
    test_split_line();
    test_join_lines();
    test_dirty_flag();
    test_action_types();
    test_filetype_detection();
    test_word_count_basic();
    test_word_count_multiple();
    test_word_count_edge_cases();

    test_undo_insert();
    test_undo_delete();
    test_undo_split();
    test_undo_join();
    test_highlight_numbers();
    test_highlight_string();
    test_highlight_comment();
    test_highlight_multiline_comment();
    test_highlight_invalidation();
    test_replace_shorter();
    test_replace_longer();
    test_replace_same_length();
    test_diff_identical();
    test_diff_completely_different();
    test_diff_addition();
    test_diff_removal();
    test_diff_empty_left();
    test_diff_empty_right();
    test_diff_both_empty();
    test_diff_hunks();

    std::cout << "All tests passed.\n";
    return 0;
}
