#ifndef UNIT_STATUS_H
#define UNIT_STATUS_H

#include "unit_health.h"
#include <stdint.h>

typedef struct UnitStatus {
  const UnitHealth unit_health;
  const uint8_t moves_remaining;
} UnitStatus;

void unit_status_init(UnitStatus* status, UnitHealth unit_health,
                      uint8_t moves_remaining);

void create_unit_statuses(UnitHealth* unit_healths, int unit_healths_count,
                          UnitStatus* moved_unit_statuses,
                          int* moved_unit_statuses_count);

#endif
