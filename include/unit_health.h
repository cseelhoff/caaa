#ifndef UNIT_HEALTH_H
#define UNIT_HEALTH_H

#include "unit_type.h"
#include <stdint.h>

typedef struct {
  const UnitType* unit_type;
  uint8_t hits_remaining;
} UnitHealth;

typedef struct {
  UnitHealth* array;
  int count;
} UnitHealths;

UnitHealths createUnitHealths(const UnitTypes unitTypes);

#endif
