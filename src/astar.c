#include "astar.h"

uint32_t pos2i(Pos x, Pos y, uint32_t xsize)
{
    /* Translate coordinates into array indices */
    return y*xsize + x;
}

void i2pos(uint32_t i, Pos* x, Pos* y, uint32_t xsize)
{
    /* Translate indice into coordinates */
    *x = i % xsize;
    *y = i / xsize;
}

struct Node* get_node(struct Node* grid, Pos x, Pos y)
{
    return &grid[pos2i(x, y, XSIZE)];
}

void astar_init(struct Astar* astar, Pos x0, Pos y0, Pos x1, Pos y1)
{
    astar->xsize = XSIZE;
    astar->ysize = YSIZE;

    astar->x0 = x0;
    astar->y0 = y0;
    astar->x1 = x1;
    astar->y1 = y1;

    struct Node* n = astar->grid;

    for (int i=0 ; i<XSIZE*YSIZE ; i++, n++) {
        n->f = 0;
        n->g = 0;

        i2pos(i, &n->x, &n->y, XSIZE);

        n->h = abs(x1 - n->x) + abs(y1 - n->y);
    }

    astar->openset.len = 0;
    astar->closedset.len = 0;
}

void astar_debug(struct Astar* astar)
{
    struct Node* n = astar->grid;

    for (int i=0 ; i<XSIZE*YSIZE ; i++, n++) {
        debug("[%d] %d x %d\n", i, n->x, n->y);
    }

}

void astar_draw(struct Astar* astar)
{
    struct Node** n = astar->openset.set;
    for (int i=0 ; i<astar->openset.len ; i++, n++)
        astar->draw_open_cb((*n)->x, (*n)->y);

    n = astar->closedset.set;
    for (int i=0 ; i<astar->closedset.len ; i++, n++)
        astar->draw_closed_cb((*n)->x, (*n)->y);

    n = astar->path.set;
    for (int i=0 ; i<astar->path.len ; i++, n++)
        astar->draw_path_cb((*n)->x, (*n)->y);
}

struct Node* astar_find_lowest_f(struct Set* set)
{
    /* Go through set and find node with lowes fscore */
    struct Node* winner = *set->set;
    struct Node** n = set->set;

    for (int i=0 ; i<set->len ; i++, n++) {
        if ((*n)->f < winner->f)
            winner = *n;
    }
    return winner;
}

void set_remove_node(struct Set* set, struct Node* n)
{
    /* Remove node by shifting array to left */
    int node_i;

    // first find node index
    for (node_i=0 ; node_i<set->len ; node_i++) {
        if (set->set[node_i] == n)
            break;
    }
    for (int i=node_i ; i<set->len ; i++) {
        set->set[i] = set->set[i+1];
    }

    set->len--;
}

bool set_node_exists(struct Set* set, struct Node* n)
{
    for (int i=0 ; i<set->len ; i++) {
        //if (set->set[i] == n)
        //debug("%dx%d %dx%d\n", set->set[i]->x, set->set[i]->y, n->x, n->y); 
        if (set->set[i]->x == n->x && set->set[i]->y == n->y) {
            return true;
        }
    }
    return false;
}

void set_add_node(struct Set* set, struct Node* n)
{
    if (! set_node_exists(set, n)) {
        set->set[set->len] = n;
        set->len++;
    }
}

void set_debug(struct Set* set, char* prefix)
{
    for (int i=0 ; i<set->len ; i++) {
        struct Node* n = set->set[i];
        debug("[%s][%d] %dx%d f=%d g=%d, h=%d\n", prefix, i, n->x, n->y, n->f, n->g, n->h); 
    }

}

bool is_in_grid(Pos x, Pos y, uint16_t xsize, uint16_t ysize)
{
    return (x >= 0 && y >= 0 && x < xsize && y < ysize);
}

void add_to_openset(struct Astar* astar, struct Node* prev, Pos x, Pos y) {
    /* move node to openset if it doesn't exist in closedset */

    if (is_in_grid(x, y, astar->xsize, astar->ysize)) {
        struct Node* n = get_node(astar->grid, x, y);

        if (!set_node_exists(&astar->closedset, n)) {

            uint16_t tmpg = prev->g + 1;

            if (set_node_exists(&astar->openset, n)) {

                // check if we already have a more efficient route
                if (tmpg < n->g)
                    n->g = tmpg;
                else
                    n->g = prev->g + 1;
            }
            n->f = n->g + n->h;
            set_add_node(&astar->openset, n);
        }
    }
}

void astar_find_path(struct Astar* astar)
{
    struct Node* n_start = get_node(astar->grid, astar->x0, astar->y0);
    struct Node* n_end = get_node(astar->grid, astar->x1, astar->y1);

    struct Set* openset = &astar->openset;
    struct Set* closedset = &astar->closedset;
    struct Set* path = &astar->path;

    // start with start node
    set_add_node(openset, n_start);

    while (openset->len > 0) {
        struct Node* n_cur = astar_find_lowest_f(openset);
        debug("current: %d x %d\n", n_cur->x, n_cur->y);

        if (n_cur->x == n_end->x && n_cur->y == n_end->y) {
            debug("Reached end node\n");
            break;
        }

        set_add_node(closedset, n_cur);
        debug("[close] add: %d x %d, cur set len: %d\n", n_cur->x, n_cur->y, closedset->len);
        set_remove_node(openset, n_cur);

        //set_add_node(path, n_cur);


        // add neighbours of current node to openset
        // only if they do not eist in closedset
        add_to_openset(astar, n_cur, n_cur->x, n_cur->y-1);
        add_to_openset(astar, n_cur, n_cur->x+1, n_cur->y);
        add_to_openset(astar, n_cur, n_cur->x, n_cur->y+1);
        add_to_openset(astar, n_cur, n_cur->x-1, n_cur->y);

        set_debug(closedset, "closed");
        set_debug(openset, "open");


        astar_draw(astar);
        usleep(0.3 * 1000 * 1000);

    }

    debug("DONE!\n");
}
