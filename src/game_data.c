#include "game_data.h"
#include "cJSON.h"
#include "unittype.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

// Existing gameDataInstance definition
GameData gameDataInstance = {0};

void createDummyGameState() {
  int dummyGameState[] = {
      0, // ActiveUnitStatus stat 1 for Territory1
      0, // ActiveUnitStatus stat 2 for Territory1

      0, // Player Money
      0, // Current Player is allied with Next Player T/F
      0, // Territory1 (CurrentPlayer perspective stat1)
      0, // Territory1 (CurrentPlayer perspective stat2)
      0, // Territory2 (CurrentPlayer perspective stat1)
      0, // Territory2 (CurrentPlayer perspective stat2)

      0, // NextPlayer Money
      0, // NextPlayer is allied with Next Player T/F
      0, // Territory1 (NextPlayer perspective stat1)
      0, // Territory1 (NextPlayer perspective stat2)
      0, // Territory2 (NextPlayer perspective stat1)
      0, // Territory2 (NextPlayer perspective stat2)
  };
}

GameData* initializeGameData(int a) {
  // print out current directory
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("Current directory: %s\n", cwd);
  } else {
    printf("Failed to get current directory\n");
    return &gameDataInstance;
  }

  cJSON* cjson = loadJsonPath("./data/unit_types.json", "unitTypes");
  int unitTypes_count = get_count_from_cjson_array(cjson);
  if (unitTypes_count == 0) {
    printf("No unit types found\n");
    return &gameDataInstance;
  }
  const UnitType* unitTypes = get_unitTypes_from_cjson(cjson, unitTypes_count);

  int unitHealths_count = 0;
  for (int i = 0; i < unitTypes_count; i++) {
    unitHealths_count += unitTypes[i].max_hits;
  }
  return &gameDataInstance;
}

cJSON* loadJsonPath(char* path, char* string) {
  cJSON* cjson;
  char* jsonString = readFileToString(path);
  if (jsonString == NULL) {
    printf("Failed to read unit types JSON file\n");
    return cjson;
  }

  cjson = cJSON_Parse(jsonString);
  free(jsonString);
  if (cjson == NULL) {
    printf("Failed to parse unit types JSON\n");
    return cjson;
  }

  cjson = cJSON_GetObjectItemCaseSensitive(cjson, string);
  if (!cJSON_IsArray(cjson)) {
    printf("%s is not an array\n", string);
  }
  return cjson;
}

char* readFileToString(const char* filename) {
  FILE* file = fopen(filename, "rb");
  if (file == NULL)
    return NULL;

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char* content = (char*)malloc(length + 1);
  if (content) {
    fread(content, 1, length, file);
    content[length] = '\0';
  }

  fclose(file);
  return content;
}

int get_count_from_cjson_array(cJSON* cjson_array) {
  int count = cJSON_GetArraySize(cjson_array);
  if (count == 0) {
    printf("No items found in array\n");
    cJSON_Delete(cjson_array);
  }
  return count;
}

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

char* getJsonString(cJSON* cjson, char* key, char* default_value) {
  char* value = default_value;
  cJSON* value_cjson = cJSON_GetObjectItemCaseSensitive(cjson, key);
  if (value_cjson && cJSON_IsString(value_cjson))
    value = strdup(value_cjson->valuestring);
  return value;
}

uint8_t getJsonChar(cJSON* cjson, char* key, uint8_t default_val) {
  uint8_t value = default_val;
  cJSON* value_cjson = cJSON_GetObjectItemCaseSensitive(cjson, key);
  if (value_cjson && cJSON_IsNumber(value_cjson))
    value = value_cjson->valueint;
  return value;
}

bool getJsonBool(cJSON* cjson, char* key, bool default_value) {
  bool value = default_value;
  cJSON* value_cjson = cJSON_GetObjectItemCaseSensitive(cjson, key);
  if (value_cjson)
    value = cJSON_IsTrue(value_cjson);
  return value;
}
