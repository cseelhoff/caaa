#ifndef INACTIVE_UNIT_STACK_H
#define INACTIVE_UNIT_STACK_H

#include "unit_health.h"

typedef struct InactiveUnitStack {
  const UnitHealth unit_health;
  uint8_t* after_hit_increment_quantity_pointer;
  uint8_t quantity;
} InactiveUnitStack;

#endif
