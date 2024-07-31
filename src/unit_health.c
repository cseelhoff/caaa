#include "unit_health.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>

void createUnitHealths() {
  unsigned int unitHealthsCount = 0;
  for (int i = 0; i < UNIT_TYPES_COUNT; i++) {
    unitHealthsCount += UnitTypes[i].max_hits;
  }
  if (unitHealthsCount != UNIT_HEALTHS_COUNT) {
    printf("Error: unitHealthsCount (%d) != UNIT_HEALTHS_COUNT (%d)\n",
           unitHealthsCount, UNIT_HEALTHS_COUNT);
    exit(1);
  }
  int index = 0;
  for (int i = 0; i < UNIT_TYPES_COUNT; i++) {
    for (int j = UnitTypes[i].max_hits; j > 0; j--) {
      UnitHealths[index].unit_type = UnitTypes[i];
      UnitHealths[index].hits_remaining = j;
      index++;
    }
  }
}