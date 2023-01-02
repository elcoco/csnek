#include "snake.h"

uint16_t get_rand(uint16_t lower, uint16_t upper)
{
    return (rand() % (upper - lower + 1)) + lower;
}

void get_newxy(Pos* x, Pos* y, uint32_t xsize, uint32_t ysize, enum Velocity v)
{
    /* Get new coordinates after applying movement.
     * Wrap if outsize matrix dimensions 
     */
    if (v == VEL_N) {
        if (*y == 0)
            *y = ysize-1;
        else
            (*y)--;
    }
    else if (v == VEL_E) {
        if (*x == xsize-1)
            *x = 0;
        else
            (*x)++;
    }
    else if (v == VEL_S) {
        if (*y == ysize-1)
            *y = 0;
        else
            (*y)++;
    }
    else if (v == VEL_W) {
        if (*x == 0)
            *x = xsize-1;
        else
            (*x)--;
    }
    else {
        debug("Failed to get coordinates: %d, %d\n", *x, *y);
    }
}

void get_free_loc(struct Food** ftail, struct Seg* stail, uint16_t xsize, uint16_t ysize, Pos* x, Pos* y)
{
    /* Get unoccupied coordinates */
    while (1) {
        *x = get_rand(0, xsize-1);
        *y = get_rand(0, ysize-1);

        // make sure we don't generate food where snake or food is
        if (seg_detect_col(stail, *x, *y, 0) == NULL) {
            if (ftail == NULL)
                break;
            if (food_detect_col(*ftail, *x, *y) == NULL)
                break;
        }
    }
}


void field_init(struct Field* field, struct Snake* s, uint32_t xsize, uint32_t ysize, uint16_t max_food)
{
    field->xsize = xsize;
    field->ysize = ysize;

    field->max_food = max_food;
    field->score = 0;

    // set random seed for food location generation
    srand(time(NULL));

    // init food linked list
    field->fhead = malloc(sizeof(struct Food*));
    field->ftail = malloc(sizeof(struct Food*));

    struct Food* f = food_init(NULL, *s->stail, xsize, ysize);

    *field->fhead = f;
    *field->ftail = f;

    for (int i=1 ; i<max_food ; i++)
        food_init(field->ftail, *s->stail, xsize, ysize);
}

void field_draw(struct Field* field, struct Snake* s)
{
    /* Print field to display */
    struct Seg* seg = *s->shead;
    while (seg) {
        s->draw_cb(seg->xpos, seg->ypos);
        seg = seg->next;
    }

    struct Food* f = *field->fhead;
    while (f != NULL) {
        field->draw_cb(f->xpos, f->ypos);
        f = f->next;
    }
}

enum GameState field_next(struct Field* field, struct Snake* s, enum Velocity v)
{
    /* Move snake one frame */
    struct Seg* old = *s->stail;

    Pos x = old->xpos;
    Pos y = old->ypos;

    get_newxy(&x, &y, field->xsize, field->ysize, v);

    // grow or move
    seg_init(s->stail, x, y);
    if (s->cur_len < s->len)
        s->cur_len++;
    else
        snake_lremove(s->shead, 1);

    // detect full field
    if (s->cur_len + field->max_food >= field->xsize*field->ysize) {
        debug("field is full of snake: slen=%d, flen=%d\n", s->cur_len, field->max_food);
        return GAME_WON;
    }

    // detect colision with food
    struct Food* f = food_detect_col(*field->ftail, x, y);
    if (f != NULL) {
        s->len+=s->grow_fac;
        field->score++;
        debug("Ate food @ %dx%d!\n", f->xpos, f->ypos);
        food_init(field->ftail, *s->stail, field->xsize, field->ysize);
        food_destroy(field, f);
    }

    // detect collision with self
    if (seg_detect_col(*s->stail, x, y, 1)) {
        debug("You die!\n");
        return GAME_LOST;
    }

    return GAME_NONE;
}


void snake_init(struct Snake* s, Pos xstart, Pos ystart)
{
    s->len = 1;
    s->cur_len = 1;
    s->grow_fac = SNAKE_DEFAULT_GROW_FACTOR;

    // create segments linked list
    s->shead = malloc(sizeof(struct Seg*));
    s->stail = malloc(sizeof(struct Seg*));

    struct Seg* seg = seg_init(NULL, xstart, ystart);

    *(s->shead) = seg;
    *(s->stail) = seg;
}

void snake_lremove(struct Seg** shead, uint16_t amount)
{
    /* Free old head and replace by new head */
    for (int i=0 ; i<amount ; i++) {
        struct Seg* old = *shead;

        // should be unreachable
        if ((*shead)->next == NULL) {
            debug("Error removing head, no more segments\n");
            return;
        }

        *shead = (*shead)->next;
        (*shead)->prev = NULL;
        free(old);
    }
}



struct Seg* seg_init(struct Seg** stail, Pos xpos, Pos ypos)
{
    /* Create new segment and replace stail */
    struct Seg* seg = malloc(sizeof(struct Seg));
    seg->xpos = xpos;
    seg->ypos = ypos;
    seg->next = NULL;

    if (stail == NULL) {
        seg->prev = NULL;
    }
    else {
        seg->prev = *stail;
        (*stail)->next = seg;
        *stail = seg;
    }
    return seg;
}

struct Seg* seg_detect_col(struct Seg* stail, Pos x, Pos y, uint16_t roffset)
{
    /* detect colision of head segment with a segment from snake body */
    struct Seg* seg = stail;

    for (int i=0 ; i<roffset ; i++)
        seg = seg->prev;

    while (seg != NULL) {
        if (x == seg->xpos && y == seg->ypos)
            return seg;

        seg = seg->prev;
    }
    return NULL;
}


struct Food* food_init(struct Food** ftail, struct Seg* stail, uint16_t xsize, uint16_t ysize)
{
    struct Food* f = malloc(sizeof(struct Food));

    get_free_loc(ftail, stail, xsize, ysize, &f->xpos, &f->ypos);
    f->next = NULL;

    if (ftail == NULL) {
        f->prev = NULL;
    }
    else {
        f->prev = *ftail;
        (*ftail)->next = f;
        *ftail = f;
    }
    debug("Generated food @ %d x %d\n", f->xpos, f->ypos);
    return f;
}

struct Food* food_detect_col(struct Food* tail, Pos x, Pos y)
{
    /* detect colision of head segment with a food item */
    struct Food* f = tail;
    while (f != NULL) {
        if (x == f->xpos && y == f->ypos)
            return f;

        f = f->prev;
    }
    return NULL;
}

void food_destroy(struct Field* field, struct Food* f)
{
    struct Food* prev = f->prev;
    struct Food* next = f->next;

    if (prev != NULL && next != NULL) {
        // food is neither head or tail
        prev->next = next;
        next->prev = prev;
    }
    else if (prev == NULL) {
        // replace head
        *field->fhead = next;
        next->prev = NULL;
    }
    else if (next == NULL) {
        // replace tail
        *field->ftail = prev;
        prev->next = NULL;
    }
    else {
        debug("Should be unreachable\n");
    }
    free(f);
}


/* Some debug functions */
void snake_debug(struct Snake* s)
{
    /* Print out linked list */
    debug("SNAKE\n");

    struct Seg* seg = *(s->shead);
    int i = 0;

    while (seg) {
        debug("%d %dx%d\n", i, seg->xpos, seg->ypos);
        seg = seg->next;
        i++;
    }
}

void food_debug(struct Field* field)
{
    /* Print out linked list */
    debug("FOOD\n");

    struct Food* f = *field->fhead;
    int i = 0;

    while (f != NULL) {
        debug("%d %dx%d\n", i, f->xpos, f->ypos);
        f = f->next;
        i++;
    }
}

void field_debug(struct Field* field, struct Snake* s)
{
    /* Print field to display */
    char matrix[field->ysize][field->xsize];
    memset(matrix, '0', field->xsize*field->ysize);

    struct Seg* seg = *s->shead;
    while (seg) {
        matrix[seg->ypos][seg->xpos] = SNAKE_SEG_CHR;
        seg = seg->next;
    }

    struct Food* f = *field->fhead;
    while (f != NULL) {
        matrix[f->ypos][f->xpos] = SNAKE_FOOD_CHR;
        f = f->next;
    }

    for (Pos y=0 ; y<field->ysize ; y++) {
        for (Pos x=0 ; x<field->xsize ; x++) {
            debug("%c", matrix[y][x]);
        }
        debug("\n");
    }
}
