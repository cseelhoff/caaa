

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
    u.attack = getJsonChar(unit, "attack", DEF_ATTACK);
    u.defense = getJsonChar(unit, "defense", DEF_DEFENSE);
    u.max_moves = getJsonChar(unit, "max_moves", DEF_MAX_MOVES);
    u.cost = getJsonChar(unit, "cost", DEF_COST);
    u.bonus_attack = getJsonChar(unit, "bonus_attack", DEF_BONUS_ATTACK);
    u.gives_bonus = getJsonChar(unit, "gives_bonus", DEF_GIVES_BONUS);
    u.weight = getJsonChar(unit, "weight", DEF_WEIGHT);
    u.raid_strength = getJsonChar(unit, "raid_strength", DEF_RAID_STRENGTH);
    u.max_land = getJsonChar(unit, "max_land", DEF_MAX_LAND);
    u.max_air = getJsonChar(unit, "max_air", DEF_MAX_AIR);
    u.sub_strength = getJsonChar(unit, "sub_strength", DEF_SUB_STRENGTH);
    u.bombard = getJsonChar(unit, "bombard", DEF_BOMBARD);
    u.aa_shots = getJsonChar(unit, "aa_shots", DEF_AA_SHOTS);
    u.is_air = getJsonBool(unit, "is_air", DEF_IS_AIR);
    u.is_water = getJsonBool(unit, "is_water", DEF_IS_WATER);
    u.is_detector = getJsonBool(unit, "is_detector", DEF_IS_DETECTOR);
    index++;
  }
  return unitTypes;
}
