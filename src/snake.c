#include "snake.h"

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
    seg->chr  = 'x';

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

void matrix_init(struct Matrix* m, uint32_t xsize, uint32_t ysize)
{
    m->xsize = xsize;
    m->ysize = ysize;
}

void matrix_debug(struct Matrix* m, struct Snake* s)
{
    /* Print matrix to display */
    char matrix[m->ysize][m->xsize];
    memset(matrix, '0', m->xsize*m->ysize);

    struct Seg* seg = *s->head;
    while (seg) {
        matrix[seg->ypos][seg->xpos] = seg->chr;
        seg = seg->next;
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

void matrix_next(struct Matrix* m, struct Snake* s, enum Velocity v)
{
    /* Move snake one frame */

    // TODO if snake needs to grow that should happen here
    struct Seg* old = *s->tail;

    Pos x = old->xpos;
    Pos y = old->ypos;

    get_newxy(&x, &y, m->xsize, m->ysize, v);
    struct Seg* tail = seg_init(s->tail, x, y);

    snake_lremove(s->head, 1);
}
