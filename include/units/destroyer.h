#pragma once
#include <stdint.h>
#define DESTROYER_NAME "destroyers"
#define DESTROYER_ATTACK 2
#define DESTROYER_DEFENSE 2
#define DESTROYER_MOVES_MAX 2
#define DESTROYER_COST 8

typedef enum : uint8_t {
    DESTROYER_DONE_MOVING,
    DESTROYER_UNMOVED,
    DESTROYER_STATES
} DestroyerState;
