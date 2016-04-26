#include "common.h"

void load_options() {
	FILE config;
}

void init_system() {}
int menu_handler() {
    render_textbufs();
    return 1;
}

void main() {
    if (fmount()) {
        // Failed to mount SD. Bomb out.
    }

    cprintf(TOP_SCREEN, "Hello, %d, %s!\n", 5042, "person");

    load_options(); // Load configuration.

    int in_menu = 1;

    while(in_menu) {
        in_menu = menu_handler();
    }

    init_system();
    // Under ideal conditions, we never get here.
}
