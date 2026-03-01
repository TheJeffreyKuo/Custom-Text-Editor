#include "terminal.hpp"
#include <cstdlib>
#include <unistd.h>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    Terminal terminal;

    while (true) {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == 1 && c == 'q') break;
    }

    return EXIT_SUCCESS;
}
