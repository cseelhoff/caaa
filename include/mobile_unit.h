#ifndef MOBILE_UNIT_H
#define MOBILE_UNIT_H

#include "config.h"
#include "unit_health.h"
#include <stdint.h>

typedef struct  {
  const UnitHealth* unit_health;
  uint8_t moves_remaining;
  //UnitStatus* unitStatusAfterMove;
} MobileUnit;

MobileUnit MobileUnits[MOBILE_UNITS_COUNT];
void createMobileUnits();

#endif
