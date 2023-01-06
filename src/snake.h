#ifndef SNAKE_H
#define SNAKE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "utils.h"

#define SNAKE_DEFAULT_MAX_FOOD 1
#define SNAKE_DEFAULT_GROW_FACTOR 1
#define SNAKE_DEBUG_FOOD_CHR 'x'
#define SNAKE_DEBUG_SEG_CHR 'o'

// position type, represents coordinate (x or y)
typedef uint16_t Pos;

// Represents direction of movement.
// Changes after user input
enum Direction {
    DIR_N,
    DIR_E,
    DIR_S,
    DIR_W,
    DIR_NONE
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
    // After eating, the snake may need to grow (or shrink)
    // to become equal to len
    uint16_t len;
    uint16_t cur_len;

    // access to snake linked list
    struct Seg** shead;
    struct Seg** stail;

    void(*draw_cb)(Pos x, Pos y);
};

// The stuf that our snake eats
struct FoodItem;
struct FoodItem {
    Pos xpos;
    Pos ypos;

    struct FoodItem* prev;
    struct FoodItem* next;
};

struct Food {
    // access to food items as a linked list
    struct FoodItem** fhead;
    struct FoodItem** ftail;

    void(*draw_cb)(Pos x, Pos y);
};

struct Game {
    // field dimensions
    uint32_t xsize;
    uint32_t ysize;

    // amount of food items created
    uint16_t score;

    // grow factor or the amount the snake will grow after eating it
    uint8_t grow_fac;

    // max amount of food items in field
    uint16_t maxfood;

    struct Snake snake;
    struct Food food;
};

// public functions
void game_init(struct Game* game, uint32_t xsize, uint32_t ysize, uint16_t maxfood);
enum GameState game_next(struct Game* game, enum Direction v);
void game_draw(struct Game* game);


// private functions
void snake_init(struct Snake*, Pos xstart, Pos ystart);
void snake_lremove(struct Seg** shead, uint16_t amount);

struct Seg* seg_init(struct Seg** stail, Pos xpos, Pos ypos);
struct Seg* seg_detect_col(struct Seg* stail, Pos x, Pos y, uint16_t roffset);

void food_init(struct Food* food, struct Seg* stail, uint16_t xsize, uint16_t ysize, uint16_t maxfood);
struct FoodItem* food_detect_col(struct FoodItem* ftail, Pos x, Pos y);

struct FoodItem* fooditem_init(struct FoodItem** ftail, struct Seg* stail, uint16_t xsize, uint16_t ysize);
void fooditem_destroy(struct FoodItem* f, struct FoodItem** fhead, struct FoodItem** ftail);

#endif
