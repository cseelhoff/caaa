#ifndef UNIT_HEALTH_H
#define UNIT_HEALTH_H

#include "unit_type.h" // Assuming this is defined based on your Rust `UnitType` enum or struct

typedef struct UnitHealth {
    UnitType unit_type;
    unsigned char hits_remaining;
} UnitHealth;

// Function to initialize a UnitHealth
void unit_health_init(UnitHealth* health, UnitType unit_type, unsigned char hits_remaining);

// Function to create an array of UnitHealth from an array of UnitType
// Note: The caller must ensure there is enough space in `inactive_unit_healths` based on the sum of `max_hits` for each `unit_types`
void create_unit_healths(UnitType* unit_types, int unit_types_count, UnitHealth* inactive_unit_healths, int* inactive_unit_healths_count);

#endif // UNIT_HEALTH_H