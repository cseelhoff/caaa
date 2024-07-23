#include "inactive_unit_stack.h"
#include "unit_health.h"
#include <stdint.h>
#include <stdlib.h>

InactiveUnitStack* create_inactive_unit_stacks(const UnitHealth* unitHealths,
                                               int unitHealths_count,
                                               int inactive_unit_stacks_count) {
  InactiveUnitStack* inactive_unit_stacks =
      malloc(sizeof(InactiveUnitStack) * inactive_unit_stacks_count);
  int inactive_unit_stacks_index = 0;
  for (int i = 0; i < unitHealths_count; i++) {
    for (int j = 0; j < unitHealths[i].unit_type->max_land; j++) {
      inactive_unit_stacks[inactive_unit_stacks_index].unit_health = unitHealths[i];
      inactive_unit_stacks[inactive_unit_stacks_index].quantity = 0;
      inactive_unit_stacks_index++;
    }
  }
  return inactive_unit_stacks;
}