#ifndef SLIB
#define SLIB

#include <stdint.h>
#include <strings.h>  // ffs

#define MASK_HOUSE 0b111111111
#define FLAG_COLLAPSED 1
#define FLAG_GIVEN 2

// cell -> house conversions
//  .0: row
//  .1: column
//  .2: house
extern const uint8_t hcord[81][3];

// box -> cells indices
extern const uint8_t boxi[9][9];

typedef struct grid {
    // collapsed cells are stored in range [0, 8], shifting the "normal" range
    // [1, 9] down by one, to simplify bit adressing and shifting in bitfields
    uint8_t collapsed[81];

    // cells superpositions
    //     1-9: on if n is a candidate
    //   10-16: not used
    uint16_t cells[81];

    // various flags
    //   1: collapsed
    //   2: given
    uint8_t flags[81];
} grid_t;

// create an empty grid
grid_t new();

// load a grid from a string in standard form, where any charachter not in range
// [1-9] is treated as a empty cell
grid_t load(const char src[81]);

// collapse a cell to a value (no checks performed)
// and reduces the affected houses
void collapse(grid_t *const g, const int cell, const int value);

// http://sudopedia.enjoysudoku.com/Naked_Single.html
// returns the amount of naked singles collapsed
int naked_single_c(grid_t *const g, const int cell);
int naked_single(grid_t *const g);

// http://sudopedia.enjoysudoku.com/Hidden_Single.html
// returns the amount of hidden singles collapsed
int hidden_single(grid_t *const g);

// apply solving techniques until solved (or failed to)
// implemented st:
//   - [x] naked singles
void solve(grid_t *const g);

#endif  // SLIB

#define SLIB_IMPLEMENTATION
#ifdef SLIB_IMPLEMENTATION

// for cell in range(0, 81):
//     row = cell // 9
//     column = cell % 9
//     box = (row // 3 * 3) + (column // 3)
//     print(f"{{{row}, {column}, {(box)}}},")
const uint8_t hcord[81][3] = {
    {0, 0, 0}, {0, 1, 0}, {0, 2, 0}, {0, 3, 1}, {0, 4, 1}, {0, 5, 1}, {0, 6, 2},
    {0, 7, 2}, {0, 8, 2}, {1, 0, 0}, {1, 1, 0}, {1, 2, 0}, {1, 3, 1}, {1, 4, 1},
    {1, 5, 1}, {1, 6, 2}, {1, 7, 2}, {1, 8, 2}, {2, 0, 0}, {2, 1, 0}, {2, 2, 0},
    {2, 3, 1}, {2, 4, 1}, {2, 5, 1}, {2, 6, 2}, {2, 7, 2}, {2, 8, 2}, {3, 0, 3},
    {3, 1, 3}, {3, 2, 3}, {3, 3, 4}, {3, 4, 4}, {3, 5, 4}, {3, 6, 5}, {3, 7, 5},
    {3, 8, 5}, {4, 0, 3}, {4, 1, 3}, {4, 2, 3}, {4, 3, 4}, {4, 4, 4}, {4, 5, 4},
    {4, 6, 5}, {4, 7, 5}, {4, 8, 5}, {5, 0, 3}, {5, 1, 3}, {5, 2, 3}, {5, 3, 4},
    {5, 4, 4}, {5, 5, 4}, {5, 6, 5}, {5, 7, 5}, {5, 8, 5}, {6, 0, 6}, {6, 1, 6},
    {6, 2, 6}, {6, 3, 7}, {6, 4, 7}, {6, 5, 7}, {6, 6, 8}, {6, 7, 8}, {6, 8, 8},
    {7, 0, 6}, {7, 1, 6}, {7, 2, 6}, {7, 3, 7}, {7, 4, 7}, {7, 5, 7}, {7, 6, 8},
    {7, 7, 8}, {7, 8, 8}, {8, 0, 6}, {8, 1, 6}, {8, 2, 6}, {8, 3, 7}, {8, 4, 7},
    {8, 5, 7}, {8, 6, 8}, {8, 7, 8}, {8, 8, 8},
};

// for box in range(0, 9):
//     first = box // 3 * 27 + box % 3 * 3
//     shifts = (0, 1, 2, 9, 10, 11, 18,  19, 20)
//     cells = (shift + first for shift in shifts)
//     print(f"{{{', '.join(map(str, cells))}}},")
const uint8_t boxi[9][9] = {
    {0, 1, 2, 9, 10, 11, 18, 19, 20},     {3, 4, 5, 12, 13, 14, 21, 22, 23},
    {6, 7, 8, 15, 16, 17, 24, 25, 26},    {27, 28, 29, 36, 37, 38, 45, 46, 47},
    {30, 31, 32, 39, 40, 41, 48, 49, 50}, {33, 34, 35, 42, 43, 44, 51, 52, 53},
    {54, 55, 56, 63, 64, 65, 72, 73, 74}, {57, 58, 59, 66, 67, 68, 75, 76, 77},
    {60, 61, 62, 69, 70, 71, 78, 79, 80},
};

grid_t new() {
    // i think this uses a gcc extension ([0 ... 80])
    return (grid_t){{0}, {[0 ... 80] = MASK_HOUSE}};
};

grid_t load(const char src[81]) {
    grid_t g = new ();

    for (int i = 0, v = src[0]; i < 81; i++, v = src[i])
        if ('1' <= v && v <= '9') {
            collapse(&g, i, v - '1');
            g.flags[i] |= FLAG_GIVEN;
        }

    return g;
}

void collapse(grid_t *const g, const int cell, const int value) {
    g->collapsed[cell] = value;
    g->flags[cell] |= FLAG_COLLAPSED;

    // i could check into xor, but this is probably faster (and non-branching)
    int mask = MASK_HOUSE ^ 1 << value;

    // collapse affected houses
    int row = hcord[cell][0] * 9, col = hcord[cell][1], box = hcord[cell][2];
    for (int i = 0; i < 9; i++, row++, col += 9) {
        if (row != cell) g->cells[row] &= mask;                    // collapse row
        if (col != cell) g->cells[col] &= mask;                    // collapse column
        if (boxi[box][i] != cell) g->cells[boxi[box][i]] &= mask;  // collapse box
    }
}

int naked_single_c(grid_t *const g, const int cell) {
    if (!(g->flags[cell] & FLAG_COLLAPSED) && __builtin_popcount(g->cells[cell]) == 1) {
        collapse(g, cell, ffs(g->cells[cell]) - 1);
        return 1;
    }

    return 0;
}

int naked_single(grid_t *const g) {
    int k = 0;
    for (int c = 0; c < 81; c++) k += naked_single_c(g, c);
    return k;
}

// TODO: bench and optimize kek
int hidden_single(grid_t *const g) {
    int digit_maps[9] = {0}, k = 0, cell;

    // scan rows
    for (int row = 0; row < 81; row += 9) {
        // reset dm
        for (int i = 0; i < 9; i++) digit_maps[i] = 0;
        cell = row;

        // update dm
        for (int i = 0; i < 9; i++, cell = row + i)
            if (!(g->flags[cell] & FLAG_COLLAPSED))
                for (int v = 0; v < 9; v++)
                    if (g->cells[cell] & 1 << v) digit_maps[v] |= 1 << i;

        // check dm
        for (int v = 0; v < 9; v++)
            if (__builtin_popcount(digit_maps[v]) == 1) {
                collapse(g, row + ffs(digit_maps[v]) - 1, v);
                k++;
            }
    }

    // scan columns
    for (int col = 0; col < 9; col += 1) {
        // reset dm
        for (int i = 0; i < 9; i++) digit_maps[i] = 0;
        cell = col;

        // update dm
        for (int i = 0; i < 9; i++, cell += 9)
            if (g->flags[cell] & FLAG_COLLAPSED) digit_maps[g->collapsed[cell]] = 0;
            else
                for (int v = 0; v < 9; v++)
                    if (g->cells[cell] & 1 << v) digit_maps[v] |= 1 << i;

        // check dm
        for (int v = 0; v < 9; v++)
            if (__builtin_popcount(digit_maps[v]) == 1) {
                collapse(g, col + 9 * (ffs(digit_maps[v]) - 1), v);
                k++;
            }
    }

    // scan boxes
    for (int box = 0; box < 9; box += 1) {
        // reset dm
        for (int i = 0; i < 9; i++) digit_maps[i] = 0;

        // update dm
        for (int i = 0; i < 9; i++) {
            cell = boxi[box][i];

            if (g->flags[cell] & FLAG_COLLAPSED) digit_maps[g->collapsed[cell]] = 0;
            else
                for (int v = 0; v < 9; v++)
                    if (g->cells[cell] & 1 << v) digit_maps[v] |= 1 << i;
        }

        // check dm
        for (int v = 0; v < 9; v++) {
            if (__builtin_popcount(digit_maps[v]) == 1) {
                collapse(g, boxi[box][ffs(digit_maps[v]) - 1], v);
                k++;
            }
        }
    }

    return k;
}

void solve(grid_t *const g) {
    int collapsed = 0, ns, hs;

    do {
        // printf("\n");
        collapsed = 0;
        
        ns = naked_single(g);
        hs = hidden_single(g);

        // printf("ns: %d\nhs: %d\n", ns, hs);
        collapsed += ns + hs;
    } while (collapsed != 0);
}

#endif  // SLIB_IMPLEMENTATION