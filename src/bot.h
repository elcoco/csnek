#ifndef BOT_H
#define BOT_H

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "astar.h"

struct Bot {
    uint32_t xsize;
    uint32_t ysize;

    // callbacks for drawing results
    void(*draw_open_cb)(Pos x, Pos y);
    void(*draw_closed_cb)(Pos x, Pos y);
    void(*draw_path_cb)(Pos x, Pos y);
    void(*draw_wall_cb)(Pos x, Pos y);
    void(*draw_refresh_cb)();
};

void bot_init(struct Bot* bot, uint32_t xsize, uint32_t ysize);
void bot_run(struct Bot* bot);

#endif
