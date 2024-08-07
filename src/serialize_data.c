#include "serialize_data.h"
#include "cJSON.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void write_json_to_file(const char* filename, cJSON* json) {
  char* string = cJSON_Print(json);
  FILE* file = fopen(filename, "w");
  if (file == NULL) {
    perror("Failed to open file");
    free(string);
    return;
  }
  fprintf(file, "%s", string);
  fclose(file);
  free(string);
}

cJSON* read_json_from_file(const char* filename) {
  FILE* file = fopen(filename, "r");
  if (file == NULL) {
    perror("Failed to open file");
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char* data = (char*)malloc(length + 1);
  if (data == NULL) {
    perror("Failed to allocate memory");
    fclose(file);
    return NULL;
  }

  fread(data, 1, length, file);
  data[length] = '\0';
  fclose(file);

  cJSON* json = cJSON_Parse(data);
  free(data);

  if (json == NULL) {
    fprintf(stderr, "Failed to parse JSON\n");
  }

  return json;
}

void deserialize_game_data_from_json(GameData* data, cJSON* json) {
  if (json == NULL) {
    fprintf(stderr, "Invalid JSON data\n");
    return;
  }

  cJSON* player_index = cJSON_GetObjectItem(json, "player_index");
  if (cJSON_IsNumber(player_index)) {
    data->player_index = player_index->valueint;
  }

  cJSON* phase = cJSON_GetObjectItem(json, "phase");
  if (cJSON_IsNumber(phase)) {
    data->phase = phase->valueint;
  }

  cJSON* money_array = cJSON_GetObjectItem(json, "money");
  if (cJSON_IsArray(money_array)) {
    int array_size = cJSON_GetArraySize(money_array);
    for (int i = 0; i < array_size && i < PLAYERS_COUNT; i++) {
      cJSON* money_item = cJSON_GetArrayItem(money_array, i);
      if (cJSON_IsNumber(money_item)) {
        data->money[i] = money_item->valueint;
      }
    }
  }

  cJSON* land_state_array = cJSON_GetObjectItem(json, "land_state");
  if (cJSON_IsArray(land_state_array)) {
    int array_size = cJSON_GetArraySize(land_state_array);
    for (int i = 0; i < array_size && i < LANDS_COUNT; i++) {
      cJSON* land_state_item = cJSON_GetArrayItem(land_state_array, i);
      deserialize_land_state_from_json(&data->land_state[i], land_state_item);
    }
  }

  cJSON* units_sea_array = cJSON_GetObjectItem(json, "units_sea");
  if (cJSON_IsArray(units_sea_array)) {
    int array_size = cJSON_GetArraySize(units_sea_array);
    for (int i = 0; i < array_size && i < SEAS_COUNT; i++) {
      cJSON* units_sea_item = cJSON_GetArrayItem(units_sea_array, i);
      deserialize_units_sea_from_json(&data->units_sea[i], units_sea_item);
    }
  }
}
void deserialize_land_state_from_json(LandState* land_state, cJSON* json) {
  if (json == NULL) {
    fprintf(stderr, "Invalid JSON data\n");
    return;
  }
  cJSON* conquered = cJSON_GetObjectItem(json, "conquered");
  if (cJSON_IsBool(conquered)) {
    land_state->conquered = cJSON_IsTrue(conquered);
  }
  set_land_state_field(json, "owner_index", &land_state->owner_index);
  set_land_state_field(json, "builds_left", &land_state->builds_left);
  set_land_state_field(json, "factory_hp", &land_state->factory_hp);
  set_land_state_field(json, "factory_max", &land_state->factory_max);
  extract_and_assign(json, "fighters", land_state->fighters);
  extract_and_assign(json, "bombers", land_state->bombers);
  extract_and_assign(json, "infantry", land_state->infantry);
  extract_and_assign(json, "artillery", land_state->artillery);
  extract_and_assign(json, "tanks", land_state->tanks);
  extract_and_assign(json, "aa_guns", land_state->aa_guns);
}

void set_land_state_field(cJSON* json, const char* key, uint8_t* field) {
  cJSON* item = cJSON_GetObjectItem(json, key);
  if (cJSON_IsNumber(item)) {
    *field = item->valueint;
  }
}

void extract_and_assign(cJSON* json, const char* key, uint8_t* target_array) {
  cJSON* array = cJSON_GetObjectItem(json, key);
  if (cJSON_IsArray(array)) {
    int array_size = cJSON_GetArraySize(array);
    for (int i = 0; i < array_size; i++) {
      cJSON* item = cJSON_GetArrayItem(array, i);
      if (cJSON_IsNumber(item)) {
        target_array[i] = item->valueint;
      }
    }
  }
}

cJSON* serialize_game_data_to_json(GameData* data) {
  cJSON* json = cJSON_CreateObject();
  cJSON_AddNumberToObject(json, "player_index", data->player_index);
  cJSON_AddNumberToObject(json, "phase", data->phase);

  cJSON* money_array = cJSON_CreateArray();
  for (int i = 0; i < PLAYERS_COUNT; i++) {
    cJSON_AddItemToArray(money_array, cJSON_CreateNumber(data->money[i]));
  }
  cJSON_AddItemToObject(json, "money", money_array);

  // Serialize land_state and units_sea if needed
  cJSON* land_state_array = cJSON_CreateArray();
  for (int i = 0; i < LANDS_COUNT; i++) {
    cJSON_AddItemToArray(land_state_array,
                         serialize_land_state_to_json(&data->land_state[i]));
  }
  cJSON_AddItemToObject(json, "land_state", land_state_array);

  cJSON* units_sea_array = cJSON_CreateArray();
  for (int i = 0; i < SEAS_COUNT; i++) {
    cJSON_AddItemToArray(units_sea_array,
                         serialize_units_sea_to_json(&data->units_sea[i]));
  }
  cJSON_AddItemToObject(json, "units_sea", units_sea_array);

  return json;
}

cJSON* serialize_land_state_to_json(LandState* land_state) {
  cJSON* json = cJSON_CreateObject();
  cJSON_AddBoolToObject(json, "conquered", land_state->conquered);
  cJSON_AddNumberToObject(json, "owner_index", land_state->owner_index);
  cJSON_AddNumberToObject(json, "builds_left", land_state->builds_left);
  cJSON_AddNumberToObject(json, "factory_hp", land_state->factory_hp);
  cJSON_AddNumberToObject(json, "factory_max", land_state->factory_max);
  return json;
}
