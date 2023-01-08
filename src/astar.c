#include "astar.h"

void set_debug(struct Set* set, char* prefix)
{
    for (int i=0 ; i<set->len ; i++) {
        struct Node* n = set->set[i];
        debug("[%s][%d] %dx%d f=%d g=%d, h=%d\n", prefix, i, n->x, n->y, n->f, n->g, n->h); 
    }
}

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
    /* Follow path back to origin and draw everything */
    struct Node* n = n_end;
    while (n != NULL) {
        astar->draw_path_cb(n->x, n->y);
        n = n->parent;
    }
}

void astar_set_points(struct Astar* astar, Pos x0, Pos y0, Pos x1, Pos y1)
{
    /* Set start and end points for algorithm */
    // srand(time(NULL));      // set random seed

    astar->x0 = x0;
    astar->y0 = y0;
    astar->x1 = x1;
    astar->y1 = y1;

    // Init all points with the propper heuristic (physical distance from end point
    // G cannot be set since it is the path distance to start point
    // F cannot be set since it is calculated from G: F=G+H)
    struct Node* n = astar->grid;
    for (int i=0 ; i<astar->xsize*astar->ysize ; i++, n++) {
        n->f = 0;
        n->g = 0;

        i2pos(i, &n->x, &n->y, astar->xsize);
        n->is_wall = false;
        n->h = abs(x1 - n->x) + abs(y1 - n->y);

        /*
         * Set random walls for testing, don't forget to uncomment seed initiation for randomness

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
    astar->draw_open_cb    = NULL;
    astar->draw_closed_cb  = NULL;
    astar->draw_wall_cb    = NULL;
    astar->draw_path_cb    = NULL;
    astar->draw_refresh_cb = NULL;
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

struct Node* astar_find_lowest_f(struct Set* set, struct Node** winner, uint32_t* winner_i)
{
    /* Go through set and find node with lowest fscore
     * returns node and its index in the set */
    *winner = *set->set;
    struct Node** n = set->set;
    *winner_i = 0;

    for (int i=0 ; i<set->len ; i++, n++) {
        if ((*n)->f < (*winner)->f) {
            *winner = *n;
            *winner_i = i;
        }
    }
    return *winner;
}

struct Node* astar_find_highest_f(struct Set* set, struct Node** winner, uint32_t* winner_i)
{
    /* Go through set and find node with highest fscore
     * returns node and its index in the set */
    *winner = *set->set;
    struct Node** n = set->set;
    *winner_i = 0;

    for (int i=0 ; i<set->len ; i++, n++) {
        if ((*n)->f > (*winner)->f) {
            *winner = *n;
            *winner_i = i;
        }
    }
    return *winner;
}

void set_remove_node(struct Set* set, uint32_t node_i)
{
    /* Remove node @node_i from array, shift everything to left */
    for (int i=node_i ; i<set->len ; i++)
        set->set[i] = set->set[i+1];

    set->len--;
}

bool set_node_exists(struct Set* set, struct Node* n)
{
    /* Find node in set */
    //for (int i=0 ; i<set->len ; i++) {
    for (int i=set->len-1 ; i>=0 ; i--) {
        if (set->set[i] == n)
            return true;
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

bool is_in_grid(Pos x, Pos y, uint16_t xsize, uint16_t ysize)
{
    /* Check if coordinates are within grid dimensions */
    return (x >= 0 && y >= 0 && x < xsize && y < ysize);
}

void add_to_openset(struct Astar* astar, struct Node* parent, Pos x, Pos y)
{
    /* move node to openset if it doesn't exist in closedset */
    // TODO needs optimization

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
    bool in_openset = set_node_exists(&astar->openset, n);

    // if node has been seen before check if it already has a shorter path
    // check if one of these is true:
    //   - new path is shorter than old path
    //   - node is not in openset
    if ((n->parent != NULL && cur_f < n->f) || !in_openset) {
        n->parent = parent;
        n->g = cur_g;
        n->f = cur_f;
        if (!in_openset)
            set_add_node(&astar->openset, n);
    }
}


uint32_t count_reachable(struct Astar* astar, struct Set* closedset, struct Node* n_cur)
{
    /* Recursive count of reachable nodes in grid from node n */
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



enum ASResult astar_find_path(struct Astar* astar, enum ASPathType path_type)
{
    struct Node* n_start = get_node(astar->grid, astar->x0, astar->y0, astar->xsize);
    struct Node* n_end = get_node(astar->grid, astar->x1, astar->y1, astar->xsize);

    struct Set* openset = &astar->openset;
    struct Set* closedset = &astar->closedset;

    // start with start node
    set_add_node(openset, n_start);
    int i = 0;

    struct Node* n_cur;
    uint32_t n_cur_i;

    // When all nodes in openset are evaluated we either solved the maze or
    // there is no solution
    while (openset->len > 0) {

        // Pick longest or shortest path
        if (path_type == AS_SHORTEST)
            astar_find_lowest_f(openset, &n_cur, &n_cur_i);
        else
            astar_find_highest_f(openset, &n_cur, &n_cur_i);

        // If current node is equal to the end node it means we solved the maze
        if (n_cur->x == n_end->x && n_cur->y == n_end->y) {

            if (get_reachable(astar, n_cur) < 80)
                debug("find other path\n");

            return AS_SOLVED;
        }

        set_add_node(closedset, n_cur);
        set_remove_node(openset, n_cur_i);

        // add neighbours of current node to openset
        // only if they do not eist in closedset
        // NOTE: there is a clear bias towards North/East because
        //       that is wat we're checking first!
        add_to_openset(astar, n_cur, n_cur->x,   n_cur->y-1);
        add_to_openset(astar, n_cur, n_cur->x+1, n_cur->y);
        add_to_openset(astar, n_cur, n_cur->x,   n_cur->y+1);
        add_to_openset(astar, n_cur, n_cur->x-1, n_cur->y);
    }

    astar_draw(astar, n_cur);


    return AS_UNSOLVED;
}
