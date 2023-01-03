#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>     // for utf8 in curses
#include <signal.h>     // catch SIGTERM

#include "snake.h"
#include "ui.h"
#include "utils.h"

// grow n segments when eating food
#define GROW_FAC 7

// amount of food items that are on screen at once
#define MAXFOOD 1

// interval inbetween frames
#define INTERVAL 100*1000

#define BAR_YSIZE 1
#define SLEEP_CHECK_INTERVAL 50
#define SNAKE_CHR "█"
#define FOOD_CHR "█"


struct State {
    enum Direction v;
    bool is_stopped;
    bool is_paused;

    Pos xsize;
    Pos ysize;
};

WINDOW* root_win;
WINDOW* bar_win;
WINDOW* field_win;

int sigint_caught = 0;

void on_sigint(int signum)
{
    sigint_caught = 1;
}

void draw_snake_cb(Pos x, Pos y)
{
    /* callback to draw snake segment to display */
    add_str(field_win, y, x, CRED, CDEFAULT, SNAKE_CHR);
}

void draw_food_cb(Pos x, Pos y)
{
    /* callback to draw food item to display */
    add_str(field_win, y, x, CBLUE, CDEFAULT, FOOD_CHR);
}

void check_term_size(WINDOW* win, Pos xsize, Pos ysize)
{
    /* Check if window is the size of field or bigger
     * Otherwise, display message and wait for window resize
     */
    int x,y;

    while (COLS < xsize || LINES < ysize) {
        getmaxyx(root_win, y, x);
        debug("xy: %d:%d ? %d:%d\n", xsize, ysize, y, x);
        sleep(1);
    }
}

bool get_user_input(void* arg)
{
    // s struct is passed as an argument to a callback, cast it to the proper type
    struct State* s = arg;

    /* check for user input, return 1 if there was input */
    int c = getch();

    if (c != ERR) {
        switch (c) {
            case 'q':
                s->is_stopped = true;
                break;
            case KEY_LEFT:
            case 'h':
                if (s->v == DIR_E)
                    break;
                s->v = DIR_W;
                break;
            case KEY_RIGHT:
            case 'l':
                if (s->v == DIR_W)
                    break;
                s->v = DIR_E;
                break;
            case KEY_UP:
            case 'k':
                if (s->v == DIR_S)
                    break;
                s->v = DIR_N;
                break;
            case KEY_DOWN:
            case 'j':
                if (s->v == DIR_N)
                    break;
                s->v = DIR_S;
                break;
            case ' ':
                s->is_paused = !s->is_paused;
                break;
            //case KEY_RESIZE:
            //    check_term_size(root_win, s->xsize, s->ysize);
            //    break;
            default:
                return false;
        }

        // flush chars
        while (c != ERR)
            c = getch();

        return true;
    }
    return false;
}

void bar_draw(WINDOW* win, struct Game* game)
{
    add_str(win, 0, 0, CGREEN, CDEFAULT, "Score: %d", game->score);
}

bool non_blocking_sleep(int interval, bool(*callback)(void* arg), void* arg)
{
    /* Do a non blocking sleep that checks for user input */
    struct timeval t_start, t_end;
    gettimeofday(&t_start, NULL);

    while (1) {
        gettimeofday(&t_end, NULL);
        if ((t_end.tv_sec*1000000 + t_end.tv_usec) - (t_start.tv_sec*1000000 + t_start.tv_usec) >= interval)
            break;

        // if input
        if (callback(arg))
            return true;

        usleep(SLEEP_CHECK_INTERVAL);
    }
    return false;
}

void state_init(struct State* s)
{
    s->v = DIR_E;
    s->is_stopped = false;
    s->is_paused = false;

    getmaxyx(root_win, s->xsize, s->ysize);

}

void show_msg(char* msg)
{
    /* Show a message and wait for keypress */
    add_str_center(field_win, CGREEN, CDEFAULT, msg);
    ui_refresh(field_win);

    nodelay(stdscr, FALSE);  // do block
    getch();
    nodelay(stdscr, TRUE);   // don't block
}

int main()
{
    // for UTF8 in curses, messes with atof() see: read_stdin()
    setlocale(LC_ALL, "");

    //// catch sigint (CTRL-C)
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = on_sigint;
    sigaction(SIGINT, &action, NULL);


    // setup ncurses windows
    ui_init();

    root_win = newwin(0, 0, 0, 0);

    int ysize, xsize;
    getmaxyx(root_win, ysize, xsize);

    const int field_ysize = ysize - BAR_YSIZE;

    bar_win = derwin(root_win, BAR_YSIZE, xsize, 0, 0);
    field_win = derwin(root_win, field_ysize, xsize, BAR_YSIZE, 0);


    // setup snake structs
    struct State s;
    struct Game game;

    state_init(&s);
    game_init(&game, xsize, field_ysize, MAXFOOD);

    game.grow_fac = GROW_FAC;
    game.snake.draw_cb = &draw_snake_cb;
    game.food.draw_cb = &draw_food_cb;

    // main loop
    while (! s.is_stopped) {
        if (s.is_paused) {
            show_msg("PAUSED");
            s.is_paused = false;
        }
        else {
            // go to next frame
            enum GameState gs = game_next(&game, s.v);

            if (gs == GAME_LOST) {
                show_msg("YOU LOST BRAH!");
                s.is_stopped = true;
            }
            else if (gs == GAME_WON) {
                show_msg("YOU WON!");
                s.is_stopped = true;
            }

            ui_erase(field_win);
            ui_erase(bar_win);

            game_draw(&game);
            bar_draw(bar_win, &game);

            ui_refresh(field_win);
            ui_refresh(bar_win);
        }

        if (non_blocking_sleep(INTERVAL, &get_user_input, &s)) {
        }
    }

    ui_cleanup();
}
