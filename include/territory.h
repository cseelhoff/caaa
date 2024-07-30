#ifndef TERRITORY_H
#define TERRITORY_H

#include "config.h"
#include "player.h"
#include <stdint.h>

struct Connection;

typedef struct Territory {
  char* name;
//  unsigned int territory_index;
  unsigned int original_owner_index;
  unsigned int owner_index;
//  MobileUnits* mobile_units;      //[TOTAL_ACTIVE_UNIT_STATUS_TYPES];
//  UnitHealths* inactive_stacks;   //[TOTAL_PLAYERS]
                                        //[TOTAL_INACTIVE_UNIT_STATUS_TYPES];
  struct Connection* water_connections[MAX_WATER_CONNECTIONS];
  struct Territory* connected_land_territories[MAX_LAND_CONNECTIONS];
  uint8_t water_conn_count;
  uint8_t land_conn_count;
  uint8_t land_value;
  uint8_t factory_max;
  uint8_t factory_hp;
  uint8_t builds_left;
  bool is_owned_by_current_player;
  bool is_ally_owned;
  bool newly_conquered;
  bool is_water;
} Territory;

Territory Territories[TERRITORIES_COUNT];

void loadTerritoriesFromJson(char* json_path);
Territory* getTerritoryByName(char* name);

#endif
