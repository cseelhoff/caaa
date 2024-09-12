#pragma once
#include <stdint.h>
#define CRUISER_NAME "cruisers"
#define CRUISER_ATTACK 3
#define CRUISER_DEFENSE 3
#define CRUISER_MOVES_MAX 2
#define CRUISER_COST 10

typedef enum : uint8_t { CRUISER_DONE_MOVING, CRUISER_UNMOVED, CRUISER_STATES } CruiserState;
