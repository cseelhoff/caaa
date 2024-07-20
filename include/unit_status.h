#ifndef UNIT_STATUS_H
#define UNIT_STATUS_H

#include "unit_health.h" // Assuming this is defined in unit_health.h

typedef struct UnitStatus {
    UnitHealth unit_health;
    unsigned char moves_remaining;
} UnitStatus;

// Function to initialize a UnitStatus
void unit_status_init(UnitStatus* status, UnitHealth unit_health, unsigned char moves_remaining);

// Function to create an array of UnitStatus from an array of UnitHealth
// Note: The caller must ensure there is enough space in `moved_unit_statuses` based on the sum of `moves` for each `unit_healths`
void create_unit_statuses(UnitHealth* unit_healths, int unit_healths_count, UnitStatus* moved_unit_statuses, int* moved_unit_statuses_count);

#endif // UNIT_STATUS_H