#include <cassert>
#include <iostream>

void test_placeholder() {
    assert(1 + 1 == 2);
}

int main() {
    test_placeholder();
    std::cout << "All tests passed.\n";
    return 0;
}
