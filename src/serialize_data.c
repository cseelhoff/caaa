#include "serialize_data.h"
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
  } else {
    fprintf(stderr, "Invalid player_index\n");
  }

  cJSON* phase = cJSON_GetObjectItem(json, "phase");
  if (cJSON_IsNumber(phase)) {
    data->phase = phase->valueint;
  } else {
    fprintf(stderr, "Invalid phase\n");
  }

  cJSON* money_array = cJSON_GetObjectItem(json, "money");
  if (cJSON_IsArray(money_array)) {
    int array_size = cJSON_GetArraySize(money_array);
    for (int i = 0; i < array_size && i < PLAYERS_COUNT; i++) {
      cJSON* money_item = cJSON_GetArrayItem(money_array, i);
      if (cJSON_IsNumber(money_item)) {
        data->money[i] = money_item->valueint;
      } else {
        fprintf(stderr, "Invalid money item\n");
      }
    }
  } else {
    fprintf(stderr, "Invalid money array\n");
  }

  cJSON* land_state_array = cJSON_GetObjectItem(json, "land_state");
  if (cJSON_IsArray(land_state_array)) {
    int array_size = cJSON_GetArraySize(land_state_array);
    for (int i = 0; i < array_size && i < LANDS_COUNT; i++) {
      cJSON* land_state_item = cJSON_GetArrayItem(land_state_array, i);
      deserialize_land_state_from_json(&data->land_state[i], land_state_item);
    }
  } else {
    fprintf(stderr, "Invalid land_state array\n");
  }

  cJSON* units_sea_array = cJSON_GetObjectItem(json, "units_sea");
  if (cJSON_IsArray(units_sea_array)) {
    int array_size = cJSON_GetArraySize(units_sea_array);
    for (int i = 0; i < array_size && i < SEAS_COUNT; i++) {
      cJSON* units_sea_item = cJSON_GetArrayItem(units_sea_array, i);
      deserialize_units_sea_from_json(&data->units_sea[i], units_sea_item);
    }
  } else {
    fprintf(stderr, "Invalid units_sea array\n");
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
  extract_and_assign(json, FIGHTER_NAME, land_state->fighters);
  extract_and_assign(json, BOMBER_NAME, land_state->bombers);
  extract_and_assign(json, INFANTRY_NAME, land_state->infantry);
  extract_and_assign(json, ARTILLERY_NAME, land_state->artillery);
  extract_and_assign(json, TANK_NAME, land_state->tanks);
  extract_and_assign(json, AA_GUN_NAME, land_state->aa_guns);

  cJSON* other_units_array = cJSON_GetObjectItem(json, "other_units");
  if (cJSON_IsArray(other_units_array)) {
    int outer_size = cJSON_GetArraySize(other_units_array);
    for (int i = 0; i < outer_size && i < PLAYERS_COUNT - 1; i++) {
      cJSON* inner_array = cJSON_GetArrayItem(other_units_array, i);
      if (cJSON_IsArray(inner_array)) {
        int inner_size = cJSON_GetArraySize(inner_array);
        for (int j = 0; j < inner_size && j < LAND_UNIT_TYPES; j++) {
          cJSON* item = cJSON_GetArrayItem(inner_array, j);
          if (cJSON_IsNumber(item)) {
            land_state->other_units[i][j] = item->valueint;
          }
        }
      }
    }
  }
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

void add_array_to_json(cJSON* json, const char* key, uint8_t* array, int size) {
  cJSON* json_array = cJSON_CreateArray();
  for (int i = 0; i < size; i++) {
    cJSON_AddItemToArray(json_array, cJSON_CreateNumber(array[i]));
  }
  cJSON_AddItemToObject(json, key, json_array);
}

cJSON* serialize_units_sea_to_json(UnitsSea* units_sea) {
  cJSON* json = cJSON_CreateObject();

  add_array_to_json(json, NAMES_UNIT_SEA[0], units_sea->fighters,
                    STATES_MOVE_SEA[0]);
  add_array_to_json(json, NAMES_UNIT_SEA[1], units_sea->trans_empty,
                    STATES_MOVE_SEA[1]);
  add_array_to_json(json, NAMES_UNIT_SEA[2], units_sea->trans_1i,
                    STATES_MOVE_SEA[2]);
  add_array_to_json(json, NAMES_UNIT_SEA[3], units_sea->trans_1a,
                    STATES_MOVE_SEA[3]);
  add_array_to_json(json, NAMES_UNIT_SEA[4], units_sea->trans_1t,
                    STATES_MOVE_SEA[4]);
  add_array_to_json(json, NAMES_UNIT_SEA[5], units_sea->trans_2i,
                    STATES_MOVE_SEA[5]);
  add_array_to_json(json, NAMES_UNIT_SEA[6], units_sea->trans_1i_1a,
                    STATES_MOVE_SEA[6]);
  add_array_to_json(json, NAMES_UNIT_SEA[7], units_sea->trans_1i_1t,
                    STATES_MOVE_SEA[7]);
  add_array_to_json(json, NAMES_UNIT_SEA[8], units_sea->submarines,
                    STATES_MOVE_SEA[8]);
  add_array_to_json(json, NAMES_UNIT_SEA[9], units_sea->destroyers,
                    STATES_MOVE_SEA[9]);
  add_array_to_json(json, NAMES_UNIT_SEA[10], units_sea->carriers,
                    STATES_MOVE_SEA[10]);
  add_array_to_json(json, NAMES_UNIT_SEA[11], units_sea->battleships,
                    STATES_MOVE_SEA[11]);
  add_array_to_json(json, NAMES_UNIT_SEA[12], units_sea->bs_damaged,
                    STATES_MOVE_SEA[12]);
  add_array_to_json(json, NAMES_UNIT_SEA[13], units_sea->bombers,
                    STATES_MOVE_SEA[13]);

  cJSON* other_units_array = cJSON_CreateArray();
  for (int i = 0; i < PLAYERS_COUNT - 1; i++) {
    cJSON* inner_array = cJSON_CreateArray();
    for (int j = 0; j < SEA_UNIT_TYPES - 1; j++) {
      cJSON_AddItemToArray(inner_array,
                           cJSON_CreateNumber(units_sea->other_units[i][j]));
    }
    cJSON_AddItemToArray(other_units_array, inner_array);
  }
  cJSON_AddItemToObject(json, "other_units", other_units_array);

  return json;
}

void deserialize_units_sea_from_json(UnitsSea* units_sea, cJSON* json) {
  if (json == NULL) {
    fprintf(stderr, "Invalid JSON data\n");
    return;
  }

  extract_and_assign(json, NAMES_UNIT_SEA[0], units_sea->fighters);
  extract_and_assign(json, NAMES_UNIT_SEA[1], units_sea->trans_empty);
  extract_and_assign(json, NAMES_UNIT_SEA[2], units_sea->trans_1i);
  extract_and_assign(json, NAMES_UNIT_SEA[3], units_sea->trans_1a);
  extract_and_assign(json, NAMES_UNIT_SEA[4], units_sea->trans_1t);
  extract_and_assign(json, NAMES_UNIT_SEA[5], units_sea->trans_2i);
  extract_and_assign(json, NAMES_UNIT_SEA[6], units_sea->trans_1i_1a);
  extract_and_assign(json, NAMES_UNIT_SEA[7], units_sea->trans_1i_1t);
  extract_and_assign(json, NAMES_UNIT_SEA[8], units_sea->submarines);
  extract_and_assign(json, NAMES_UNIT_SEA[9], units_sea->destroyers);
  extract_and_assign(json, NAMES_UNIT_SEA[10], units_sea->carriers);
  extract_and_assign(json, NAMES_UNIT_SEA[11], units_sea->battleships);
  extract_and_assign(json, NAMES_UNIT_SEA[12], units_sea->bs_damaged);
  extract_and_assign(json, NAMES_UNIT_SEA[13], units_sea->bombers);

  cJSON* other_units_array = cJSON_GetObjectItem(json, "other_units");
  if (cJSON_IsArray(other_units_array)) {
    int array_size = cJSON_GetArraySize(other_units_array);
    for (int i = 0; i < array_size && i < PLAYERS_COUNT - 1; i++) {
      cJSON* inner_array = cJSON_GetArrayItem(other_units_array, i);
      if (cJSON_IsArray(inner_array)) {
        int inner_array_size = cJSON_GetArraySize(inner_array);
        for (int j = 0; j < inner_array_size && j < SEA_UNIT_TYPES - 1; j++) {
          cJSON* item = cJSON_GetArrayItem(inner_array, j);
          if (cJSON_IsNumber(item)) {
            units_sea->other_units[i][j] = item->valueint;
          } else {
            fprintf(stderr, "Invalid other_units item\n");
          }
        }
      }
    }
  }
}

cJSON* serialize_land_state_to_json(LandState* land_state) {
  cJSON* json = cJSON_CreateObject();

  cJSON_AddNumberToObject(json, "owner_index", land_state->owner_index);
  cJSON_AddNumberToObject(json, "builds_left", land_state->builds_left);
  cJSON_AddNumberToObject(json, "factory_hp", land_state->factory_hp);
  cJSON_AddNumberToObject(json, "factory_max", land_state->factory_max);

  add_array_to_json(json, NAMES_UNIT_LAND[0], land_state->fighters,
                    STATES_MOVE_LAND[0]);
  add_array_to_json(json, NAMES_UNIT_LAND[1], land_state->bombers,
                    STATES_MOVE_LAND[1]);
  add_array_to_json(json, NAMES_UNIT_LAND[2], land_state->infantry,
                    STATES_MOVE_LAND[2]);
  add_array_to_json(json, NAMES_UNIT_LAND[3], land_state->artillery,
                    STATES_MOVE_LAND[3]);
  add_array_to_json(json, NAMES_UNIT_LAND[4], land_state->tanks,
                    STATES_MOVE_LAND[4]);
  add_array_to_json(json, NAMES_UNIT_LAND[5], land_state->aa_guns,
                    STATES_MOVE_LAND[5]);

  cJSON* other_units_array = cJSON_CreateArray();
  for (int i = 0; i < PLAYERS_COUNT - 1; i++) {
    cJSON* inner_array = cJSON_CreateArray();
    for (int j = 0; j < LAND_UNIT_TYPES; j++) {
      cJSON_AddItemToArray(inner_array,
                           cJSON_CreateNumber(land_state->other_units[i][j]));
    }
    cJSON_AddItemToArray(other_units_array, inner_array);
  }
  cJSON_AddItemToObject(json, "other_units", other_units_array);

  return json;
}