#include "territory.h"
#include "json_data_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Territory* getJsonTerritories(cJSON* ters_cjson, int ters_count,
                              Player* players, int player_count) {
  // Allocate memory for the array of Territory structures
  Territory* territories = malloc(ters_count * sizeof(Territory));
  if (!territories) {
    printf("Memory allocation failed\n");
    cJSON_Delete(ters_cjson);
    return territories;
  }
  int index = 0;
  cJSON* ter;
  cJSON_ArrayForEach(ter, ters_cjson) {
    Territory t = t;
    t.name = getJsonString(ter, "name", DEF_TER_NAME);
    t.territory_index = index;
    char* original_owner = getJsonString(ter, "original_owner", NULL);
    t.original_owner = getPlayerByName(players, player_count, original_owner);
    char* owner = getJsonString(ter, "owner", NULL);
    t.owner = getPlayerByName(players, player_count, owner);
    // ActiveUnitStack* my_unit_stacks
    // InactiveUnitStack* inactive_armies
    // Connection* water_connections
    // Territory* connected_land_territories
    t.land_value = getJsonUint8_t(ter, "land_value", DEF_LAND_VALUE);
    t.factory_max = getJsonUint8_t(ter, "factory_max", DEF_FACTORY_MAX);
    t.factory_hp = getJsonUint8_t(ter, "factory_hp", DEF_FACTORY_HP);
    t.builds_left = getJsonUint8_t(ter, "builds_left", DEF_BUILDS_LEFT);
    // bool is_owned_by_current_player;
    // bool is_ally_owned;
    t.newly_conquered = getJsonBool(ter, "newly_conquered", DEF_CONQUERED);
    t.is_water = getJsonBool(ter, "is_water", DEF_IS_WATER);
    index++;
  }
  return territories;
}

Territory* getTerritoryByName(Territory* territories, int t_count, char* name) {
  for (int i = 0; i < t_count; i++) {
    if (strcmp(territories[i].name, name) == 0) {
      return &territories[i];
    }
  }
  printf("Territory name: %s not found\n", name);
  return NULL;
}
