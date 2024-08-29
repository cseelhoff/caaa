#pragma once
#define SUB_NAME "subs"
#define SUB_ATTACK 2
#define SUB_DEFENSE 1
#define SUB_MOVES_MAX 2
#define SUB_COST 8

typedef enum {
    SUB_DONE_MOVING,
    SUB_1_MOVE_LEFT,
    SUB_UNMOVED,
    SUB_STATES
} SubState;
