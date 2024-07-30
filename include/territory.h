#ifndef TERRITORY_H
#define TERRITORY_H

#include "mobile_unit.h"
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
  Player* original_owner;
  Player* owner;
//  MobileUnits* mobile_units;      //[TOTAL_ACTIVE_UNIT_STATUS_TYPES];
//  UnitHealths* inactive_stacks;   //[TOTAL_PLAYERS]
                                        //[TOTAL_INACTIVE_UNIT_STATUS_TYPES];
  struct Connection* water_connections; //[8];
  struct Territory* connected_land_territories; //[8];
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

typedef struct {
  Territory* array;
  unsigned int count;
} Territories;

Territories getTerritoriesFromJson(char* json_path, Players players);
Territory* getTerritoryByName(Territories territories, char* name);

#endif
