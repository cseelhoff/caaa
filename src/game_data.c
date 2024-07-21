#include "game_data.h"
#include "cJSON.h"
#include "unittype.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Add this line to include the header file for getcwd function

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

  cJSON* cjson = get_cjson_from_path("./data/unit_types.json", "unitTypes");
  int unitTypes_count = get_count_from_cjson_array(cjson);
  if (unitTypes_count == 0)
    return &gameDataInstance;
  UnitType* unitTypes = get_unitTypes_from_cjson(cjson, unitTypes_count);
  return &gameDataInstance;
}

cJSON* get_cjson_from_path(char* unit_types_json_path,
                           char* object_item_string) {
  cJSON* cJSON_ptr;
  char* jsonString = readFileToString(unit_types_json_path);
  if (jsonString == NULL) {
    printf("Failed to read unit types JSON file\n");
    return cJSON_ptr;
  }

  cJSON_ptr = cJSON_Parse(jsonString);
  free(jsonString);
  if (cJSON_ptr == NULL) {
    printf("Failed to parse unit types JSON\n");
    return cJSON_ptr;
  }

  cJSON_ptr = cJSON_GetObjectItemCaseSensitive(cJSON_ptr, object_item_string);
  if (!cJSON_IsArray(cJSON_ptr)) {
    printf("%s is not an array\n", object_item_string);
  }
  return cJSON_ptr;
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
    unitTypes[index].name = get_string_from_cjson(unit, "name", UT_DEF_NAME);
    unitTypes[index].attack =
        get_char_from_cjson(unit, "attack", UT_DEF_ATTACK);
    unitTypes[index].defense =
        get_char_from_cjson(unit, "defense", UT_DEF_DEFENSE);
    unitTypes[index].max_moves =
        get_char_from_cjson(unit, "max_moves", UT_DEF_MAX_MOVES);
    unitTypes[index].cost = get_char_from_cjson(unit, "cost", UT_DEF_COST);
    unitTypes[index].max_supportable =
        get_char_from_cjson(unit, "max_supportable", UT_DEF_MAX_SUPPORTABLE);
    unitTypes[index].max_supported =
        get_char_from_cjson(unit, "max_supported", UT_DEF_MAX_SUPPORTED);
    unitTypes[index].weight =
        get_char_from_cjson(unit, "weight", UT_DEF_WEIGHT);
    unitTypes[index].raid_strength =
        get_char_from_cjson(unit, "raid_strength", UT_DEF_RAID_STRENGTH);
    unitTypes[index].max_land =
        get_char_from_cjson(unit, "max_land", UT_DEF_MAX_LAND);
    unitTypes[index].max_air =
        get_char_from_cjson(unit, "max_air", UT_DEF_MAX_AIR);
    unitTypes[index].sub_strength =
        get_char_from_cjson(unit, "sub_strength", UT_DEF_SUB_STRENGTH);
    unitTypes[index].bombard =
        get_char_from_cjson(unit, "bombard", UT_DEF_BOMBARD);
    unitTypes[index].aa_shots =
        get_char_from_cjson(unit, "aa_shots", UT_DEF_AA_SHOTS);
    unitTypes[index].is_air =
        get_bool_from_cjson(unit, "is_air", UT_DEF_IS_AIR);
    unitTypes[index].is_water =
        get_bool_from_cjson(unit, "is_water", UT_DEF_IS_WATER);
    unitTypes[index].is_detector =
        get_bool_from_cjson(unit, "is_detector", UT_DEF_IS_DETECTOR);
    index++;
  }
  return unitTypes;
}

char* get_string_from_cjson(cJSON* cjson, char* key, char* default_value) {
  char* value = default_value;
  cJSON* value_cjson = cJSON_GetObjectItemCaseSensitive(cjson, key);
  if (value_cjson && cJSON_IsString(value_cjson))
    value = strdup(value_cjson->valuestring);
  return value;
}

unsigned char get_char_from_cjson(cJSON* cjson, char* key,
                                  unsigned char default_value) {
  unsigned char value = default_value;
  cJSON* value_cjson = cJSON_GetObjectItemCaseSensitive(cjson, key);
  if (value_cjson && cJSON_IsNumber(value_cjson))
    value = value_cjson->valueint;
  return value;
}

bool get_bool_from_cjson(cJSON* cjson, char* key, bool default_value) {
  bool value = default_value;
  cJSON* value_cjson = cJSON_GetObjectItemCaseSensitive(cjson, key);
  if (value_cjson)
    value = cJSON_IsTrue(value_cjson);
  return value;
}
