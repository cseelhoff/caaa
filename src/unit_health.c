#include "unit_health.h"
#include <stdio.h>
#include <stdlib.h>

UnitHealths createUnitHealths(const UnitTypes unitTypes) {
  UnitHealths unitHealths = {};
  for (int i = 0; i < unitTypes.count; i++) {
    unitHealths.count += unitTypes.array[i].max_hits;
  }
  unitHealths.array = malloc(unitHealths.count * sizeof(UnitHealth));
  if (unitHealths.array == NULL) {
    printf("Failed to allocate memory for unit healths\n");
    return unitHealths;
  }
  // UnitHealth* graveyard = malloc(sizeof(UnitHealth));
  int index = 0;
  for (int i = 0; i < unitTypes.count; i++) {
    // UnitHealth* unitHealthAfterHit = graveyard;
    for (int j = 0; j < unitTypes.array[i].max_hits; j++) {
      unitHealths.array[index].unit_type = &unitTypes.array[i];
      unitHealths.array[index].hits_remaining = j;
      // unitHealths[unitHealthsIndex].unitHealthAfterHit = unitHealthAfterHit;
      index++;
      // unitHealthAfterHit = &unitHealths[unitHealthsIndex];
    }
  }

  return unitHealths;
}