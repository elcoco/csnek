#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>   // for non blocking sleep
#include <curses.h>
#include <errno.h>


#define LOG_PATH "./snake.log"

void debug(char* fmt, ...);
void die(char* msg);

#endif
