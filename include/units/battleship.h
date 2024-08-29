#pragma once
#define BATTLESHIP_NAME "battleship"
#define BATTLESHIP_ATTACK 4
#define BATTLESHIP_DEFENSE 4
#define BATTLESHIP_MOVES_MAX 2
#define BATTLESHIP_COST 22

typedef enum {
    BATTLESHIP_DONE_MOVING,
    BATTLESHIP_UNMOVED,
    BATTLESHIP_STATES
} BattleshipState;

#define BS_DAMAGED_NAME "damaged_bs"
