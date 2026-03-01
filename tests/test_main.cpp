#include "terminal.hpp"
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

int main() {
    test_arrow_keys();
    test_home_end_variants();
    test_page_and_delete();
    test_unrecognized();

    std::cout << "All tests passed.\n";
    return 0;
}
