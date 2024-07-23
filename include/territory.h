#ifndef TERRITORY_H
#define TERRITORY_H

#include "active_unit_stack.h"
#include "inactive_unit_stack.h"
#include "player.h"
#include <stdint.h>

#define DEF_TER_NAME "DEFAULT UNITTYPE"
#define DEF_LAND_VALUE 1
#define DEF_IS_WATER false
#define DEF_FACTORY_MAX 0
#define DEF_FACTORY_HP 0
#define DEF_BUILDS_LEFT 0
#define DEF_CONQUERED false

struct Connection;

typedef struct Territory {
  char* name;
  unsigned int territory_index;
  bool is_water;
  ActiveUnitStack* my_unit_stacks;      //[TOTAL_ACTIVE_UNIT_STATUS_TYPES];
  InactiveUnitStack* inactive_armies;   //[TOTAL_PLAYERS]
                                        //[TOTAL_INACTIVE_UNIT_STATUS_TYPES];
  struct Connection* water_connections; //[8];
  struct Territory* connected_land_territories; //[8];
  Player* original_owner;
  Player* owner;
  uint8_t land_conn_count;
  uint8_t water_conn_count;
  uint8_t land_value;
  uint8_t factory_max;
  uint8_t factory_hp;
  uint8_t builds_left;
  bool is_owned_by_current_player;
  bool is_ally_owned;
  bool newly_conquered;
} Territory;

Territory* getJsonTerritories(cJSON* ters_cjson, int ters_count,
                              Player* players, int player_count);
Territory* getTerritoryByName(Territory* territories, int territory_count,
                              char* dest_territory);

#endif
