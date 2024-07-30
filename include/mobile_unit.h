#ifndef MOBILE_UNIT_H
#define MOBILE_UNIT_H

#include "unit_health.h"
#include <stdint.h>

typedef struct  {
  const UnitHealth* unit_health;
  uint8_t moves_remaining;
  //UnitStatus* unitStatusAfterMove;
} MobileUnit;

typedef struct {
  MobileUnit* array;
  int count;
} MobileUnits;

MobileUnits createMobileUnits(const UnitHealths unitHealths);

#endif
