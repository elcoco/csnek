#ifndef STATE_H
#define STATE_H

enum GameMode {
    GM_BOT,
    GM_USER
};

struct State {
    enum Direction v;
    bool is_stopped;
    bool is_paused;

    Pos xsize;
    Pos ysize;

    enum GameMode mode;
    uint32_t speed_ms;

    uint8_t grow_amount;
    uint8_t max_food;
};

#endif
