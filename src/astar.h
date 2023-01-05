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
};

struct Set {
    uint32_t len;

    // TODO find a more memory efficient way to store nodes
    struct Node* set[XSIZE*YSIZE];
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

    // the most efficient path from start to end
    // TODO tweak size
    struct Set path;
    
    struct Node grid[XSIZE*YSIZE];
    struct Set openset;

    // Nodes that are finished being evaluated and should never be revisited
    struct Set closedset;

    void(*draw_open_cb)(Pos x, Pos y);
    void(*draw_closed_cb)(Pos x, Pos y);
    void(*draw_path_cb)(Pos x, Pos y);
    void(*draw_wall_cb)(Pos x, Pos y);
};

void astar_init(struct Astar* astar, Pos x0, Pos y0, Pos x1, Pos y1);
void astar_debug(struct Astar* astar);
void astar_find_path(struct Astar* astar);

#endif
