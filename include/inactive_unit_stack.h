#ifndef INACTIVE_UNIT_STACK_H
#define INACTIVE_UNIT_STACK_H

#include "unit_status.h"

typedef struct InactiveUnitStack {
  UnitHealth unit_health;
  uint8_t* after_hit_increment_quantity_pointer;
  uint8_t quantity;
} InactiveUnitStack;

typedef struct {
  InactiveUnitStack* array;
  int count;
} InactiveUnitStacks;

InactiveUnitStack* createInactiveUnitStacks(const UnitHealth* unitHealths,
                                               int unitHealths_count,
                                               int inactive_unit_stacks_count);

#endif
