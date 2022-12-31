#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "snake.h"

int main()
{
    struct Snake snake;
    snake_init(&snake);
    seg_init(snake.tail, 3, 5);
    seg_init(snake.tail, 7, 9);
    snake_debug(&snake);
}
