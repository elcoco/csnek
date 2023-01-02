#ifndef SNAKE_H
#define SNAKE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "utils.h"

#define SNAKE_DEFAULT_GROW_FACTOR 1
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

// Is returned from field_next when calling for next frame calculation
enum GameState {
    GAME_WON,
    GAME_LOST,
    GAME_NONE
};

// Represents segment of snake linked as linked list
struct Seg;
struct Seg {
    Pos xpos;
    Pos ypos;
    struct Seg* prev;
    struct Seg* next;
};

struct Snake {
    // The length of the snake after eating something
    // This does not necessarily reflect current length
    // so snake may need to grow (or shrink) to become equal
    // to len
    uint16_t len;
    uint16_t cur_len;

    // grow factor of food or the amount the snake will grow after eating it
    uint8_t grow_fac;

    // access to snake linked list
    struct Seg** shead;
    struct Seg** stail;

    void(*draw_cb)(Pos x, Pos y);
};

// The stuf that our snake eats
struct Food;
struct Food {

    Pos xpos;
    Pos ypos;

    struct Food* prev;
    struct Food* next;
};

struct Field {
    uint32_t xsize;
    uint32_t ysize;

    // max amount of food items in field
    uint16_t max_food;

    // amount of food items created
    uint16_t score;

    // access to food items as a linked list
    struct Food** fhead;
    struct Food** ftail;

    void(*draw_cb)(Pos x, Pos y);
};

struct Seg* seg_init(struct Seg** stail, Pos xpos, Pos ypos);
struct Seg* seg_detect_col(struct Seg* stail, Pos x, Pos y, uint16_t roffset);

void snake_init(struct Snake*, Pos xstart, Pos ystart);
void snake_debug(struct Snake* s);
void snake_lremove(struct Seg** shead, uint16_t amount);

void field_init(struct Field* field, struct Snake* s, uint32_t xsize, uint32_t ysize, uint16_t max_food);
void field_debug(struct Field* field, struct Snake* s);
enum GameState field_next(struct Field* field, struct Snake* s, enum Velocity v);
void field_draw(struct Field* field, struct Snake* s);

struct Food* food_init(struct Food** ftail, struct Seg* stail, uint16_t xsize, uint16_t ysize);
struct Food* food_detect_col(struct Food* tail, Pos x, Pos y);
void food_destroy(struct Field* field, struct Food* f);
void food_debug(struct Field* field);

#endif
