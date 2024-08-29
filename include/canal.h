#pragma once
#include <stdint.h>
#include "typedefs.h"

#define CANALS_COUNT 2
#define CANAL_STATES 4

typedef struct {
    char* name;
    SeaIndex seas[2];
    LandIndex lands[2];
} Canal;

extern const Canal CANALS[CANALS_COUNT];
