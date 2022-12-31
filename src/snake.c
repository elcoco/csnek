#include "snake.h"


uint16_t get_rand(uint16_t lower, uint16_t upper) {
    return (rand() % (upper - lower + 1)) + lower;
}

void snake_init(struct Snake* s)
{
    s->len = 1;
    s->cur_len = 1;

    // create segments linked list
    s->head = malloc(sizeof(struct Seg*));
    s->tail = malloc(sizeof(struct Seg*));

    struct Seg* seg = seg_init(NULL, 0, 0);

    *(s->head) = seg;
    *(s->tail) = seg;

}

struct Seg* seg_init(struct Seg** tail, Pos xpos, Pos ypos)
{
    /* Create new segment and replace tail */
    struct Seg* seg = malloc(sizeof(struct Seg));
    seg->xpos = xpos;
    seg->ypos = ypos;

    seg->next = NULL;

    if (tail == NULL) {
        seg->prev = NULL;
    }
    else {
        seg->prev = *tail;
        (*tail)->next = seg;
        *tail = seg;
    }
    return seg;
}

void snake_debug(struct Snake* s)
{
    /* Print out linked list */
    struct Seg* seg = *(s->head);
    int i = 0;

    while (seg) {
        printf("%d %dx%d\n", i, seg->xpos, seg->ypos);
        seg = seg->next;
        i++;
    }
}

void snake_lremove(struct Seg** head, uint16_t amount)
{
    /* Free hold head and replace by new head */
    for (int i=0 ; i<amount ; i++) {
        struct Seg* old = *head;

        // should be unreachable
        if ((*head)->next == NULL) {
            printf("Error removing head, no more segments\n");
            return;
        }

        *head = (*head)->next;
        (*head)->prev = NULL;
        free(old);
    }
}

struct Food* food_init(struct Food** tail, uint16_t xsize, uint16_t ysize)
{
    struct Food* f = malloc(sizeof(struct Food));

    // FIXME should be random
    f->xpos = get_rand(0, xsize-1);
    f->ypos = get_rand(0, ysize-1);
    f->ypos = 0;

    f->grow_fac = SNAKE_GROW_FACTOR;

    f->next = NULL;

    if (tail == NULL) {
        f->prev = NULL;
    }
    else {
        f->prev = *tail;
        (*tail)->next = f;
        *tail = f;
    }
    return f;
}

void matrix_init(struct Matrix* m, uint32_t xsize, uint32_t ysize, uint16_t nfood)
{
    m->xsize = xsize;
    m->ysize = ysize;

    // init food linked list
    m->fhead = malloc(sizeof(struct Food*));
    m->ftail = malloc(sizeof(struct Food*));

    struct Food* f = food_init(NULL, xsize, ysize);

    *m->fhead = f;
    *m->ftail = f;

    for (int i=0 ; i<nfood ; i++)
        food_init(m->ftail, xsize, ysize);

}

void matrix_debug(struct Matrix* m, struct Snake* s)
{
    /* Print matrix to display */
    char matrix[m->ysize][m->xsize];
    memset(matrix, '0', m->xsize*m->ysize);

    struct Seg* seg = *s->head;
    while (seg) {
        matrix[seg->ypos][seg->xpos] = SNAKE_SEG_CHR;
        seg = seg->next;
    }

    struct Food* f = *m->fhead;
    while (f != NULL) {
        matrix[f->ypos][f->xpos] = SNAKE_FOOD_CHR;
        f = f->next;
    }

    for (Pos y=0 ; y<m->ysize ; y++) {
        for (Pos x=0 ; x<m->xsize ; x++) {
            printf("%c", matrix[y][x]);
        }
        printf("\n");
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
        printf("Failed to get coordinates: %d, %d\n", *x, *y);
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

struct Seg* seg_detect_col(struct Seg* tail, Pos x, Pos y)
{
    /* detect colision of head segment with a segment from snake */
    struct Seg* seg = tail->prev;
    while (seg != NULL) {
        if (x == seg->xpos && y == seg->ypos)
            return seg;

        seg = seg->prev;
    }
    return NULL;
}

int8_t matrix_next(struct Matrix* m, struct Snake* s, enum Velocity v)
{
    /* Move snake one frame */

    // TODO if snake needs to grow that should happen here
    struct Seg* old = *s->tail;

    Pos x = old->xpos;
    Pos y = old->ypos;

    get_newxy(&x, &y, m->xsize, m->ysize, v);

    // detect collision with food
    // grow if collision is detected
    // otherwise remove from head
    //
    // detect collision with self
    struct Food* f = food_detect_col(*m->ftail, x, y);
    if (f != NULL) {
        s->len+=f->grow_fac;
        printf("Ate food @ %dx%d!\n", f->xpos, f->ypos);
    }

    if (seg_detect_col(*s->tail, x, y)) {
        printf("You die!\n");
        return -1;
    }

    seg_init(s->tail, x, y);
    if (s->cur_len < s->len)
        s->cur_len++;
    else
        snake_lremove(s->head, 1);

    return 0;
}
