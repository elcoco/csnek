#ifndef SNAKE_H
#define SNAKE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define SNAKE_GROW_FACTOR 1
#define SNAKE_FOOD_CHR 'F'
#define SNAKE_SEG_CHR 'X'

// position type, represents coordinate (x or y)
typedef uint16_t Pos;

// Represents direction of movement.
// Changes after user input
enum Velocity {
    VEL_N,
    VEL_E,
    VEL_S,
    VEL_W
};


// Represents segment of snake linked as linked list
struct Seg;
struct Seg {
    struct Seg* prev;
    struct Seg* next;

    Pos xpos;
    Pos ypos;
};

struct Snake {
    // The length of the snake after eating something
    // This does not necessarily reflect current length
    // so snake may need to grow (or shrink) to become equal
    // to len
    uint16_t len;
    uint16_t cur_len;

    // access to snake linked list
    struct Seg** head;
    struct Seg** tail;
};

// The stuf that our snake eats
struct Food;
struct Food {
    // grow factor of food or the amount the snake will grow after eating it
    uint8_t grow_fac;

    Pos xpos;
    Pos ypos;

    struct Food* prev;
    struct Food* next;
};

struct Matrix {
    uint32_t xsize;
    uint32_t ysize;

    // access to food items as a linked list
    struct Food** fhead;
    struct Food** ftail;
};

struct Seg* seg_init(struct Seg** tail, Pos xpos, Pos ypos);
struct Seg* seg_detect_col(struct Seg* tail, Pos x, Pos y);

void snake_init(struct Snake*);
void snake_debug(struct Snake* s);
void snake_lremove(struct Seg** head, uint16_t amount);

void matrix_init(struct Matrix* m, uint32_t xsize, uint32_t ysize, uint16_t nfood);
void matrix_debug(struct Matrix* m, struct Snake* s);
int8_t matrix_next(struct Matrix* m, struct Snake* s, enum Velocity v);

struct Food* food_init(struct Food** tail, uint16_t xsize, uint16_t ysize);
struct Food* food_detect_col(struct Food* tail, Pos x, Pos y);

#endif
