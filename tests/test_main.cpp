#include "terminal.hpp"
#include "buffer.hpp"
#include <cassert>
#include <iostream>

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

int main() {
    test_arrow_keys();
    test_home_end_variants();
    test_page_and_delete();
    test_unrecognized();
    test_chars_to_render();
    test_render_to_chars();
    test_no_tabs();
    test_multiple_tabs();

    std::cout << "All tests passed.\n";
    return 0;
}
