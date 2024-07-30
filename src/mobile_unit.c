#include "mobile_unit.h"
#include "unit_health.h"
#include <stdio.h>
#include <stdlib.h>

MobileUnits createMobileUnits(const UnitHealths unitHealths) {
  MobileUnits mobileUnits;
  mobileUnits.count = 0;
  for (int i = 0; i < unitHealths.count; i++) {
    mobileUnits.count += unitHealths.array[i].unit_type->max_moves;
  }
  mobileUnits.array = malloc(sizeof(MobileUnit) * mobileUnits.count);
  if (!mobileUnits.array) {
    printf("Memory allocation failed\n");
    return mobileUnits;
  }
  int index = 0;
  for (int i = 0; i < unitHealths.count; i++) {
    for (int j = 0; j < unitHealths.array[i].unit_type->max_moves; j++) {
      MobileUnit mu = mu;
      mu.unit_health = &unitHealths.array[i];
      mu.moves_remaining = j;
      mobileUnits.array[index] = mu;
      index++;
    }
  }
  
  return mobileUnits;
}