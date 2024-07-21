#ifndef UNIT_HEALTH_H
#define UNIT_HEALTH_H

#include "unit_type.h"

typedef struct UnitHealth {
    const UnitType unit_type;
    const unsigned char hits_remaining;
} UnitHealth;

void unit_health_init(UnitHealth* health, UnitType unit_type, unsigned char hits_remaining);

void create_unit_healths(UnitType* unit_types, int unit_types_count, UnitHealth* inactive_unit_healths, int* inactive_unit_healths_count);

#endif
