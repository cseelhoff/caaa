#include "game_data.h"
#include "cJSON.h"
#include "unit_type.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Add this line to include the header file for getcwd function

// Existing gameDataInstance definition
GameData gameDataInstance = {0};

void createDummyGameState() {
  int dummyGameState[] = {
      0, // Current Player is allied with Next Player T/F
      0, // Player Money
      0, // Next Player is allied with Next,Next Player T/F
      0, // Next Player Money
      0, // Territory1 Land Value
      0, // T1 is water T/F
      0, // T1 original owner (player_index offset from current - unsign char)
      0, //
  };
}

// Function to read the entire content of a file into a string
char *readFileToString(const char *filename) {
  FILE *file = fopen(filename, "rb");
  if (file == NULL)
    return NULL;

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *content = (char *)malloc(length + 1);
  if (content) {
    fread(content, 1, length, file);
    content[length] = '\0'; // Null-terminate the string
  }

  fclose(file);
  return content;
}

// Modified initializeGameData function
GameData *initializeGameData(int a) {
  // print out current directory
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("Current directory: %s\n", cwd);
  } else {
    printf("Failed to get current directory\n");
    return &gameDataInstance;
  }
  UnitType* unitTypes = get_unitTypes("./data/unit_types.json");
  return &gameDataInstance;
}

UnitType* get_unitTypes(char *unit_types_json_path) {
  UnitType *unitTypes;
  char *jsonString = readFileToString(unit_types_json_path);
  if (jsonString == NULL) {
    printf("Failed to read unit types JSON file\n");
    return unitTypes;
  }

  cJSON *json = cJSON_Parse(jsonString);
  if (json == NULL) {
    printf("Failed to parse unit types JSON\n");
    free(jsonString);
    return unitTypes;
  }

  cJSON *unitTypesArray = cJSON_GetObjectItemCaseSensitive(json, "unitTypes");
  if (!cJSON_IsArray(unitTypesArray)) {
    printf("unitTypes is not an array\n");
    cJSON_Delete(json);
    free(jsonString);
    return unitTypes;
  }

  int unitTypeCount = cJSON_GetArraySize(unitTypesArray);
  if (unitTypeCount == 0) {
    printf("No unit types found\n");
    cJSON_Delete(json);
    free(jsonString);
    return unitTypes;
  }

  // Allocate memory for the array of UnitType structures
  unitTypes = malloc(unitTypeCount * sizeof(UnitType));
  if (!unitTypes) {
    printf("Memory allocation failed\n");
    cJSON_Delete(json);
    free(jsonString);
    return unitTypes;
  }

  cJSON *unitType_cjson;
  cJSON_ArrayForEach(unitType_cjson, unitTypesArray) {
    cJSON *name = cJSON_GetObjectItemCaseSensitive(unitType_cjson, "name");
    if (!name || !cJSON_IsString(name))
      break;

    cJSON *attack = cJSON_GetObjectItemCaseSensitive(unitType_cjson, "attack");
    cJSON *defense =
        cJSON_GetObjectItemCaseSensitive(unitType_cjson, "defense");
    cJSON *max_moves =
        cJSON_GetObjectItemCaseSensitive(unitType_cjson, "max_moves");
    cJSON *max_hits =
        cJSON_GetObjectItemCaseSensitive(unitType_cjson, "max_hits");
    cJSON *cost = cJSON_GetObjectItemCaseSensitive(unitType_cjson, "cost");
    cJSON *max_supported =
        cJSON_GetObjectItemCaseSensitive(unitType_cjson, "max_supported");
    cJSON *weight = cJSON_GetObjectItemCaseSensitive(unitType_cjson, "weight");

    UnitType unitType = {strdup(name->valuestring),
                         attack->valueint,
                         defense->valueint,
                         max_moves->valueint,
                         max_hits->valueint,
                         cost->valueint,
                         0,
                         max_supported->valueint,
                         weight->valueint,
                         false,
                         0,
                         false,
                         0,
                         0,
                         0,
                         0,
                         0,
                         0};
  }

  cJSON_Delete(json);
  free(jsonString);

  return unitTypes;
}
