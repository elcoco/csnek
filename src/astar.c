#include "astar.h"

uint16_t get_rnd(uint16_t lower, uint16_t upper)
{
    return (rand() % (upper - lower + 1)) + lower;
}

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

struct Node* get_node(struct Node* grid, Pos x, Pos y, uint16_t xsize)
{
    return &grid[pos2i(x, y, xsize)];
}

void path_trace_back(struct Astar* astar, struct Node* n_end)
{
    struct Node* n = n_end;
    while (n != NULL) {
        astar->draw_path_cb(n->x, n->y);
        n = n->parent;
    }
}

void astar_set_points(struct Astar* astar, Pos x0, Pos y0, Pos x1, Pos y1)
{
    // set random seed
    srand(time(NULL));

    astar->x0 = x0;
    astar->y0 = y0;
    astar->x1 = x1;
    astar->y1 = y1;

    struct Node* n = astar->grid;
    for (int i=0 ; i<astar->xsize*astar->ysize ; i++, n++) {
        n->f = 0;
        n->g = 0;

        i2pos(i, &n->x, &n->y, astar->xsize);
        n->is_wall = false;

        n->h = abs(x1 - n->x) + abs(y1 - n->y);
        /*

        if (get_rnd(0, 100) < 15)
            n->is_wall = true;

        if (n->x == 25 && n->y >= 0 && n->y < astar->ysize)
            n->is_wall = true;

        if (n->x == 25 && n->y == 25)
            n->is_wall = false;

        if (n->x == 30 && n->y >= 0 && n->y < astar->ysize)
            n->is_wall = true;

        if (n->x == 30 && n->y == 5)
            n->is_wall = false;
        */

        n->parent = NULL;
    }
}

void astar_init(struct Astar* astar, struct Node* grid, struct Node** openset, struct Node** closedset, uint16_t xsize, uint16_t ysize)
{
    astar->xsize = xsize;
    astar->ysize = ysize;

    astar->grid = grid;
    astar->openset.set = openset;
    astar->closedset.set = closedset;

    astar->openset.len = 0;
    astar->closedset.len = 0;

    // set default values for callbacks. NULL will not draw!
    astar->draw_open_cb   = NULL;
    astar->draw_closed_cb = NULL;
    astar->draw_wall_cb   = NULL;
    astar->draw_path_cb   = NULL;
    astar->draw_refresh_cb     = NULL;
}

void astar_debug(struct Astar* astar)
{
    struct Node* n = astar->grid;

    for (int i=0 ; i<astar->xsize*astar->ysize ; i++, n++) {
        debug("[%d] %d x %d\n", i, n->x, n->y);
    }

}

void astar_draw(struct Astar* astar, struct Node* n_cur)
{
    struct Node** n;

    if (astar->draw_open_cb != NULL) {
        n = astar->openset.set;
        for (int i=0 ; i<astar->openset.len ; i++, n++)
            astar->draw_open_cb((*n)->x, (*n)->y);
    }

    if (astar->draw_closed_cb != NULL) {
        n = astar->closedset.set;
        for (int i=0 ; i<astar->closedset.len ; i++, n++)
            astar->draw_closed_cb((*n)->x, (*n)->y);
    }

    if (astar->draw_wall_cb != NULL) {
        for (int i=0 ; i<astar->xsize*astar->ysize ; i++) {
            struct Node* n = &astar->grid[i];
            if (n->is_wall)
                astar->draw_wall_cb(n->x, n->y);
        }
    }

    if (astar->draw_path_cb != NULL)
        path_trace_back(astar, n_cur);

    if (astar->draw_refresh_cb != NULL)
        astar->draw_refresh_cb();

    // draw start and endpoints
    //astar->draw_path_cb(astar->x0, astar->y0);
    //astar->draw_path_cb(astar->x1, astar->y1);
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

void add_to_openset(struct Astar* astar, struct Node* parent, Pos x, Pos y) {
    /* move node to openset if it doesn't exist in closedset */

    // exit if node is not in grid
    if (!is_in_grid(x, y, astar->xsize, astar->ysize))
        return;

    struct Node* n = get_node(astar->grid, x, y, astar->xsize);

    // exit if node is a wall
    if (n->is_wall)
        return;

    // exit if node is in closedlist
    if (set_node_exists(&astar->closedset, n))
        return;

    int cur_g = parent->g + 1;
    int cur_f = n->h + cur_g;

    // if node has been seen before check if it already has a shorter path
    // check if one of these is true:
    //   - new path is shorter than old path
    //   - node is not in openset
    if ((n->parent != NULL && cur_f < n->f) || !set_node_exists(&astar->openset, n)) {
        n->parent = parent;
        n->g = cur_g;
        n->f = cur_f;
        if (!set_node_exists(&astar->openset, n))
            set_add_node(&astar->openset, n);
    }
}

enum ASResult astar_find_path(struct Astar* astar)
{
    struct Node* n_start = get_node(astar->grid, astar->x0, astar->y0, astar->xsize);
    struct Node* n_end = get_node(astar->grid, astar->x1, astar->y1, astar->xsize);

    struct Set* openset = &astar->openset;
    struct Set* closedset = &astar->closedset;

    // start with start node
    set_add_node(openset, n_start);
    int i = 0;

    struct Node* n_cur;

    while (openset->len > 0) {
        n_cur = astar_find_lowest_f(openset);
        //debug("[%d] current: %d x %d\n", i++, n_cur->x, n_cur->y);

        if (n_cur->x == n_end->x && n_cur->y == n_end->y) {
            //debug("Reached end node\n");
            //astar_draw(astar, n_cur);
            return AS_SOLVED;
        }

        set_add_node(closedset, n_cur);
        //debug("[close] add: %d x %d, cur set len: %d\n", n_cur->x, n_cur->y, closedset->len);
        set_remove_node(openset, n_cur);

        //set_add_node(path, n_cur);

        // add neighbours of current node to openset
        // only if they do not eist in closedset
        // NOTE: there is very clearly a bias towards North/East because
        //       that is wat we're checking first!
        add_to_openset(astar, n_cur, n_cur->x,   n_cur->y-1);
        add_to_openset(astar, n_cur, n_cur->x+1, n_cur->y);
        add_to_openset(astar, n_cur, n_cur->x,   n_cur->y+1);
        add_to_openset(astar, n_cur, n_cur->x-1, n_cur->y);

        //set_debug(closedset, "closed");
        //set_debug(openset, "open");


        // drawing makes things a LOT slower
        // astar_draw(astar, n_cur);
    }

    //debug("DONE!\n");

    // FIXME drawing will crash if there are no nodes
    astar_draw(astar, n_cur);

    return AS_UNSOLVED;
}
