#include <stddef.h>
#include "unit_health.h"

void unit_health_init(UnitHealth* health, UnitType unit_type, unsigned char hits_remaining) {
    if (health != NULL) {
        health->unit_type = unit_type;
        health->hits_remaining = hits_remaining;
    }
}

void create_unit_healths(UnitType* unit_types, int unit_types_count, UnitHealth* inactive_unit_healths, int* inactive_unit_healths_count) {
    int index = 0;
    for (int i = 0; i < unit_types_count; ++i) {
        UnitType current_type = unit_types[i];
        for (unsigned char hits_remaining = 1; hits_remaining <= current_type.max_hits; ++hits_remaining) {
            if (index < *inactive_unit_healths_count) {
                unit_health_init(&inactive_unit_healths[index], current_type, hits_remaining);
                ++index;
            }
        }
    }
    *inactive_unit_healths_count = index;
}