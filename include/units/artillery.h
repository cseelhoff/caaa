#pragma once
#include <stdint.h>
#define ARTILLERY_NAME "artillery"
#define ARTILLERY_ATTACK 2
#define ARTILLERY_DEFENSE 2
#define ARTILLERY_MOVES_MAX 1
#define ARTILLERY_COST 4
#define ARTILLERY_WEIGHT 3

typedef enum : uint8_t {
  ARTILLERY_DONE_MOVING,
  ARTILLERY_UNMOVED,
  ARTILLERY_STATES
} ArtilleryState;