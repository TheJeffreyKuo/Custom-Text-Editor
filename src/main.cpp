#include "editor.hpp"
#include <cstdlib>
#include <string>

int main(int argc, char* argv[]) {
    std::string filename;
    if (argc >= 2) filename = argv[1];

    Editor editor(filename);
    editor.run();

    return EXIT_SUCCESS;
}
