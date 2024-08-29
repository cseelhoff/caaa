#pragma once
#define TANK_NAME "tanks"
#define TANK_ATTACK 3
#define TANK_DEFENSE 3
#define TANK_MOVES_MAX 2
#define TANK_COST 5
#define TANK_WEIGHT 3

typedef enum {
    TANK_DONE_MOVING,
    TANK_1_MOVE_LEFT,
    TANK_UNMOVED,
    TANK_STATES
} TankState;
