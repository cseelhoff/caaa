#pragma once
#include <stdint.h>
#define FIGHTER_NAME "fighters"
#define FIGHTER_ATTACK 3
#define FIGHTER_DEFENSE 4
#define FIGHTER_MOVES_MAX 4
#define FIGHTER_COST 10

typedef enum : uint8_t {
    FIGHTER_DONE_MOVING,
    FIGHTER_1_MOVE_LEFT,
    FIGHTER_2_MOVES_LEFT,
    FIGHTER_3_MOVES_LEFT,
    FIGHTER_UNMOVED,
    FIGHTER_STATES
} FighterState;
