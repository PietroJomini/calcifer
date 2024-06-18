/*
 * sudoku display lib
 */

#ifndef SLIB_D
#define SLIB_D

#include "stdio.h"

#ifdef SLIB_IMPLEMENTATION
#undef SLIB_IMPLEMENTATION
#include "s.h"
#define SLIB_IMPLEMENTATION
#else
#include "s.h"
#endif

// print debug infos on a grid
void debug(const grid_t g);

#endif  // SLIB_D

#define SLIB_IMPLEMENTATION
#ifdef SLIB_IMPLEMENTATION
// NOLINTBEGIN(misc-definitions-in-headers)

void debug(const grid_t g) {
    // flat collapsed
    printf("COLLAPSED: ");
    for (int c = 0; c < 81; c++)
        printf("%c", g.flags[c] & FLAG_COLLAPSED ? '1' + g.collapsed[c] : '.');
    printf("\n");

    // givens
    printf("    GIVEN: ");
    for (int c = 0; c < 81; c++) printf("%c", g.flags[c] & FLAG_GIVEN ? '+' : ' ');
    printf("\n\n");

    // collapsed grid + candidate grid
    for (int row = 0; row < 9; row++) {
        // grid
        for (int col = 0; col < 9; col++) {
            printf("%c ", g.flags[row * 9 + col] & FLAG_COLLAPSED
                              ? '1' + g.collapsed[row * 9 + col]
                              : '.');

            //   seps
            if (col == 2 || col == 5) printf("| ");
        }

        // candidate
        printf("   ");
        for (int col = 0; col < 9; col++) {
            int n = 9;
            // candidate values
            if (g.flags[row * 9 + col] & FLAG_COLLAPSED) {
                char indicator = g.flags[row * 9 + col] & FLAG_GIVEN ? '+' : '.';
                printf("%d%c", g.collapsed[row * 9 + col] + 1, indicator);
                n -= 2;
            } else
                for (int v = 0; v < 9; v++)
                    if (g.cells[row * 9 + col] & 1 << v) {
                        printf("%d", v + 1);
                        n--;
                    }

            // alignment + seps
            for (int i = 0; i < n; i++) printf(" ");
            if (col == 2 || col == 5) printf(" | ");
            else printf(" ");
        }

        //  seps
        printf("\n");
        if (row == 2 || row == 5) {
            printf("------+-------+------    ");          // grid
            printf("------------------------------+");  // candidate col 1-3
            printf("-------------------------------+");  // candidate col 4-6
            printf("------------------------------\n");   // candidate col 7-9
        }
    }
}

// NOLINTEND(misc-definitions-in-headers)
#endif  // SLIB_IMPLEMENTATION