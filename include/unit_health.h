#ifndef UNIT_HEALTH_H
#define UNIT_HEALTH_H

#include "config.h"
#include "unit_type.h"
#include <stdint.h>

typedef struct {
  const UnitType* unit_type;
  uint8_t hits_remaining;
} UnitHealth;

UnitHealth UnitHealths[UNIT_HEALTHS_COUNT];
void loadUnitHealths();

#endif
