#include "diff.hpp"
#include "diffviewer.hpp"
#include "editor.hpp"
#include <cstdlib>
#include <string>

int main(int argc, char* argv[]) {
    if (argc >= 4 && std::string(argv[1]) == "--diff") {
        Terminal terminal;
        auto left = readFileLines(argv[2]);
        auto right = readFileLines(argv[3]);
        DiffResult diff = computeDiff(left, right);
        DiffViewer viewer(terminal, diff, argv[2], argv[3]);
        viewer.run();
        return EXIT_SUCCESS;
    }

    std::string filename;
    if (argc >= 2) filename = argv[1];

    Editor editor(filename);
    editor.run();

    return EXIT_SUCCESS;
}
