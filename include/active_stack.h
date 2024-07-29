#ifndef ACTIVE_STACKS_H
#define ACTIVE_STACKS_H

#include "player.h"
#include "unit_health.h"
#include "territory.h"
#include <stdint.h>

typedef struct {
  const UnitHealth* unitHealth;
  const Player* owner;
//  uint8_t* after_move_increment_quantity_pointer;
//  uint8_t* after_hit_increment_quantity_pointer;
//  uint8_t* after_turn_increment_quantity_pointer;
  unsigned int unit_quantity;
} ActiveUnitStack;

typedef struct {
  ActiveUnitStack* stacks;
  unsigned int stack_quantity;
} ActiveStacks;

ActiveStacks createActiveStacks(const Players players, const UnitHealths unitHealths);

#endif
