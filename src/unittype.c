#include "unittype.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "json_data_loader.h"

UnitType* get_unitTypes_from_cjson(cJSON* unitTypes_cjson, int unitType_count) {
  // Allocate memory for the array of UnitType structures
  UnitType* unitTypes = malloc(unitType_count * sizeof(UnitType));
  if (!unitTypes) {
    printf("Memory allocation failed\n");
    cJSON_Delete(unitTypes_cjson);
    return unitTypes;
  }
  int index = 0;
  cJSON* unit;
  cJSON_ArrayForEach(unit, unitTypes_cjson) {
    UnitType u = u;
    u.name = getJsonString(unit, "name", DEF_UT_NAME);
    u.attack = getJsonUint8_t(unit, "attack", DEF_ATTACK);
    u.defense = getJsonUint8_t(unit, "defense", DEF_DEFENSE);
    u.max_moves = getJsonUint8_t(unit, "max_moves", DEF_MAX_MOVES);
    u.cost = getJsonUint8_t(unit, "cost", DEF_COST);
    u.bonus_attack = getJsonUint8_t(unit, "bonus_attack", DEF_BONUS_ATTACK);
    u.gives_bonus = getJsonUint8_t(unit, "gives_bonus", DEF_GIVES_BONUS);
    u.weight = getJsonUint8_t(unit, "weight", DEF_WEIGHT);
    u.raid_strength = getJsonUint8_t(unit, "raid_strength", DEF_RAID_STRENGTH);
    u.max_land = getJsonUint8_t(unit, "max_land", DEF_MAX_LAND);
    u.max_air = getJsonUint8_t(unit, "max_air", DEF_MAX_AIR);
    u.sub_strength = getJsonUint8_t(unit, "sub_strength", DEF_SUB_STRENGTH);
    u.bombard = getJsonUint8_t(unit, "bombard", DEF_BOMBARD);
    u.aa_shots = getJsonUint8_t(unit, "aa_shots", DEF_AA_SHOTS);
    u.is_air = getJsonBool(unit, "is_air", DEF_IS_AIR);
    u.is_water = getJsonBool(unit, "is_water", DEF_IS_WATER);
    u.is_detector = getJsonBool(unit, "is_detector", DEF_IS_DETECTOR);
    index++;
  }
  return unitTypes;
}
