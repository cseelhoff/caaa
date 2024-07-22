#ifndef ACTIVE_UNIT_STACK_H
#define ACTIVE_UNIT_STACK_H

#include "player.h"
#include "unit_status.h"
#include <stdint.h>

typedef struct ActiveUnitStack {
  const UnitStatus* unit_status;
  const Player* owner;
  const unsigned int* after_move_increment_quantity_pointer;
  const unsigned int* after_hit_increment_quantity_pointer;
  const unsigned int* after_turn_increment_quantity_pointer;
  unsigned int unit_quantity;
} ActiveUnitStack;

void active_unit_stack_init(ActiveUnitStack* stack, UnitStatus unit_status,
                            Player owner, uint8_t unit_quantity);

void create_unit_stacks(Player* players, int players_count,
                        UnitStatus* unmoved_unit_statuses, int unmoved_count,
                        UnitStatus* moved_unit_statuses, int moved_count,
                        ActiveUnitStack** unmoved_unit_stacks,
                        int* unmoved_stacks_count,
                        ActiveUnitStack** moved_unit_stacks,
                        int* moved_stacks_count);

#endif
