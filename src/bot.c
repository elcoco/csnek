#include "bot.h"

void bot_init(struct Bot* bot, uint32_t xsize, uint32_t ysize)
{
    bot->xsize = xsize;
    bot->ysize = ysize;
}

void bot_run(struct Bot* bot)
{
    while (1) {
        struct Astar astar;

        // we can't use malloc so we need to init everything here
        struct Node grid[bot->xsize*bot->ysize];
        struct Node* openset[bot->xsize*bot->ysize];
        struct Node* closedset[bot->xsize*bot->ysize];

        astar_init(&astar, grid, openset, closedset, bot->xsize, bot->ysize);

        // disable by uncommenting
        astar.draw_open_cb    = bot->draw_open_cb;
        astar.draw_closed_cb  = bot->draw_closed_cb;
        astar.draw_path_cb    = bot->draw_path_cb;
        //astar.draw_wall_cb    = bot->draw_wall_cb;
        astar.draw_refresh_cb = bot->draw_refresh_cb;

        astar_set_points(&astar, 0, 0, bot->xsize-1, bot->ysize-1);

        enum ASResult res = astar_find_path(&astar);

        if (res == AS_UNSOLVED)
            show_msg("UNSOLVABLE");
        else if (res == AS_SOLVED)
            show_msg("SOLVED");
        else
            show_msg("UNKNOWN");

    }
}
