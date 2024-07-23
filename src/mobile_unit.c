#include "unit_status.h"
#include <stdio.h>
#include <stdlib.h>

UnitStatus* create_unit_statuses(const UnitHealth* unitHealths, int unitHealths_count,
                                 int unitStatuses_count) {
  UnitStatus* unitStatuses = malloc(unitStatuses_count * sizeof(UnitStatus));
  if (unitStatuses == NULL) {
    printf("Failed to allocate memory for unit statuses\n");
    return NULL;
  }

  for (int i = 0; i < unitHealths_count; i++) {
    unitStatuses[i].unit_health = &unitHealths[i];
    unitStatuses[i].moves_remaining = unitHealths[i].unit_type->max_moves;
  }

  return unitStatuses;
}