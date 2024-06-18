#include <stdint.h>
#include <stdio.h>

#define SLIB_IMPLEMENTATION
#include "d.h"
// #include "s.h"

#define P0 \
    "5.....37....6..............7..54.....4......2...1..6...6..83........2.4...1......"

#define P1 \
    "..43..2.9..5..9..1.7..6..43..6..2.8719...74...5..83...6.....1.5..35.869..4291.3.."

#define P2 \
    "......3.186..........2.........4.76...1.............8..7....64.5..1.3......5....."

int main(int argc, char** argv) {
    grid_t g = load(P2);

    solve(&g);
    debug(g);

    return 0;
}