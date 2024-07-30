#include "territory.h"
#include "json_data_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Territories getJsonTerritories(char* json_path, Players players) {
  cJSON* ters_cjson = loadJsonPath(json_path, "territories");
  Territories territories = {};
  territories.count = getJsonArrayLength(ters_cjson);

  if (territories.count == 0) {
    printf("No territories found\n");
    cJSON_Delete(ters_cjson);
    return territories;
  }

  territories.array = malloc(territories.count * sizeof(Territory));
  if (!territories.array) {
    printf("Memory allocation failed\n");
    cJSON_Delete(ters_cjson);
    return territories;
  }

  int index = 0;
  cJSON* cjson;
  cJSON_ArrayForEach(cjson, ters_cjson) {
    Territory t = t;
    t.name = getJsonString(cjson, "name", DEF_TER_NAME);
    t.territory_index = index;
    char* original_owner = getJsonString(cjson, "original_owner", NULL);
    t.original_owner = getPlayerByName(players, original_owner);
    char* owner = getJsonString(cjson, "owner", NULL);
    t.owner = getPlayerByName(players, owner);
    // ActiveUnitStack* my_unit_stacks
    // InactiveUnitStack* inactive_armies
    // Connection* water_connections
    // Territory* connected_land_territories
    t.land_value = getJsonUint8_t(cjson, "land_value", DEF_LAND_VALUE);
    t.factory_max = getJsonUint8_t(cjson, "factory_max", DEF_FACTORY_MAX);
    t.factory_hp = getJsonUint8_t(cjson, "factory_hp", DEF_FACTORY_HP);
    t.builds_left = getJsonUint8_t(cjson, "builds_left", DEF_BUILDS_LEFT);
    // bool is_owned_by_current_player;
    // bool is_ally_owned;
    t.newly_conquered = getJsonBool(cjson, "newly_conquered", DEF_CONQUERED);
    t.is_water = getJsonBool(cjson, "is_water", DEF_IS_WATER);
    index++;
  }
  return territories;
}

Territory* getTerritoryByName(Territories territories, char* name) {
  for(int i = 0; i < territories.count; i++) {
    if (strcmp(territories.array[i].name, name) == 0) {
      return &territories.array[i];
    }
  }
  printf("Territory name: %s not found\n", name);
  return NULL;
}
