#pragma once
#define DESTROYER_NAME "destroyers"
#define DESTROYER_ATTACK 2
#define DESTROYER_DEFENSE 2
#define DESTROYER_MOVES_MAX 2
#define DESTROYER_COST 8

typedef enum {
    DESTROYER_DONE_MOVING,
    DESTROYER_UNMOVED,
    DESTROYER_STATES
} DestroyerState;
