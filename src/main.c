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

#define XSIZE 10
#define YSIZE 10

#define BAR_YSIZE 1

#define NFOOD 1
#define SLEEP_CHECK_INTERVAL 50
#define INTERVAL 100*1000

#define SNAKE_CHR "█"
#define FOOD_CHR "█"


struct State {
    enum Velocity v;
    bool is_stopped;
    bool is_paused;
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
    add_str(field_win, y, x, CRED, CDEFAULT, SNAKE_CHR);
}

void draw_food_cb(Pos x, Pos y)
{
    add_str(field_win, y, x, CBLUE, CDEFAULT, FOOD_CHR);
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
                if (s->v == VEL_E)
                    break;
                s->v = VEL_W;
                break;
            case KEY_RIGHT:
            case 'l':
                if (s->v == VEL_W)
                    break;
                s->v = VEL_E;
                break;
            case KEY_UP:
            case 'k':
                if (s->v == VEL_S)
                    break;
                s->v = VEL_N;
                break;
            case KEY_DOWN:
            case 'j':
                if (s->v == VEL_N)
                    break;
                s->v = VEL_S;
                break;
            case ' ':
                s->is_paused = !s->is_paused;
                break;
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

void bar_draw(WINDOW* win, struct Field* field)
{
    add_str(win, 0, 0, CGREEN, CDEFAULT, "Score: %d", field->score);
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
    s->v = VEL_E;
    s->is_stopped = false;
    s->is_paused = false;
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

    ui_init();

    root_win = newwin(0, 0, 0, 0);

    int ysize, xsize;
    getmaxyx(root_win, ysize, xsize);

    const int field_ysize = ysize - BAR_YSIZE;

    bar_win = derwin(root_win, BAR_YSIZE, xsize, 0, 0);
    field_win = derwin(root_win, field_ysize, xsize, BAR_YSIZE, 0);

    struct State s;
    struct Snake snake;
    struct Field field;

    state_init(&s);
    snake_init(&snake, xsize/2, field_ysize/2);
    field_init(&field, &snake, xsize, field_ysize, NFOOD);

    snake.grow_fac = 5;

    snake.draw_cb = &draw_snake_cb;
    field.draw_cb = &draw_food_cb;

    while (! s.is_stopped) {
        if (s.is_paused) {
            add_str_center(field_win, CGREEN, CDEFAULT, "PAUSED");
            ui_refresh(field_win);
        }
        else {

            // get next frame
            enum GameState gs = field_next(&field, &snake, s.v);

            if (gs == GAME_LOST)
                s.is_stopped = true;
            else if (gs == GAME_LOST)
                s.is_stopped = true;

            ui_erase(field_win);
            ui_erase(bar_win);

            field_draw(&field, &snake);
            bar_draw(bar_win, &field);

            ui_refresh(field_win);
            ui_refresh(bar_win);
        }

        if (non_blocking_sleep(INTERVAL, &get_user_input, &s)) {
        }
    }

    ui_cleanup();
}
