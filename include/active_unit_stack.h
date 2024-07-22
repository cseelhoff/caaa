#ifndef ACTIVE_UNIT_STACK_H
#define ACTIVE_UNIT_STACK_H

#include "player.h"
#include "unit_status.h"
#include <stdint.h>

typedef struct ActiveUnitStack {
  const UnitStatus* unit_status;
  const Player* owner;
  uint8_t* after_move_increment_quantity_pointer;
  uint8_t* after_hit_increment_quantity_pointer;
  uint8_t* after_turn_increment_quantity_pointer;
  unsigned int unit_quantity;
} ActiveUnitStack;

#endif
