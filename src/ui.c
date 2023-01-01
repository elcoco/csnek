#include "ui.h"

WINDOW *ui_window = NULL;

int ui_init()
{
    // https://stackoverflow.com/questions/61347351/ncurses-stdin-redirection
    // NOTE we have to reopen stdin, piping data into application doesn't play well with ncurses
    //if (!freopen("/dev/tty", "r", stdin)) {
    //    perror("/dev/tty");
    //    exit(1);
    //}

    // set ESCDELAY so we don't have to wait a seconds after pressing escape key
    setenv("ESCDELAY", "25", 1); 
    ui_window = initscr();

    if (ui_window == NULL)
        return 0;

    ui_init_colors();
    curs_set(0);            // don't show cursor
    cbreak();               // don't wait for enter
    noecho();               // don't echo input to screen
    nodelay(stdscr, TRUE);  // don't block
    keypad(stdscr, TRUE);   // Enables keypad mode also necessary for mouse clicks

    return 1;
}

void ui_cleanup()
{
    printf("\033[?1003l\n"); // Disable mouse movement events, as l = low
    delwin(ui_window);
    endwin();
}

int add_chr(WINDOW* win, int32_t y, int32_t x, int32_t fgcol, int32_t bgcol, char c)
{
    set_color(win, fgcol, bgcol);
    mvwaddch(win, y, x, c);
    unset_color(win, fgcol, bgcol);
    return 0;
}

int add_str(WINDOW* win, int32_t y, int32_t x, int32_t fgcol, int32_t bgcol, char* fmt, ...)
{
//    assertf(x>=0, "x = %d, max = %d", x, getmaxx(win)-1);
//    assertf(y>=0, "y = %d, max = %d", y, getmaxy(win)-1);
//    assertf(x<getmaxx(win), "x = %d, max = %d", x, getmaxx(win)-1);
//    assertf(y<getmaxy(win), "y = %d, max = %d", y, getmaxy(win)-1);
//
    va_list ptr;
    va_start(ptr, fmt);

    set_color(win, fgcol, bgcol);
    wmove(win, y, x);
    vw_printw(win, fmt, ptr);
    unset_color(win, fgcol, bgcol);
    va_end(ptr);

    return 0;
}
void ui_refresh(WINDOW* win)
{
    if (win)
        wrefresh(win);
    else
        refresh();
}

void ui_init_colors()
{
    use_default_colors();

    if(has_colors()) {
        if(start_color() == OK) {
            for (uint32_t bg=0 ; bg<ncolors ; bg++) {
                for (uint32_t fg=0 ; fg<ncolors ; fg++)
                    init_pair((fg+1)+(bg*ncolors), ccolors[fg], ccolors[bg]);
            }
        } else {
            addstr("Cannot start colours\n");
            refresh();
        }
    } else {
        addstr("Not colour capable\n");
        refresh();
    }
}

void set_color(WINDOW* win, uint32_t fgcolor, uint32_t bgcolor)
{
    wattrset(win, COLOR_PAIR(fgcolor + ((bgcolor-1)*ncolors)));
}

void unset_color(WINDOW* win, uint32_t fgcolor, uint32_t bgcolor)
{
    wattroff(win, COLOR_PAIR(fgcolor + (bgcolor*ncolors)));
}

void ui_erase(WINDOW* win)
{
    /* Erase window wrapper */

    // TODO reset color matrix here

    if (win)
        werase(win);
    else
        erase();
}

void ui_show_error(WINDOW* win, char* fmt, ...)
{
    va_list ptr;
    va_start(ptr, fmt);

    char msg[100] = {'\0'};
    vsprintf(msg, fmt, ptr);

    uint32_t xpos = (getmaxx(win) / 2) - (strlen(msg) / 2);
    uint32_t ypos = getmaxy(win) / 2;
    ui_erase(win);
    add_str(win, ypos, xpos, CRED, CDEFAULT, msg);
    wrefresh(win);

    va_end(ptr);
}
