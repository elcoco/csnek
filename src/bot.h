#ifndef BOT_H
#define BOT_H

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "astar.h"
#include "snake.h"

struct Bot {
    uint32_t xsize;
    uint32_t ysize;

    struct Game* game;

    // callbacks for drawing results
    void(*draw_open_cb)(Pos x, Pos y);
    void(*draw_closed_cb)(Pos x, Pos y);
    void(*draw_path_cb)(Pos x, Pos y);
    void(*draw_wall_cb)(Pos x, Pos y);
    void(*draw_refresh_cb)();
};

void bot_init(struct Bot* bot, struct Game* game, uint32_t xsize, uint32_t ysize);
void bot_run(struct Bot* bot, WINDOW* win);

#endif
