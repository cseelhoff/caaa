#ifndef UNIT_STATUS_H
#define UNIT_STATUS_H

#include "unit_health.h"
#include <stdint.h>

typedef struct UnitStatus {
  const UnitHealth* unit_health;
  uint8_t moves_remaining;
  //UnitStatus* unitStatusAfterMove;
} UnitStatus;

UnitStatus* create_unit_statuses(const UnitHealth* unitHealths, int unitHealths_count,
                                 int unitStatuses_count);

#endif
