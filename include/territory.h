#ifndef TERRITORY_H
#define TERRITORY_H

#include "active_unit_stack.h"
#include "connection.h"
#include "inactive_unit_stack.h"
#include "player.h"
#include <stdint.h>

#define TOTAL_ACTIVE_UNIT_STATUS_TYPES 1
#define TOTAL_INACTIVE_UNIT_STATUS_TYPES 1
#define TOTAL_PLAYERS 2

typedef struct Territory {
  const char* name;
  const unsigned int territory_index;
  const uint8_t land_value;
  const bool is_water;
  const uint8_t water_connections_count;
  const Connection* water_connections[8];
  const uint8_t land_connections_count;
  const struct Territory* connected_land_territories[8];
  const Player* original_owner;
  Player* owner;
  bool is_owned_by_current_player;
  bool is_ally_owned;
  uint8_t factory_max;
  uint8_t factory_health;
  uint8_t construction_remaining;
  bool recently_conquered;
  ActiveUnitStack my_unit_stacks[TOTAL_ACTIVE_UNIT_STATUS_TYPES];
  InactiveUnitStack inactive_armies[TOTAL_PLAYERS - 1]
                                   [TOTAL_INACTIVE_UNIT_STATUS_TYPES];
} Territory;

#endif
