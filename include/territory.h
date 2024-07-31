#ifndef TERRITORY_H
#define TERRITORY_H

#include "config.h"
#include "player.h"
#include <stdint.h>

struct Connection;

typedef struct Territory {
  char* name;
  unsigned int original_owner_index;
  unsigned int owner_index;
  unsigned int connected_sea_index[MAX_LAND_TO_SEA_CONNECTIONS];
  unsigned int connected_land_index[MAX_LAND_TO_LAND_CONNECTIONS];
  uint8_t sea_conn_count;
  uint8_t land_conn_count;
  uint8_t land_value;
  uint8_t factory_max;
  uint8_t factory_hp;
  uint8_t builds_left;
  bool is_owned_by_current_player;
  bool is_ally_owned;
  bool newly_conquered;
} Territory;

Territory* getTerritoryByName(char* name);

#endif
