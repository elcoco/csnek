#ifndef ASTAR_H
#define ASTAR_H

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "utils.h"

// https://optimization.cbe.cornell.edu/index.php?title=A-star_algorithm
// https://en.wikipedia.org/wiki/A*_search_algorithm#Pseudocode

/* Algorithm is intended to run on microcontrollers so
 * that means no malloc
 *
 * f(n) = g(n) + h(n)
 *
 * g = distance from current node to start node
 *     in our case it is a equally spaced grid
 *     so g = amount_prev_nodes * 1
 *
 * h = heuristic. An educated guess from current node
 *     to destination. We could calculate a straight line
 *     h = 
 *     This gives the algorithm a bias to prefer quick routes.
 *     NOTE: should never over estimate
 *
 * f = g_score + h_score
 */

#define XSIZE 50
#define YSIZE 50

#define CHKSUM 123456

enum ASResult {
    AS_SOLVED,
    AS_UNSOLVED,
    AS_ERROR
};

enum ASPathType {
    AS_SHORTEST,
    AS_LONGEST
};

typedef uint16_t Pos;

struct Node {
    uint16_t f;     // g+h
    uint16_t g;     // cost of start->current
    uint16_t h;     // cost of current->end
                    
    // indicate if this node is an obstacle to move around to
    bool is_wall;

    Pos x;
    Pos y;

    // keep reference to previous node
    struct Node* parent;

    int chksum;
};

struct Set {
    uint32_t len;

    // TODO find a more memory efficient way to store nodes
    struct Node** set;
};

struct Astar {
    uint16_t xsize;
    uint16_t ysize;

    // start node
    Pos x0;
    Pos y0;

    // end node
    Pos x1;
    Pos y1;

    struct Node* grid;

    struct Set openset;

    // Nodes that are finished being evaluated and should never be revisited
    struct Set closedset;

    void(*draw_open_cb)(Pos x, Pos y);
    void(*draw_closed_cb)(Pos x, Pos y);
    void(*draw_path_cb)(Pos x, Pos y);
    void(*draw_wall_cb)(Pos x, Pos y);
    void(*draw_refresh_cb)();

};

void astar_init(struct Astar* astar, struct Node* grid, struct Node** openset, struct Node** closedset, uint16_t xsize, uint16_t ysize);
void astar_set_points(struct Astar* astar, Pos x0, Pos y0, Pos x1, Pos y1);
void astar_debug(struct Astar* astar);
enum ASResult astar_find_path(struct Astar* astar, enum ASPathType path_type);

struct Node* get_node(struct Node* grid, Pos x, Pos y, uint16_t xsize);
void astar_draw(struct Astar* astar, struct Node* n_cur);

bool set_node_exists(struct Set* set, struct Node* n);
void set_add_node(struct Set* set, struct Node* n);
bool is_in_grid(Pos x, Pos y, uint16_t xsize, uint16_t ysize);

#endif
