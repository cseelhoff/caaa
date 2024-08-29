#pragma once
#define INFANTRY_NAME "infantry"
#define INFANTRY_ATTACK 1
#define INFANTRY_DEFENSE 2
#define INFANTRY_MOVES_MAX 1
#define INFANTRY_COST 3
#define INFANTRY_WEIGHT 2

typedef enum {
    INFANTRY_DONE_MOVING,
    INFANTRY_UNMOVED,
    INFANTRY_STATES
} InfantryState;
