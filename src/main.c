#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>     // for utf8 in curses
#include <signal.h>     // catch SIGTERM

#include "snake.h"
#include "ui.h"

#define XSIZE 10
#define YSIZE 10

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

WINDOW* win;

int sigint_caught = 0;

void on_sigint(int signum)
{
    sigint_caught = 1;
}

void write_snake_cb(Pos x, Pos y)
{
    add_str(win, y, x, CRED, CDEFAULT, SNAKE_CHR);
}

void write_food_cb(Pos x, Pos y)
{
    add_str(win, y, x, CBLUE, CDEFAULT, FOOD_CHR);
}

bool check_user_input(void* arg)
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
            case 'h':
                s->v = VEL_W;
                break;
            case 'l':
                s->v = VEL_E;
                break;
            case 'k':
                s->v = VEL_N;
                break;
            case 'j':
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

    //// init lock
    //if (pthread_mutex_init(&lock, NULL) != 0)
    //    die("\nMutex init failed\n");

    ui_init();

    win = newwin(0, 0, 0, 0);


    struct State s;
    state_init(&s);

    struct Snake snake;
    struct Field field;

    int ysize, xsize;
    getmaxyx(win, ysize, xsize);

    snake_init(&snake);
    field_init(&field, &snake, xsize, ysize, NFOOD);

    snake.write_cb = &write_snake_cb;
    field.write_cb = &write_food_cb;
    

    while (! s.is_stopped) {
        if (! s.is_paused) {

            if (field_next(&field, &snake, s.v) < 0)
                s.is_stopped = true;;

            //snake_debug(&snake);
            //food_debug(&field);
            //field_debug(&field, &snake);

            ui_erase(win);
            field_print(&field, &snake);
            ui_refresh(win);
        }

        if (non_blocking_sleep(INTERVAL, &check_user_input, &s)) {
        }
    }

    ui_cleanup();
}
