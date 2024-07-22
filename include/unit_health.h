#ifndef UNIT_HEALTH_H
#define UNIT_HEALTH_H

#include "unittype.h"
#include <stdint.h>

typedef struct UnitHealth {
  const UnitType* unit_type;
  uint8_t hits_remaining;
  //struct UnitHealth* unitHealthAfterHit;
  //uint8 cargo_options_enum_index = 9 options;
} UnitHealth;

UnitHealth* create_unit_healths(const UnitType* unitTypes, int unitTypes_count,
                                int unitHealths_count);

#endif
