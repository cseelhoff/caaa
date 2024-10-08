#include "serialize_data.h"
#include "game_state.h"
#include <cjson/cJSON.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PATH_MAX 4096
void load_game_data_from_json(char const* filename, GameState* data) {
  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("Current working directory: %s\n", cwd);
  } else {
    perror("getcwd() error");
  }
  cJSON* json = read_json_from_file(filename);
  deserialize_game_data_from_json(json, data);
  cJSON_Delete(json);
}

void write_json_to_file(const char* filename, cJSON* json) {
  char* string = cJSON_Print(json);
  FILE* file = fopen(filename, "w");
  if (file == NULL) {
    perror("Failed to open file");
    free(string);
    exit(1);
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
    exit(1);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char* data = (char*)malloc((size_t)length + 1);
  if (data == NULL) {
    perror("Failed to allocate memory");
    fclose(file);
    return NULL;
  }

  size_t read_size = fread(data, 1, (size_t)length, file);
  if (read_size != (size_t)length) {
    perror("Failed to read the expected number of bytes");
    free(data);
    (void)fclose(file);
    return NULL;
  }
  data[length] = '\0';
  fclose(file);

  cJSON* json = cJSON_Parse(data);
  free(data);

  if (json == NULL) {
    fprintf(stderr, "Failed to parse JSON\n");
  }

  return json;
}

void deserialize_game_data_from_json(cJSON* json, GameState* data) {
  if (json == NULL) {
    fprintf(stderr, "Invalid JSON data\n");
    return;
  }

  cJSON* player_index = cJSON_GetObjectItem(json, "player_index");
  if (cJSON_IsNumber(player_index)) {
    data->player_index = (uint8_t)player_index->valueint;
  } else {
    fprintf(stderr, "Invalid player_index\n");
  }

  // cJSON* phase = cJSON_GetObjectItem(json, "phase");
  // if (cJSON_IsNumber(phase)) {
  //   data->phase = phase->valueint;
  // } else {
  //   fprintf(stderr, "Invalid phase\n");
  // }

  cJSON* money_array = cJSON_GetObjectItem(json, "money");
  if (cJSON_IsArray(money_array)) {
    int array_size = cJSON_GetArraySize(money_array);
    for (int i = 0; i < array_size && i < PLAYERS_COUNT; i++) {
      cJSON* money_item = cJSON_GetArrayItem(money_array, i);
      if (cJSON_IsNumber(money_item)) {
        data->money[i] = (uint8_t)money_item->valueint;
      } else {
        fprintf(stderr, "Invalid money item\n");
      }
    }
  } else {
    fprintf(stderr, "Invalid money array\n");
  }

  cJSON* builds_left_array = cJSON_GetObjectItem(json, "builds_left");
  if (cJSON_IsArray(builds_left_array)) {
    int array_size = cJSON_GetArraySize(builds_left_array);
    for (int i = 0; i < array_size && i < AIRS_COUNT; i++) {
      cJSON* builds_left_item = cJSON_GetArrayItem(builds_left_array, i);
      if (cJSON_IsNumber(builds_left_item)) {
        data->builds_left[i] = (uint8_t)builds_left_item->valueint;
      } else {
        fprintf(stderr, "Invalid builds_left item\n");
      }
    }
  } else {
    fprintf(stderr, "Invalid builds_left array\n");
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

  cJSON* flagged_for_combat = cJSON_GetObjectItem(json, "flagged_for_combat");
  if (cJSON_IsArray(flagged_for_combat)) {
    int array_size = cJSON_GetArraySize(flagged_for_combat);
    for (int i = 0; i < array_size && i < AIRS_COUNT; i++) {
      cJSON* flagged_for_combat_item = cJSON_GetArrayItem(flagged_for_combat, i);
      if (cJSON_IsNumber(flagged_for_combat_item)) {
        data->flagged_for_combat[i] = (uint8_t)flagged_for_combat_item->valueint;
      } else {
        fprintf(stderr, "Invalid flagged_for_combat item\n");
      }
    }
  } else {
    fprintf(stderr, "Invalid flagged_for_combat array\n");
  }

  cJSON* other_land_units_array = cJSON_GetObjectItem(json, "other_land_units");
  if (cJSON_IsArray(other_land_units_array)) {
    int outer_size = cJSON_GetArraySize(other_land_units_array);
    for (int i = 0; i < outer_size && i < PLAYERS_COUNT - 1; i++) {
      cJSON* middle_array = cJSON_GetArrayItem(other_land_units_array, i);
      if (cJSON_IsArray(middle_array)) {
        int middle_size = cJSON_GetArraySize(middle_array);
        for (int j = 0; j < middle_size && j < LANDS_COUNT; j++) {
          cJSON* inner_array = cJSON_GetArrayItem(middle_array, j);
          if (cJSON_IsArray(inner_array)) {
            int inner_size = cJSON_GetArraySize(inner_array);
            for (int k = 0; k < inner_size && k < LAND_UNIT_TYPES_COUNT; k++) {
              cJSON* item = cJSON_GetArrayItem(inner_array, k);
              if (cJSON_IsNumber(item)) {
                data->other_land_units[i][j][k] = (uint8_t)item->valueint;
              }
            }
          }
        }
      }
    }
  } else {
    fprintf(stderr, "Invalid other_land_units array\n");
  }

  cJSON* other_sea_units_array = cJSON_GetObjectItem(json, "other_sea_units");
  if (cJSON_IsArray(other_sea_units_array)) {
    int outer_size = cJSON_GetArraySize(other_sea_units_array);
    for (int i = 0; i < outer_size && i < PLAYERS_COUNT - 1; i++) {
      cJSON* middle_array = cJSON_GetArrayItem(other_sea_units_array, i);
      if (cJSON_IsArray(middle_array)) {
        int middle_size = cJSON_GetArraySize(middle_array);
        for (int j = 0; j < middle_size && j < SEAS_COUNT; j++) {
          cJSON* inner_array = cJSON_GetArrayItem(middle_array, j);
          if (cJSON_IsArray(inner_array)) {
            int inner_size = cJSON_GetArraySize(inner_array);
            for (int k = 0; k < inner_size && k < SEA_UNIT_TYPES_COUNT; k++) {
              cJSON* item = cJSON_GetArrayItem(inner_array, k);
              if (cJSON_IsNumber(item)) {
                data->other_sea_units[i][j][k] = (uint8_t)item->valueint;
              } else {
                fprintf(stderr, "Invalid other_sea_units item\n");
              }
            }
          }
        }
      }
    }
  } else {
    fprintf(stderr, "Invalid other_sea_units array\n");
  }
}
void deserialize_land_state_from_json(LandState* land_state, cJSON* json) {
  if (json == NULL) {
    fprintf(stderr, "Invalid JSON data\n");
    return;
  }
  set_land_state_field(json, "owner_index", &land_state->owner_idx);
  // set_land_state_field(json, "builds_left", &land_state->builds_left);
  set_land_state_signed_field(json, "factory_hp", &land_state->factory_hp);
  set_land_state_field(json, "factory_max", &land_state->factory_max);
  extract_and_assign(json, FIGHTER_NAME, land_state->fighters);
  extract_and_assign(json, BOMBER_NAME, land_state->bombers);
  extract_and_assign(json, INFANTRY_NAME, land_state->infantry);
  extract_and_assign(json, ARTILLERY_NAME, land_state->artillery);
  extract_and_assign(json, TANK_NAME, land_state->tanks);
  extract_and_assign(json, AA_GUN_NAME, land_state->aa_guns);
}

void set_land_state_signed_field(cJSON* json, const char* key, int8_t* field) {
  cJSON* item = cJSON_GetObjectItem(json, key);
  if (cJSON_IsNumber(item)) {
    *field = (int8_t)item->valueint;
  }
}

void set_land_state_field(cJSON* json, const char* key, uint8_t* field) {
  cJSON* item = cJSON_GetObjectItem(json, key);
  if (cJSON_IsNumber(item)) {
    *field = (uint8_t)item->valueint;
  }
}

void extract_and_assign(cJSON* json, const char* key, uint8_t* target_array) {
  cJSON* array = cJSON_GetObjectItem(json, key);
  if (cJSON_IsArray(array)) {
    int array_size = cJSON_GetArraySize(array);
    for (int i = 0; i < array_size; i++) {
      cJSON* item = cJSON_GetArrayItem(array, i);
      if (cJSON_IsNumber(item)) {
        target_array[i] = (uint8_t)item->valueint;
      }
    }
  }
}

cJSON* serialize_game_data_to_json(GameState* data) {
  cJSON* json = cJSON_CreateObject();
  cJSON_AddNumberToObject(json, "player_index", data->player_index);
  // cJSON_AddNumberToObject(json, "phase", data->phase);

  cJSON* money_array = cJSON_CreateArray();
  for (int i = 0; i < PLAYERS_COUNT; i++) {
    cJSON_AddItemToArray(money_array, cJSON_CreateNumber(data->money[i]));
  }
  cJSON_AddItemToObject(json, "money", money_array);

  cJSON* builds_left_array = cJSON_CreateArray();
  for (int i = 0; i < AIRS_COUNT; i++) {
    cJSON_AddItemToArray(builds_left_array, cJSON_CreateNumber(data->builds_left[i]));
  }
  cJSON_AddItemToObject(json, "builds_left", builds_left_array);

  // Serialize land_state and units_sea if needed
  cJSON* land_state_array = cJSON_CreateArray();
  for (int i = 0; i < LANDS_COUNT; i++) {
    cJSON_AddItemToArray(land_state_array, serialize_land_state_to_json(&data->land_state[i]));
  }
  cJSON_AddItemToObject(json, "land_state", land_state_array);

  cJSON* units_sea_array = cJSON_CreateArray();
  for (int i = 0; i < SEAS_COUNT; i++) {
    cJSON_AddItemToArray(units_sea_array, serialize_units_sea_to_json(&data->units_sea[i]));
  }
  cJSON_AddItemToObject(json, "units_sea", units_sea_array);

  cJSON* flagged_for_combat = cJSON_CreateArray();
  for (int i = 0; i < AIRS_COUNT; i++) {
    cJSON_AddItemToArray(flagged_for_combat, cJSON_CreateNumber(data->flagged_for_combat[i]));
  }
  cJSON_AddItemToObject(json, "flagged_for_combat", flagged_for_combat);

  cJSON* other_land_units_array = cJSON_CreateArray();
  for (int i = 0; i < PLAYERS_COUNT - 1; i++) {
    cJSON* middle_array = cJSON_CreateArray();
    for (int j = 0; j < LANDS_COUNT; j++) {
      cJSON* inner_array = cJSON_CreateArray();
      for (int k = 0; k < LAND_UNIT_TYPES_COUNT; k++) {
        cJSON_AddItemToArray(inner_array, cJSON_CreateNumber(data->other_land_units[i][j][k]));
      }
      cJSON_AddItemToArray(middle_array, inner_array);
    }
    cJSON_AddItemToArray(other_land_units_array, middle_array);
  }
  cJSON_AddItemToObject(json, "other_land_units", other_land_units_array);

  cJSON* other_sea_units_array = cJSON_CreateArray();
  for (int i = 0; i < PLAYERS_COUNT - 1; i++) {
    cJSON* middle_array = cJSON_CreateArray();
    for (int j = 0; j < SEAS_COUNT; j++) {
      cJSON* inner_array = cJSON_CreateArray();
      for (int k = 0; k < SEA_UNIT_TYPES_COUNT; k++) {
        cJSON_AddItemToArray(inner_array, cJSON_CreateNumber(data->other_sea_units[i][j][k]));
      }
      cJSON_AddItemToArray(middle_array, inner_array);
    }
    cJSON_AddItemToArray(other_sea_units_array, middle_array);
  }
  cJSON_AddItemToObject(json, "other_sea_units", other_sea_units_array);
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

  add_array_to_json(json, NAMES_UNIT_SEA[FIGHTERS], units_sea->fighters, STATES_MOVE_SEA[FIGHTERS]);
  add_array_to_json(json, NAMES_UNIT_SEA[TRANS_EMPTY], units_sea->trans_empty,
                    STATES_MOVE_SEA[TRANS_EMPTY]);
  add_array_to_json(json, NAMES_UNIT_SEA[TRANS_1I], units_sea->trans_1i, STATES_MOVE_SEA[TRANS_1I]);
  add_array_to_json(json, NAMES_UNIT_SEA[TRANS_1A], units_sea->trans_1a, STATES_MOVE_SEA[TRANS_1A]);
  add_array_to_json(json, NAMES_UNIT_SEA[TRANS_1T], units_sea->trans_1t, STATES_MOVE_SEA[TRANS_1T]);
  add_array_to_json(json, NAMES_UNIT_SEA[TRANS_2I], units_sea->trans_2i, STATES_MOVE_SEA[TRANS_2I]);
  add_array_to_json(json, NAMES_UNIT_SEA[TRANS_1I_1A], units_sea->trans_1i_1a,
                    STATES_MOVE_SEA[TRANS_1I_1A]);
  add_array_to_json(json, NAMES_UNIT_SEA[TRANS_1I_1T], units_sea->trans_1i_1t,
                    STATES_MOVE_SEA[TRANS_1I_1T]);
  add_array_to_json(json, NAMES_UNIT_SEA[SUBMARINES], units_sea->submarines,
                    STATES_MOVE_SEA[SUBMARINES]);
  add_array_to_json(json, NAMES_UNIT_SEA[DESTROYERS], units_sea->destroyers,
                    STATES_MOVE_SEA[DESTROYERS]);
  add_array_to_json(json, NAMES_UNIT_SEA[CARRIERS], units_sea->carriers, STATES_MOVE_SEA[CARRIERS]);
  add_array_to_json(json, NAMES_UNIT_SEA[CRUISERS], units_sea->cruisers, STATES_MOVE_SEA[CRUISERS]);
  add_array_to_json(json, NAMES_UNIT_SEA[BATTLESHIPS], units_sea->battleships,
                    STATES_MOVE_SEA[BATTLESHIPS]);
  add_array_to_json(json, NAMES_UNIT_SEA[BS_DAMAGED], units_sea->bs_damaged,
                    STATES_MOVE_SEA[BS_DAMAGED]);
  add_array_to_json(json, NAMES_UNIT_SEA[BOMBERS_SEA], units_sea->bombers,
                    STATES_MOVE_SEA[BOMBERS_SEA]);

  return json;
}

void deserialize_units_sea_from_json(UnitsSea* units_sea, cJSON* json) {
  if (json == NULL) {
    fprintf(stderr, "Invalid JSON data\n");
    return;
  }

  extract_and_assign(json, NAMES_UNIT_SEA[FIGHTERS], units_sea->fighters);
  extract_and_assign(json, NAMES_UNIT_SEA[TRANS_EMPTY], units_sea->trans_empty);
  extract_and_assign(json, NAMES_UNIT_SEA[TRANS_1I], units_sea->trans_1i);
  extract_and_assign(json, NAMES_UNIT_SEA[TRANS_1A], units_sea->trans_1a);
  extract_and_assign(json, NAMES_UNIT_SEA[TRANS_1T], units_sea->trans_1t);
  extract_and_assign(json, NAMES_UNIT_SEA[TRANS_2I], units_sea->trans_2i);
  extract_and_assign(json, NAMES_UNIT_SEA[TRANS_1I_1A], units_sea->trans_1i_1a);
  extract_and_assign(json, NAMES_UNIT_SEA[TRANS_1I_1T], units_sea->trans_1i_1t);
  extract_and_assign(json, NAMES_UNIT_SEA[SUBMARINES], units_sea->submarines);
  extract_and_assign(json, NAMES_UNIT_SEA[DESTROYERS], units_sea->destroyers);
  extract_and_assign(json, NAMES_UNIT_SEA[CARRIERS], units_sea->carriers);
  extract_and_assign(json, NAMES_UNIT_SEA[CRUISERS], units_sea->cruisers);
  extract_and_assign(json, NAMES_UNIT_SEA[BATTLESHIPS], units_sea->battleships);
  extract_and_assign(json, NAMES_UNIT_SEA[BS_DAMAGED], units_sea->bs_damaged);
  extract_and_assign(json, NAMES_UNIT_SEA[BOMBERS_SEA], units_sea->bombers);
}

cJSON* serialize_land_state_to_json(LandState* land_state) {
  cJSON* json = cJSON_CreateObject();

  cJSON_AddNumberToObject(json, "owner_index", land_state->owner_idx);
  cJSON_AddNumberToObject(json, "factory_hp", land_state->factory_hp);
  cJSON_AddNumberToObject(json, "factory_max", land_state->factory_max);

  add_array_to_json(json, NAMES_UNIT_LAND[FIGHTERS], land_state->fighters,
                    STATES_MOVE_LAND[FIGHTERS]);
  add_array_to_json(json, NAMES_UNIT_LAND[BOMBERS_LAND_AIR], land_state->bombers,
                    STATES_MOVE_LAND[BOMBERS_LAND_AIR]);
  add_array_to_json(json, NAMES_UNIT_LAND[INFANTRY], land_state->infantry,
                    STATES_MOVE_LAND[INFANTRY]);
  add_array_to_json(json, NAMES_UNIT_LAND[ARTILLERY], land_state->artillery,
                    STATES_MOVE_LAND[ARTILLERY]);
  add_array_to_json(json, NAMES_UNIT_LAND[TANKS], land_state->tanks, STATES_MOVE_LAND[TANKS]);
  add_array_to_json(json, NAMES_UNIT_LAND[AA_GUNS], land_state->aa_guns, STATES_MOVE_LAND[AA_GUNS]);

  return json;
}