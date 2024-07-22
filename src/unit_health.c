#include "unit_health.h"
#include <stdio.h>
#include <stdlib.h>

UnitHealth* create_unit_healths(const UnitType* unitTypes, int unitTypes_count,
                                int unitHealths_count) {
  UnitHealth* unitHealths = malloc(unitHealths_count * sizeof(UnitHealth));
  if (unitHealths == NULL) {
    printf("Failed to allocate memory for unit healths\n");
    return NULL;
  }
  //UnitHealth* graveyard = malloc(sizeof(UnitHealth));
  int unitHealthsIndex = 0;
  for (int i = 0; i < unitTypes_count; i++) {
    //UnitHealth* unitHealthAfterHit = graveyard;
    for (int j = 0; j < unitTypes[i].max_hits; j++) {
      unitHealths[unitHealthsIndex].unit_type = &unitTypes[i];
      unitHealths[unitHealthsIndex].hits_remaining = j;
      //unitHealths[unitHealthsIndex].unitHealthAfterHit = unitHealthAfterHit;
      unitHealthsIndex++;
      //unitHealthAfterHit = &unitHealths[unitHealthsIndex];
    }
  }

  return unitHealths;
}