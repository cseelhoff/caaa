#ifndef INACTIVE_UNIT_STACK_H
#define INACTIVE_UNIT_STACK_H

#include "unit_health.h"

#define TOTAL_INACTIVE_UNIT_STATUS_TYPES 1

typedef struct InactiveUnitStack {
    UnitHealth unit_health;
    unsigned char quantity;
    struct InactiveUnitStack* stack_after_hit;
} InactiveUnitStack;

#endif