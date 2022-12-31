#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>     // for utf8 in curses
#include <signal.h>     // catch SIGTERM

#include "snake.h"

#define XSIZE 10
#define YSIZE 10

#define NFOOD 1


int sigint_caught = 0;

void on_sigint(int signum)
{
    sigint_caught = 1;
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



    struct Snake s;
    struct Field field;

    snake_init(&s);
    field_init(&field, &s, XSIZE, YSIZE, NFOOD);

    while (field_next(&field, &s, VEL_W) >= 0) {
        snake_debug(&s);
        food_debug(&field);
        field_debug(&field, &s);
        debug("\n");
        sleep(1);
    }

    ui_cleanup();
}
