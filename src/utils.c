#include "utils.h"

void die(char* msg)
{
    printf("ERROR: %s\n", msg);
    abort();
    //exit(0);
}

void debug(char* fmt, ...)
{
    char buf[100];

    va_list ptr;
    va_start(ptr, fmt);
    vsprintf(buf, fmt, ptr);
    va_end(ptr);

    FILE* fp = fopen(LOG_PATH, "a");
    fputs(buf, fp);
    fclose(fp);
}
