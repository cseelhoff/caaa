#pragma once
#include "land.h"
#include <stdint.h>

#define ALIGNMENT_128 128
#define ALIGNMENT_64 64
#define ALIGNMENT_32 32
#define ALIGNMENT_16 16
#define MAX_UINT8_T 255

typedef uint8_t MovesRemaining;
typedef uint8_t AirIndex;
typedef uint8_t LandIndex;
typedef uint8_t Dollars;
typedef uint8_t AirIndexCount;
typedef uint8_t LandIndexCount;
typedef uint8_t SeaIndexCount;
typedef uint8_t SeaConnectionIndex;
typedef uint8_t LandConnectionIndex;

typedef uint8_t PlayerIndex;
typedef uint8_t LandUnitStateSum;
typedef uint8_t SeaUnitStateSum;
typedef uint8_t AirUnitStateSum;
typedef uint8_t LandUnitTypeSum;
typedef uint8_t SeaUnitTypeSum;
typedef uint8_t ArmySum;
typedef uint8_t NavySum;

typedef uint8_t Tonnes;
typedef uint8_t Power;
typedef uint8_t Movement;
typedef uint8_t LandUnitStateCount;
typedef uint8_t SeaUnitStateCount;
typedef uint8_t SeaUnitState;

typedef uint8_t CanalState;

typedef LandIndex LandIndexArray[LANDS_COUNT - 1];
