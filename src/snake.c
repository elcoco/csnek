#include "snake.h"


uint16_t get_rand(uint16_t lower, uint16_t upper) {
    return (rand() % (upper - lower + 1)) + lower;
}

void snake_init(struct Snake* s)
{
    s->len = 1;
    s->cur_len = 1;
    s->grow_fac = SNAKE_DEFAULT_GROW_FACTOR;

    // create segments linked list
    s->shead = malloc(sizeof(struct Seg*));
    s->stail = malloc(sizeof(struct Seg*));

    struct Seg* seg = seg_init(NULL, 0, 0);

    *(s->shead) = seg;
    *(s->stail) = seg;
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

void snake_lremove(struct Seg** shead, uint16_t amount)
{
    /* Free hold head and replace by new head */
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

struct Food* food_init(struct Food** ftail, struct Seg* stail, uint16_t xsize, uint16_t ysize)
{
    struct Food* f = malloc(sizeof(struct Food));

    while (1) {
        f->xpos = get_rand(0, xsize-1);
        f->ypos = get_rand(0, ysize-1);

        // FIXME 0 is hardcoded for debugging !!!!!
        //f->ypos = 0;

        // make sure we don't generate food where snake or food is
        if (seg_detect_col(stail, f->xpos, f->ypos, 0) == NULL) {
            if (ftail == NULL)
                break;
            if (food_detect_col(*ftail, f->xpos, f->ypos) == NULL)
                break;
        }
    }

    f->next = NULL;

    if (ftail == NULL) {
        f->prev = NULL;
    }
    else {
        f->prev = *ftail;
        (*ftail)->next = f;
        *ftail = f;
    }
    debug("NEW FOOD @ %d x %d\n", f->xpos, f->ypos);
    return f;
}

void field_init(struct Field* field, struct Snake* s, uint32_t xsize, uint32_t ysize, uint16_t max_food)
{
    field->xsize = xsize;
    field->ysize = ysize;

    field->max_food = max_food;
    field->score = 0;

    // init food linked list
    field->fhead = malloc(sizeof(struct Food*));
    field->ftail = malloc(sizeof(struct Food*));

    struct Food* f = food_init(NULL, *s->stail, xsize, ysize);

    *field->fhead = f;
    *field->ftail = f;

    for (int i=0 ; i<max_food ; i++)
        food_init(field->ftail, *s->stail, xsize, ysize);
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

void field_print(struct Field* field, struct Snake* s)
{
    /* Print field to display */
    struct Seg* seg = *s->shead;
    while (seg) {
        s->write_cb(seg->xpos, seg->ypos);
        seg = seg->next;
    }

    struct Food* f = *field->fhead;
    while (f != NULL) {
        field->write_cb(f->xpos, f->ypos);
        f = f->next;
    }
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

struct Seg* seg_detect_col(struct Seg* stail, Pos x, Pos y, uint16_t roffset)
{
    /* detect colision of head segment with a segment from snake */
    //struct Seg* seg = stail->prev;
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
        debug("bot are NULL\n");
    }
    free(f);
}

int8_t field_next(struct Field* field, struct Snake* s, enum Velocity v)
{
    /* Move snake one frame, returns -1 on death */

    struct Seg* old = *s->stail;

    Pos x = old->xpos;
    Pos y = old->ypos;

    get_newxy(&x, &y, field->xsize, field->ysize, v);

    debug("POS: %d x %d\n", x, y);

    // grow or move
    seg_init(s->stail, x, y);
    if (s->cur_len < s->len)
        s->cur_len++;
    else
        snake_lremove(s->shead, 1);

    // detect full field
    // FIXME uncomment, using top line only for debugging
    if (s->cur_len + field->max_food >= field->xsize*field->ysize) {
    //if (s->cur_len + field->max_food >= field->xsize) {
        debug("field is full of snake: slen=%d, flen=%d\n", s->cur_len, field->max_food);
        return -1;
    }

    // detect colision with food
    struct Food* f = food_detect_col(*field->ftail, x, y);
    if (f != NULL) {
        s->len+=s->grow_fac;
        field->score++;
        debug("Ate food @ %dx%d!\n", f->xpos, f->ypos);
        food_destroy(field, f);
        food_init(field->ftail, *s->stail, field->xsize, field->ysize);
    }

    // detect collision with self
    if (seg_detect_col(*s->stail, x, y, 1)) {
        debug("You die!\n");
        return -1;
    }


    return 0;
}
