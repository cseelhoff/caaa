#include "unit_type.h"
#include "json_data_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

UnitTypes getUnitTypesFromJson(char* json_path) {
  cJSON* unitTypes_cjson = loadJsonPath(json_path, "unitTypes");
  UnitTypes unitTypes = {};
  unitTypes.count = getJsonArrayLength(unitTypes_cjson);
  if (unitTypes.count == 0) {
    printf("No unit types found\n");
    cJSON_Delete(unitTypes_cjson);
    return unitTypes;
  }
  unitTypes.array = malloc(unitTypes.count * sizeof(UnitType));
  if (!unitTypes.array) {
    printf("Memory allocation failed\n");
    cJSON_Delete(unitTypes_cjson);
    return unitTypes;
  }
  int index = 0;
  cJSON* cjson;
  cJSON_ArrayForEach(cjson, unitTypes_cjson) {
    UnitType u = u;
    u.name = getJsonString(cjson, "name", DEF_UT_NAME);
    u.attack = getJsonUint8_t(cjson, "attack", DEF_ATTACK);
    u.defense = getJsonUint8_t(cjson, "defense", DEF_DEFENSE);
    u.max_moves = getJsonUint8_t(cjson, "max_moves", DEF_MAX_MOVES);
    u.cost = getJsonUint8_t(cjson, "cost", DEF_COST);
    u.bonus_attack = getJsonUint8_t(cjson, "bonus_attack", DEF_BONUS_ATTACK);
    u.gives_bonus = getJsonUint8_t(cjson, "gives_bonus", DEF_GIVES_BONUS);
    u.weight = getJsonUint8_t(cjson, "weight", DEF_WEIGHT);
    u.raid_strength = getJsonUint8_t(cjson, "raid_strength", DEF_RAID_STRENGTH);
    u.max_land = getJsonUint8_t(cjson, "max_land", DEF_MAX_LAND);
    u.max_air = getJsonUint8_t(cjson, "max_air", DEF_MAX_AIR);
    u.sub_strength = getJsonUint8_t(cjson, "sub_strength", DEF_SUB_STRENGTH);
    u.bombard = getJsonUint8_t(cjson, "bombard", DEF_BOMBARD);
    u.aa_shots = getJsonUint8_t(cjson, "aa_shots", DEF_AA_SHOTS);
    u.is_air = getJsonBool(cjson, "is_air", DEF_IS_AIR);
    u.is_water = getJsonBool(cjson, "is_water", DEF_IS_WATER);
    u.is_detector = getJsonBool(cjson, "is_detector", DEF_IS_DETECTOR);
    index++;
  }

  cJSON_Delete(unitTypes_cjson);
  return unitTypes;
}
