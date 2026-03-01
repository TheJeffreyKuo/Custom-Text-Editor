#include "terminal.hpp"
#include "buffer.hpp"
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

    std::cout << "All tests passed.\n";
    return 0;
}
