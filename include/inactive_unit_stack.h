#ifndef INACTIVE_UNIT_STACK_H
#define INACTIVE_UNIT_STACK_H

#include "unit_health.h"

typedef struct InactiveUnitStack {
    const UnitHealth unit_health;
    const unsigned int* after_hit_increment_quantity_pointer;
    unsigned int quantity;
} InactiveUnitStack;

#endif
