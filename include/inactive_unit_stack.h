#ifndef INACTIVE_UNIT_STACK_H
#define INACTIVE_UNIT_STACK_H

#include "unit_health.h"

typedef struct InactiveUnitStack {
  UnitHealth unit_health;
  uint8_t* after_hit_increment_quantity_pointer;
  uint8_t quantity;
} InactiveUnitStack;

InactiveUnitStack* create_inactive_unit_stacks(const UnitHealth* unitHealths,
                                               int unitHealths_count,
                                               int inactive_unit_stacks_count);

#endif
