#include "bot.h"

void bot_init(struct Bot* bot, struct Game* game, struct State* state, uint32_t xsize, uint32_t ysize)
{
    bot->xsize = xsize;
    bot->ysize = ysize;

    bot->game = game;
    bot->state = state;
}

enum Direction pos_to_dir(Pos x0, Pos y0, Pos x1, Pos y1)
{
    if (x0 == x1 && y0 > y1)
        return DIR_N;
    else if (x0 == x1 && y0 < y1)
        return DIR_S;
    else if (x0 < x1 && y0 == y1)
        return DIR_E;
    else if (x0 > x1 && y0 == y1)
        return DIR_W;
    else
        return DIR_NONE;
}

static void draw_bar(WINDOW* win, char* str)
{
    add_str(win, 0, 0, CGREEN, CDEFAULT, "%s", str);
}

uint32_t count_reachable(struct Astar* astar, struct Set* closedset, struct Node* n_cur)
{
    /* Recursive count of reachable nodes in grid starting from node n */
    uint32_t amount = 0;

    if (set_node_exists(closedset, n_cur))
        return amount;

    set_add_node(closedset, n_cur);

    if (n_cur->is_wall)
        return amount;

    amount++;
    astar->draw_open_cb(n_cur->x, n_cur->y);

    // check all neighboring nodes
    int8_t xy[4][2] = {{0,-1}, {1,0}, {0,1}, {-1,0}};

    for (int i=0 ; i<4 ; i++) {

        Pos x = n_cur->x + xy[i][0];
        Pos y = n_cur->y + xy[i][1];

        if (!is_in_grid(x, y, astar->xsize, astar->ysize))
            continue;

        struct Node* n_neighbor = get_node(astar->grid, x, y, astar->xsize);

        amount += count_reachable(astar, closedset, n_neighbor);
    }

    return amount;
}

float get_reachable(struct Astar* astar, struct Node* n_cur)
{
    /* Find percentage of reachable nodes starting from n_cur */
    // follow back found path and set all nodes to wall
    struct Node* n_tmp = n_cur->parent;
    while (n_tmp->g != 0) {
        n_tmp->is_wall = true;
        n_tmp = n_tmp->parent;
    }

    // count all nodes marked as wall
    int n_wall = 0;
    for (int i=0 ; i<astar->xsize*astar->ysize ; i++) {
        if (astar->grid[i].is_wall)
            n_wall++;
    }

    // calculate unoccupied nodes (not wall)
    int unoccupied = astar->xsize*astar->ysize - n_wall;

    // create a set to store closed nodes in
    struct Node* closed_arr[astar->xsize*astar->ysize];
    struct Set set;
    set.set = closed_arr;
    set.len = 0;

    // calculate reachable nodes
    int amount = count_reachable(astar, &set, n_cur);
    debug("%d amount: %d   %.1f%%\n", unoccupied, amount, ((float)amount/unoccupied)*100);

    return ((float)amount/unoccupied)*100;
}

enum GameState exec_path(struct Game* game, struct Node* n_end, WINDOW* win, float speed_ms)
{
    /* Execute found path in snake game */
    enum GameState gs;

    for (int gi=1 ; gi<=n_end->g ; gi++) {

        struct Node* n = n_end;

        while (n->g != gi)
            n = n->parent;

        // apply move
        gs = game_next(game, pos_to_dir(n->parent->x, n->parent->y, n->x, n->y));

        werase(win);
        game_draw(game);
        wrefresh(win);
        usleep(speed_ms*1000);
    }
    return gs;
}

float get_perc_used(struct Astar* astar)
{
    /* Calculate percentage of occupied nodes (that are marked as wall) */
    int n_wall = 0;
    for (int gi=0 ; gi<astar->xsize*astar->ysize ; gi++) {
        if (astar->grid[gi].is_wall)
            n_wall++;
    }
    return (float)n_wall/(astar->xsize*astar->ysize)*100;
}

void bot_run(struct Bot* bot, WINDOW* field_win, WINDOW* bar_win)
{
    // FIXME All references to curses should be removed from this module,
    //       Drawing should only happen by using callbacks
    //
    for (int i=0 ; ; i++) {
        struct Astar astar;
        struct Node grid[bot->xsize*bot->ysize];
        struct Node* openset[bot->xsize*bot->ysize];
        struct Node* closedset[bot->xsize*bot->ysize];

        // NOTE shead/stail refers to the head/tail of linked list, not snake's head/tail
        struct Seg* start = *bot->game->snake.stail;
        Pos xstart = start->xpos;
        Pos ystart = start->ypos;
        Pos xend, yend;

        enum ASPathType ptype;

        // Use longest route as snake grows
        // Start by using food as destination point
        // Later Use tail as destination so snek won't lock himself up
        if (bot->game->snake.len < 50) {
            struct FoodItem* fend = *bot->game->food.fhead;
            xend = fend->xpos;
            yend = fend->ypos;
            ptype = AS_SHORTEST;
        }
        else {
            struct Seg* send = *bot->game->snake.shead;
            xend = send->xpos;
            yend = send->ypos;
            ptype = AS_LONGEST;
        }

        astar_init(&astar, grid, openset, closedset, bot->xsize, bot->ysize);
        astar_set_points(&astar, xstart, ystart, xend, yend);

        // disable drawing by uncommenting
        //astar.draw_open_cb    = bot->draw_open_cb;
        //astar.draw_closed_cb  = bot->draw_closed_cb;
        //astar.draw_path_cb    = bot->draw_path_cb;
        //astar.draw_wall_cb    = bot->draw_wall_cb;
        //astar.draw_refresh_cb = bot->draw_refresh_cb;

        // Set snake body as wall in astar
        // Don't mark tail as wall or we will not be able to use it as a destination
        struct Seg* seg = (*bot->game->snake.shead)->next;
        while (seg != NULL) {
            struct Node* n = get_node(astar.grid, seg->xpos, seg->ypos, bot->xsize);
            n->is_wall = true;
            seg = seg->next;
        }

        // solve path using algorithm
        if (astar_find_path(&astar, ptype) == AS_UNSOLVED) {
            show_msg("ASTAR UNSOLVABLE");
            return;
        }

        struct Node* n_end = get_node(astar.grid, xend, yend, bot->xsize);
        exec_path(bot->game, n_end, field_win, bot->state->speed_ms);

        float perc_occ = get_perc_used(&astar);

        char buf[256] = "";
        sprintf(buf, "i: %d  snek_len: %d  score: %d, os_len: %d, cs_len: %d, occ: %.2f%%", i, bot->game->snake.len, bot->game->score, astar.openset.len, astar.closedset.len, perc_occ);
        werase(bar_win);
        draw_bar(bar_win, buf);
        wrefresh(bar_win);
        bot->draw_refresh_cb();
    }
}
