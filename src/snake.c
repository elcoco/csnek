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

    seg->next = NULL;

    if (tail == NULL) {
        printf("first\n");
        seg->prev = NULL;
    }
    else {
        printf("replace tail\n");
        seg->prev = *tail;
        (*tail)->next = seg;
        *tail = seg;
    }
    return seg;
}

void snake_debug(struct Snake* s)
{
    struct Seg* seg = *(s->head);
    int i = 0;

    while (seg) {
        printf("%d %dx%d\n", i, seg->xpos, seg->ypos);
        seg = seg->next;
        i++;
    }

}
