#include "terminal.hpp"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    Terminal terminal;

    while (true) {
        int key = terminal.readKey();

        if (key == 'q') break;

        if (key >= 1000) {
            write(STDOUT_FILENO, "[special]\r\n", 10);
        } else if (key < 32) {
            char buf[32];
            int n = snprintf(buf, sizeof(buf), "Ctrl-%c\r\n", key + '@');
            write(STDOUT_FILENO, buf, n);
        } else {
            char buf[32];
            int n = snprintf(buf, sizeof(buf), "%c (%d)\r\n", key, key);
            write(STDOUT_FILENO, buf, n);
        }
    }

    return EXIT_SUCCESS;
}
