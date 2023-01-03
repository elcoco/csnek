#include "snake.h"

uint16_t get_rand(uint16_t lower, uint16_t upper)
{
    return (rand() % (upper - lower + 1)) + lower;
}

void get_newxy(Pos* x, Pos* y, uint32_t xsize, uint32_t ysize, enum Direction v)
{
    /* Get new coordinates after applying movement.
     * Wrap if outsize matrix dimensions 
     */
    if (v == DIR_N) {
        if (*y == 0)
            *y = ysize-1;
        else
            (*y)--;
    }
    else if (v == DIR_E) {
        if (*x == xsize-1)
            *x = 0;
        else
            (*x)++;
    }
    else if (v == DIR_S) {
        if (*y == ysize-1)
            *y = 0;
        else
            (*y)++;
    }
    else if (v == DIR_W) {
        if (*x == 0)
            *x = xsize-1;
        else
            (*x)--;
    }
    else {
        debug("Failed to get coordinates: %d, %d\n", *x, *y);
    }
}

void get_free_loc(struct FoodItem** ftail, struct Seg* stail, uint16_t xsize, uint16_t ysize, Pos* x, Pos* y)
{
    /* Get random coordinates not occupied with snake body or fooditem */
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


void game_init(struct Game* game, uint32_t xsize, uint32_t ysize, uint16_t maxfood)
{
    game->xsize = xsize;
    game->ysize = ysize;
    game->score = 0;
    game->maxfood = maxfood;
    game->grow_fac = SNAKE_DEFAULT_GROW_FACTOR;

    snake_init(&game->snake, xsize/2, ysize/2);
    food_init(&game->food, *game->snake.stail, xsize, ysize, maxfood);
}

void game_draw(struct Game* game)
{
    /* Call all draw callbacks */
    struct Snake* snake = &game->snake;
    struct Food* food = &game->food;

    /* Print field to display */
    struct Seg* seg = *snake->shead;
    while (seg) {
        snake->draw_cb(seg->xpos, seg->ypos);
        seg = seg->next;
    }

    struct FoodItem* f = *food->fhead;
    while (f != NULL) {
        food->draw_cb(f->xpos, f->ypos);
        f = f->next;
    }
}

enum GameState game_next(struct Game* game, enum Direction v)
{
    /* Move snake one frame */
    struct Snake* snake = &game->snake;
    struct Food* food = &game->food;

    struct Seg* old = *snake->stail;
    Pos x = old->xpos;
    Pos y = old->ypos;

    get_newxy(&x, &y, game->xsize, game->ysize, v);

    // grow or move
    seg_init(snake->stail, x, y);
    if (snake->cur_len < snake->len)
        snake->cur_len++;
    else
        snake_lremove(snake->shead, 1);

    // detect full field
    if (snake->cur_len + game->maxfood >= game->xsize*game->ysize) {
        return GAME_WON;
    }

    // detect colision with food item
    struct FoodItem* f = food_detect_col(*food->ftail, x, y);
    if (f != NULL) {
        snake->len+=game->grow_fac;
        game->score++;
        fooditem_init(food->ftail, *snake->stail, game->xsize, game->ysize);
        fooditem_destroy(f, food->fhead, food->ftail);
    }

    // detect collision with snake body
    if (seg_detect_col(*snake->stail, x, y, 1)) {
        return GAME_LOST;
    }

    return GAME_NONE;
}


void snake_init(struct Snake* s, Pos xstart, Pos ystart)
{
    s->len = 1;
    s->cur_len = 1;

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


void food_init(struct Food* food, struct Seg* stail, uint16_t xsize, uint16_t ysize, uint16_t maxfood)
{
    // set random seed for food location generation
    srand(time(NULL));

    // init food linked list
    food->fhead = malloc(sizeof(struct FoodItem*));
    food->ftail = malloc(sizeof(struct FoodItem*));

    struct FoodItem* f = fooditem_init(NULL, stail, xsize, ysize);

    *food->fhead = f;
    *food->ftail = f;

    for (int i=1 ; i<maxfood ; i++)
        fooditem_init(food->ftail, stail, xsize, ysize);
}

struct FoodItem* fooditem_init(struct FoodItem** ftail, struct Seg* stail, uint16_t xsize, uint16_t ysize)
{
    struct FoodItem* f = malloc(sizeof(struct FoodItem));

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

struct FoodItem* food_detect_col(struct FoodItem* ftail, Pos x, Pos y)
{
    /* detect colision of head segment with a food item */
    struct FoodItem* f = ftail;
    while (f != NULL) {
        if (x == f->xpos && y == f->ypos)
            return f;

        f = f->prev;
    }
    return NULL;
}

void fooditem_destroy(struct FoodItem* f, struct FoodItem** fhead, struct FoodItem** ftail)
{
    /* Free fooditem and remove from linked list */
    struct FoodItem* prev = f->prev;
    struct FoodItem* next = f->next;

    if (prev != NULL && next != NULL) {
        // food is neither head or tail
        prev->next = next;
        next->prev = prev;
    }
    else if (prev == NULL) {
        // replace head
        *fhead = next;
        next->prev = NULL;
    }
    else if (next == NULL) {
        // replace tail
        *ftail = prev;
        prev->next = NULL;
    }
    else {
        debug("Should be unreachable\n");
    }
    free(f);
}
