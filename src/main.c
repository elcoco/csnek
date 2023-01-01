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


int sigint_caught = 0;

void on_sigint(int signum)
{
    sigint_caught = 1;
}

void write_snake_cb(Pos x, Pos y)
{
    ui_matrix_set(&m, y, x, COLOR_RED, COLOR_BLACK, 'X');
    debug("%d x %d\n");
}

void write_food_cb(Pos x, Pos y)
{
    debug("%d x %d\n");
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



    struct Snake snake;
    struct Field field;

    snake_init(&snake);
    field_init(&field, &snake, XSIZE, YSIZE, NFOOD);

    snake.write_cb = &write_snake_cb;
    field.write_cb = &write_food_cb;
    

    while (field_next(&field, &snake, VEL_W) >= 0) {
        snake_debug(&snake);
        food_debug(&field);
        field_debug(&field, &snake);
        field_print(&field, &snake);
        debug("\n");
        sleep(1);
    }

    ui_cleanup();
}
