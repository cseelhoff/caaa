#include "serialize_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

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

cJSON* read_json_from_file(const char *filename) {
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
                data->money[i] = money_item->valuedouble;
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
    cJSON* owner_index = cJSON_GetObjectItem(json, "owner_index");
    if (cJSON_IsNumber(owner_index)) {
        land_state->owner_index = owner_index->valueint;
    }
    cJSON* builds_left = cJSON_GetObjectItem(json, "builds_left");
    if (cJSON_IsNumber(builds_left)) {
        land_state->builds_left = builds_left->valueint;
    }
    cJSON* factory_hp = cJSON_GetObjectItem(json, "factory_hp");
    if (cJSON_IsNumber(factory_hp)) {
        land_state->factory_hp = factory_hp->valueint;
    }
    cJSON* factory_max = cJSON_GetObjectItem(json, "factory_max");
    if (cJSON_IsNumber(factory_max)) {
        land_state->factory_max = factory_max->valueint;
    }
    cJSON* conquered = cJSON_GetObjectItem(json, "conquered");
    if (cJSON_IsBool(conquered)) {
        land_state->conquered = cJSON_IsTrue(conquered);
    }
    cJSON* units_land = cJSON_GetObjectItem(json, "units_land");
    if (cJSON_IsObject(units_land)) {
        deserialize_units_land_from_json(&land_state->units_land, units_land);
    }
}
void deserialize_units_land_from_json(UnitsLand* units_land, cJSON* json) {
    if (json == NULL) {
        fprintf(stderr, "Invalid JSON data\n");
        return;
    }

    cJSON* units_land_mobile = cJSON_GetObjectItem(json, "units_land_mobile");
    if (cJSON_IsObject(units_land_mobile)) {
        deserialize_units_land_mobile_from_json(&units_land->units_land_mobile, units_land_mobile);
    }

    cJSON* units_land_static_array = cJSON_GetObjectItem(json, "units_land_static");
    if (cJSON_IsArray(units_land_static_array)) {
        int array_size = cJSON_GetArraySize(units_land_static_array);
        for (int i = 0; i < array_size && i < PLAYERS_COUNT - 1; i++) {
            cJSON* units_land_static_item = cJSON_GetArrayItem(units_land_static_array, i);
            if (cJSON_IsObject(units_land_static_item)) {
                deserialize_units_land_static_from_json(&units_land->units_land_static[i], units_land_static_item);
            }
        }
    }
}

void deserialize_units_land_mobile_from_json(UnitsLandMobile* units_land_mobile, cJSON* json) {
    if (json == NULL) {
        fprintf(stderr, "Invalid JSON data\n");
        return;
    }

    cJSON* infantry_0 = cJSON_GetObjectItem(json, "infantry_0");
    if (cJSON_IsNumber(infantry_0)) {
        units_land_mobile->infantry_0 = infantry_0->valuedouble;
    }

    cJSON* infantry_1 = cJSON_GetObjectItem(json, "infantry_1");
    if (cJSON_IsNumber(infantry_1)) {
        units_land_mobile->infantry_1 = infantry_1->valuedouble;
    }

    cJSON* artillery_0 = cJSON_GetObjectItem(json, "artillery_0");
    if (cJSON_IsNumber(artillery_0)) {
        units_land_mobile->artillery_0 = artillery_0->valuedouble;
    }

    cJSON* artillery_1 = cJSON_GetObjectItem(json, "artillery_1");
    if (cJSON_IsNumber(artillery_1)) {
        units_land_mobile->artillery_1 = artillery_1->valuedouble;
    }

    cJSON* tanks_0 = cJSON_GetObjectItem(json, "tanks_0");
    if (cJSON_IsNumber(tanks_0)) {
        units_land_mobile->tanks_0 = tanks_0->valuedouble;
    }

    cJSON* tanks_1 = cJSON_GetObjectItem(json, "tanks_1");
    if (cJSON_IsNumber(tanks_1)) {
        units_land_mobile->tanks_1 = tanks_1->valuedouble;
    }

    cJSON* tanks_2 = cJSON_GetObjectItem(json, "tanks_2");
    if (cJSON_IsNumber(tanks_2)) {
        units_land_mobile->tanks_2 = tanks_2->valuedouble;
    }

    cJSON* aa_guns_0 = cJSON_GetObjectItem(json, "aa_guns_0");
    if (cJSON_IsNumber(aa_guns_0)) {
        units_land_mobile->aa_guns_0 = aa_guns_0->valuedouble;
    }

    cJSON* aa_guns_1 = cJSON_GetObjectItem(json, "aa_guns_1");
    if (cJSON_IsNumber(aa_guns_1)) {
        units_land_mobile->aa_guns_1 = aa_guns_1->valuedouble;
    }

    cJSON* fighters_0 = cJSON_GetObjectItem(json, "fighters_0");
    if (cJSON_IsNumber(fighters_0)) {
        units_land_mobile->fighters_0 = fighters_0->valuedouble;
    }

    cJSON* fighters_1 = cJSON_GetObjectItem(json, "fighters_1");
    if (cJSON_IsNumber(fighters_1)) {
        units_land_mobile->fighters_1 = fighters_1->valuedouble;
    }

    cJSON* fighters_2 = cJSON_GetObjectItem(json, "fighters_2");
    if (cJSON_IsNumber(fighters_2)) {
        units_land_mobile->fighters_2 = fighters_2->valuedouble;
    }

    cJSON* fighters_3 = cJSON_GetObjectItem(json, "fighters_3");
    if (cJSON_IsNumber(fighters_3)) {
        units_land_mobile->fighters_3 = fighters_3->valuedouble;
    }

    cJSON* fighters_4 = cJSON_GetObjectItem(json, "fighters_4");
    if (cJSON_IsNumber(fighters_4)) {
        units_land_mobile->fighters_4 = fighters_4->valuedouble;
    }

    cJSON* bombers_0 = cJSON_GetObjectItem(json, "bombers_0");
    if (cJSON_IsNumber(bombers_0)) {
        units_land_mobile->bombers_0 = bombers_0->valuedouble;
    }

    cJSON* bombers_1 = cJSON_GetObjectItem(json, "bombers_1");
    if (cJSON_IsNumber(bombers_1)) {
        units_land_mobile->bombers_1 = bombers_1->valuedouble;
    }

    cJSON* bombers_2 = cJSON_GetObjectItem(json, "bombers_2");
    if (cJSON_IsNumber(bombers_2)) {
        units_land_mobile->bombers_2 = bombers_2->valuedouble;
    }

    cJSON* bombers_3 = cJSON_GetObjectItem(json, "bombers_3");
    if (cJSON_IsNumber(bombers_3)) {
        units_land_mobile->bombers_3 = bombers_3->valuedouble;
    }

    cJSON* bombers_4 = cJSON_GetObjectItem(json, "bombers_4");
    if (cJSON_IsNumber(bombers_4)) {
        units_land_mobile->bombers_4 = bombers_4->valuedouble;
    }

    cJSON* bombers_5 = cJSON_GetObjectItem(json, "bombers_5");
    if (cJSON_IsNumber(bombers_5)) {
        units_land_mobile->bombers_5 = bombers_5->valuedouble;
    }

    cJSON* bombers_6 = cJSON_GetObjectItem(json, "bombers_6");
    if (cJSON_IsNumber(bombers_6)) {
        units_land_mobile->bombers_6 = bombers_6->valuedouble;
    }
}
void deserialize_units_land_static_from_json(UnitsLandStatic* units_land_static, cJSON* json) {
    if (json == NULL) {
        fprintf(stderr, "Invalid JSON data\n");
        return;
    }

    cJSON* infantry = cJSON_GetObjectItem(json, "infantry");
    if (cJSON_IsNumber(infantry)) {
        units_land_static->infantry = infantry->valuedouble;
    }

    cJSON* artillery = cJSON_GetObjectItem(json, "artillery");
    if (cJSON_IsNumber(artillery)) {
        units_land_static->artillery = artillery->valuedouble;
    }

    cJSON* tanks = cJSON_GetObjectItem(json, "tanks");
    if (cJSON_IsNumber(tanks)) {
        units_land_static->tanks = tanks->valuedouble;
    }

    cJSON* aa_guns = cJSON_GetObjectItem(json, "aa_guns");
    if (cJSON_IsNumber(aa_guns)) {
        units_land_static->aa_guns = aa_guns->valuedouble;
    }

    cJSON* fighters = cJSON_GetObjectItem(json, "fighters");
    if (cJSON_IsNumber(fighters)) {
        units_land_static->fighters = fighters->valuedouble;
    }

    cJSON* bombers = cJSON_GetObjectItem(json, "bombers");
    if (cJSON_IsNumber(bombers)) {
        units_land_static->bombers = bombers->valuedouble;
    }
}
void deserialize_units_sea_from_json(UnitsSea* units_sea, cJSON* json) {
    cJSON* units_sea_mobile_json = cJSON_GetObjectItem(json, "units_sea_mobile");
    if (units_sea_mobile_json) {
        deserialize_units_sea_mobile_from_json(&units_sea->units_sea_mobile, units_sea_mobile_json);
    }

    cJSON* units_sea_static_array = cJSON_GetObjectItem(json, "units_sea_static");
    if (units_sea_static_array) {
        int array_size = cJSON_GetArraySize(units_sea_static_array);
        for (int i = 0; i < array_size; i++) {
            cJSON* units_sea_static_json = cJSON_GetArrayItem(units_sea_static_array, i);
            if (units_sea_static_json) {
                deserialize_units_sea_static_from_json(&units_sea->units_sea_static[i], units_sea_static_json);
            }
        }
    }
}

void deserialize_units_sea_mobile_from_json(UnitsSeaMobile* units_sea_mobile, cJSON* json) {
    units_sea_mobile->transports_empty_0 = cJSON_GetObjectItem(json, "transports_empty_0")->valueint;
    units_sea_mobile->transports_1i_0 = cJSON_GetObjectItem(json, "transports_1i_0")->valueint;
    units_sea_mobile->transports_1a_0 = cJSON_GetObjectItem(json, "transports_1a_0")->valueint;
    units_sea_mobile->transports_1t_0 = cJSON_GetObjectItem(json, "transports_1t_0")->valueint;
    units_sea_mobile->transports_2i_0 = cJSON_GetObjectItem(json, "transports_2i_0")->valueint;
    units_sea_mobile->transports_1i_1a_0 = cJSON_GetObjectItem(json, "transports_1i_1a_0")->valueint;
    units_sea_mobile->transports_1i_1t_0 = cJSON_GetObjectItem(json, "transports_1i_1t_0")->valueint;
    units_sea_mobile->destroyers_0 = cJSON_GetObjectItem(json, "destroyers_0")->valueint;
    units_sea_mobile->carriers_0 = cJSON_GetObjectItem(json, "carriers_0")->valueint;
    units_sea_mobile->battleships_0 = cJSON_GetObjectItem(json, "battleships_0")->valueint;
    units_sea_mobile->battleships_damaged_0 = cJSON_GetObjectItem(json, "battleships_damaged_0")->valueint;
    units_sea_mobile->submarines_0 = cJSON_GetObjectItem(json, "submarines_0")->valueint;
    units_sea_mobile->fighters_0 = cJSON_GetObjectItem(json, "fighters_0")->valueint;
    units_sea_mobile->transports_empty_1 = cJSON_GetObjectItem(json, "transports_empty_1")->valueint;
    units_sea_mobile->transports_1i_1 = cJSON_GetObjectItem(json, "transports_1i_1")->valueint;
    units_sea_mobile->transports_1a_1 = cJSON_GetObjectItem(json, "transports_1a_1")->valueint;
    units_sea_mobile->transports_1t_1 = cJSON_GetObjectItem(json, "transports_1t_1")->valueint;
    units_sea_mobile->transports_2i_1 = cJSON_GetObjectItem(json, "transports_2i_1")->valueint;
    units_sea_mobile->transports_1i_1a_1 = cJSON_GetObjectItem(json, "transports_1i_1a_1")->valueint;
    units_sea_mobile->transports_1i_1t_1 = cJSON_GetObjectItem(json, "transports_1i_1t_1")->valueint;
    units_sea_mobile->destroyers_1 = cJSON_GetObjectItem(json, "destroyers_1")->valueint;
    units_sea_mobile->carriers_1 = cJSON_GetObjectItem(json, "carriers_1")->valueint;
    units_sea_mobile->battleships_1 = cJSON_GetObjectItem(json, "battleships_1")->valueint;
    units_sea_mobile->battleships_damaged_1 = cJSON_GetObjectItem(json, "battleships_damaged_1")->valueint;
    units_sea_mobile->submarines_1 = cJSON_GetObjectItem(json, "submarines_1")->valueint;
    units_sea_mobile->fighters_1 = cJSON_GetObjectItem(json, "fighters_1")->valueint;
    units_sea_mobile->bombers_1 = cJSON_GetObjectItem(json, "bombers_1")->valueint;
    units_sea_mobile->transports_empty_2 = cJSON_GetObjectItem(json, "transports_empty_2")->valueint;
    units_sea_mobile->transports_1i_2 = cJSON_GetObjectItem(json, "transports_1i_2")->valueint;
    units_sea_mobile->transports_1a_2 = cJSON_GetObjectItem(json, "transports_1a_2")->valueint;
    units_sea_mobile->transports_1t_2 = cJSON_GetObjectItem(json, "transports_1t_2")->valueint;
    units_sea_mobile->transports_2i_2 = cJSON_GetObjectItem(json, "transports_2i_2")->valueint;
    units_sea_mobile->transports_1i_1a_2 = cJSON_GetObjectItem(json, "transports_1i_1a_2")->valueint;
    units_sea_mobile->transports_1i_1t_2 = cJSON_GetObjectItem(json, "transports_1i_1t_2")->valueint;
    units_sea_mobile->destroyers_2 = cJSON_GetObjectItem(json, "destroyers_2")->valueint;
    units_sea_mobile->carriers_2 = cJSON_GetObjectItem(json, "carriers_2")->valueint;
    units_sea_mobile->battleships_2 = cJSON_GetObjectItem(json, "battleships_2")->valueint;
    units_sea_mobile->battleships_damaged_2 = cJSON_GetObjectItem(json, "battleships_damaged_2")->valueint;
    units_sea_mobile->submarines_2 = cJSON_GetObjectItem(json, "submarines_2")->valueint;
    units_sea_mobile->fighters_2 = cJSON_GetObjectItem(json, "fighters_2")->valueint;
    units_sea_mobile->bombers_2 = cJSON_GetObjectItem(json, "bombers_2")->valueint;
    units_sea_mobile->fighters_3 = cJSON_GetObjectItem(json, "fighters_3")->valueint;
    units_sea_mobile->bombers_3 = cJSON_GetObjectItem(json, "bombers_3")->valueint;
    units_sea_mobile->fighters_4 = cJSON_GetObjectItem(json, "fighters_4")->valueint;
    units_sea_mobile->bombers_4 = cJSON_GetObjectItem(json, "bombers_4")->valueint;
    units_sea_mobile->bombers_5 = cJSON_GetObjectItem(json, "bombers_5")->valueint;
}

void deserialize_units_sea_static_from_json(UnitsSeaStatic* units_sea_static, cJSON* json) {
    units_sea_static->transports_empty = cJSON_GetObjectItem(json, "transports_empty")->valueint;
    units_sea_static->transports_1i = cJSON_GetObjectItem(json, "transports_1i")->valueint;
    units_sea_static->transports_1a = cJSON_GetObjectItem(json, "transports_1a")->valueint;
    units_sea_static->transports_1t = cJSON_GetObjectItem(json, "transports_1t")->valueint;
    units_sea_static->transports_2i = cJSON_GetObjectItem(json, "transports_2i")->valueint;
    units_sea_static->transports_1i_1a = cJSON_GetObjectItem(json, "transports_1i_1a")->valueint;
    units_sea_static->transports_1i_1t = cJSON_GetObjectItem(json, "transports_1i_1t")->valueint;
    units_sea_static->destroyers = cJSON_GetObjectItem(json, "destroyers")->valueint;
    units_sea_static->carriers = cJSON_GetObjectItem(json, "carriers")->valueint;
    units_sea_static->battleships = cJSON_GetObjectItem(json, "battleships")->valueint;
    units_sea_static->battleships_damaged = cJSON_GetObjectItem(json, "battleships_damaged")->valueint;
    units_sea_static->submarines = cJSON_GetObjectItem(json, "submarines")->valueint;
    units_sea_static->fighters = cJSON_GetObjectItem(json, "fighters")->valueint;
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

  cJSON *units_sea_array = cJSON_CreateArray();
  for (int i = 0; i < SEAS_COUNT; i++) {
    cJSON_AddItemToArray(units_sea_array,
                         serialize_units_sea_to_json(&data->units_sea[i]));
  }
  cJSON_AddItemToObject(json, "units_sea", units_sea_array);

  return json;
}

cJSON* serialize_land_state_to_json(LandState* land_state) {
  cJSON* json = cJSON_CreateObject();
  cJSON_AddNumberToObject(json, "owner_index", land_state->owner_index);
  cJSON_AddNumberToObject(json, "builds_left", land_state->builds_left);
  cJSON_AddNumberToObject(json, "factory_hp", land_state->factory_hp);
  cJSON_AddNumberToObject(json, "factory_max", land_state->factory_max);
  cJSON_AddBoolToObject(json, "conquered", land_state->conquered);
  cJSON_AddItemToObject(json, "units_land",
                        serialize_units_land_to_json(&land_state->units_land));
  return json;
}

cJSON* serialize_units_land_to_json(UnitsLand* units_land) {
  cJSON* json = cJSON_CreateObject();
  cJSON_AddItemToObject(json, "units_land_mobile",
                        serialize_units_land_mobile_to_json(&units_land->units_land_mobile));
  cJSON* units_land_static_array = cJSON_CreateArray();
  for (int i = 0; i < PLAYERS_COUNT - 1; i++) {
    cJSON_AddItemToArray(units_land_static_array,
                         serialize_units_land_static_to_json(&units_land->units_land_static[i]));
  }
  cJSON_AddItemToObject(json, "units_land_static", units_land_static_array);
  return json;
}

cJSON* serialize_units_land_mobile_to_json(UnitsLandMobile* units_land_mobile) {
  cJSON* json = cJSON_CreateObject();
  cJSON_AddNumberToObject(json, "infantry_0", units_land_mobile->infantry_0);
  cJSON_AddNumberToObject(json, "infantry_1", units_land_mobile->infantry_1);
  cJSON_AddNumberToObject(json, "artillery_0", units_land_mobile->artillery_0);
  cJSON_AddNumberToObject(json, "artillery_1", units_land_mobile->artillery_1);
  cJSON_AddNumberToObject(json, "tanks_0", units_land_mobile->tanks_0);
  cJSON_AddNumberToObject(json, "tanks_1", units_land_mobile->tanks_1);
  cJSON_AddNumberToObject(json, "tanks_2", units_land_mobile->tanks_2);
  cJSON_AddNumberToObject(json, "aa_guns_0", units_land_mobile->aa_guns_0);
  cJSON_AddNumberToObject(json, "aa_guns_1", units_land_mobile->aa_guns_1);
  cJSON_AddNumberToObject(json, "fighters_0", units_land_mobile->fighters_0);
  cJSON_AddNumberToObject(json, "fighters_1", units_land_mobile->fighters_1);
  cJSON_AddNumberToObject(json, "fighters_2", units_land_mobile->fighters_2);
  cJSON_AddNumberToObject(json, "fighters_3", units_land_mobile->fighters_3);
  cJSON_AddNumberToObject(json, "fighters_4", units_land_mobile->fighters_4);
  cJSON_AddNumberToObject(json, "bombers_0", units_land_mobile->bombers_0);
  cJSON_AddNumberToObject(json, "bombers_1", units_land_mobile->bombers_1);
  cJSON_AddNumberToObject(json, "bombers_2", units_land_mobile->bombers_2);
  cJSON_AddNumberToObject(json, "bombers_3", units_land_mobile->bombers_3);
  cJSON_AddNumberToObject(json, "bombers_4", units_land_mobile->bombers_4);
  cJSON_AddNumberToObject(json, "bombers_5", units_land_mobile->bombers_5);
  cJSON_AddNumberToObject(json, "bombers_6", units_land_mobile->bombers_6);
  return json;
}

cJSON* serialize_units_land_static_to_json(UnitsLandStatic* units_land_static) {
  cJSON* json = cJSON_CreateObject();
  cJSON_AddNumberToObject(json, "infantry", units_land_static->infantry);
  cJSON_AddNumberToObject(json, "artillery", units_land_static->artillery);
  cJSON_AddNumberToObject(json, "tanks", units_land_static->tanks);
  cJSON_AddNumberToObject(json, "aa_guns", units_land_static->aa_guns);
  cJSON_AddNumberToObject(json, "fighters", units_land_static->fighters);
  cJSON_AddNumberToObject(json, "bombers", units_land_static->bombers);
  return json;
}

cJSON* serialize_units_sea_to_json(UnitsSea* units_sea) {
  cJSON* json = cJSON_CreateObject();
  cJSON_AddItemToObject(json, "units_sea_mobile",
                        serialize_units_sea_mobile_to_json(&units_sea->units_sea_mobile));
  cJSON* units_sea_static_array = cJSON_CreateArray();
  for (int i = 0; i < PLAYERS_COUNT - 1; i++) {
    cJSON_AddItemToArray(units_sea_static_array,
                         serialize_units_sea_static_to_json(&units_sea->units_sea_static[i]));
  }
  cJSON_AddItemToObject(json, "units_sea_static", units_sea_static_array);
  return json;
}

cJSON* serialize_units_sea_mobile_to_json(UnitsSeaMobile* units_sea_mobile) {
  cJSON* json = cJSON_CreateObject();
  cJSON_AddNumberToObject(json, "transports_empty_0", units_sea_mobile->transports_empty_0);
  cJSON_AddNumberToObject(json, "transports_1i_0", units_sea_mobile->transports_1i_0);
  cJSON_AddNumberToObject(json, "transports_1a_0", units_sea_mobile->transports_1a_0);
  cJSON_AddNumberToObject(json, "transports_1t_0", units_sea_mobile->transports_1t_0);
  cJSON_AddNumberToObject(json, "transports_2i_0", units_sea_mobile->transports_2i_0);
  cJSON_AddNumberToObject(json, "transports_1i_1a_0", units_sea_mobile->transports_1i_1a_0);
  cJSON_AddNumberToObject(json, "transports_1i_1t_0", units_sea_mobile->transports_1i_1t_0);
  cJSON_AddNumberToObject(json, "destroyers_0", units_sea_mobile->destroyers_0);
  cJSON_AddNumberToObject(json, "carriers_0", units_sea_mobile->carriers_0);
  cJSON_AddNumberToObject(json, "battleships_0", units_sea_mobile->battleships_0);
  cJSON_AddNumberToObject(json, "battleships_damaged_0", units_sea_mobile->battleships_damaged_0);
  cJSON_AddNumberToObject(json, "submarines_0", units_sea_mobile->submarines_0);
  cJSON_AddNumberToObject(json, "fighters_0", units_sea_mobile->fighters_0);
  cJSON_AddNumberToObject(json, "transports_empty_1", units_sea_mobile->transports_empty_1);
  cJSON_AddNumberToObject(json, "transports_1i_1", units_sea_mobile->transports_1i_1);
  cJSON_AddNumberToObject(json, "transports_1a_1", units_sea_mobile->transports_1a_1);
  cJSON_AddNumberToObject(json, "transports_1t_1", units_sea_mobile->transports_1t_1);
  cJSON_AddNumberToObject(json, "transports_2i_1", units_sea_mobile->transports_2i_1);
  cJSON_AddNumberToObject(json, "transports_1i_1a_1", units_sea_mobile->transports_1i_1a_1);
  cJSON_AddNumberToObject(json, "transports_1i_1t_1", units_sea_mobile->transports_1i_1t_1);
  cJSON_AddNumberToObject(json, "destroyers_1", units_sea_mobile->destroyers_1);
  cJSON_AddNumberToObject(json, "carriers_1", units_sea_mobile->carriers_1);
  cJSON_AddNumberToObject(json, "battleships_1", units_sea_mobile->battleships_1);
  cJSON_AddNumberToObject(json, "battleships_damaged_1", units_sea_mobile->battleships_damaged_1);
  cJSON_AddNumberToObject(json, "submarines_1", units_sea_mobile->submarines_1);
  cJSON_AddNumberToObject(json, "fighters_1", units_sea_mobile->fighters_1);
  cJSON_AddNumberToObject(json, "bombers_1", units_sea_mobile->bombers_1);
  cJSON_AddNumberToObject(json, "transports_empty_2", units_sea_mobile->transports_empty_2);
  cJSON_AddNumberToObject(json, "transports_1i_2", units_sea_mobile->transports_1i_2);
  cJSON_AddNumberToObject(json, "transports_1a_2", units_sea_mobile->transports_1a_2);
  cJSON_AddNumberToObject(json, "transports_1t_2", units_sea_mobile->transports_1t_2);
  cJSON_AddNumberToObject(json, "transports_2i_2", units_sea_mobile->transports_2i_2);
  cJSON_AddNumberToObject(json, "transports_1i_1a_2", units_sea_mobile->transports_1i_1a_2);
  cJSON_AddNumberToObject(json, "transports_1i_1t_2", units_sea_mobile->transports_1i_1t_2);
  cJSON_AddNumberToObject(json, "destroyers_2", units_sea_mobile->destroyers_2);
  cJSON_AddNumberToObject(json, "carriers_2", units_sea_mobile->carriers_2);
  cJSON_AddNumberToObject(json, "battleships_2", units_sea_mobile->battleships_2);
  cJSON_AddNumberToObject(json, "battleships_damaged_2", units_sea_mobile->battleships_damaged_2);
  cJSON_AddNumberToObject(json, "submarines_2", units_sea_mobile->submarines_2);
  cJSON_AddNumberToObject(json, "fighters_2", units_sea_mobile->fighters_2);
  cJSON_AddNumberToObject(json, "bombers_2", units_sea_mobile->bombers_2);
  cJSON_AddNumberToObject(json, "fighters_3", units_sea_mobile->fighters_3);
  cJSON_AddNumberToObject(json, "bombers_3", units_sea_mobile->bombers_3);
  cJSON_AddNumberToObject(json, "fighters_4", units_sea_mobile->fighters_4);
  cJSON_AddNumberToObject(json, "bombers_4", units_sea_mobile->bombers_4);
  cJSON_AddNumberToObject(json, "bombers_5", units_sea_mobile->bombers_5);  
  return json;
}

cJSON* serialize_units_sea_static_to_json(UnitsSeaStatic* units_sea_static) {
  cJSON* json = cJSON_CreateObject();
  cJSON_AddNumberToObject(json, "transports_empty", units_sea_static->transports_empty);
  cJSON_AddNumberToObject(json, "transports_1i", units_sea_static->transports_1i);
  cJSON_AddNumberToObject(json, "transports_1a", units_sea_static->transports_1a);
  cJSON_AddNumberToObject(json, "transports_1t", units_sea_static->transports_1t);
  cJSON_AddNumberToObject(json, "transports_2i", units_sea_static->transports_2i);
  cJSON_AddNumberToObject(json, "transports_1i_1a", units_sea_static->transports_1i_1a);
  cJSON_AddNumberToObject(json, "transports_1i_1t", units_sea_static->transports_1i_1t);
  cJSON_AddNumberToObject(json, "destroyers", units_sea_static->destroyers);
  cJSON_AddNumberToObject(json, "carriers", units_sea_static->carriers);
  cJSON_AddNumberToObject(json, "battleships", units_sea_static->battleships);
  cJSON_AddNumberToObject(json, "battleships_damaged", units_sea_static->battleships_damaged);
  cJSON_AddNumberToObject(json, "submarines", units_sea_static->submarines);
  cJSON_AddNumberToObject(json, "fighters", units_sea_static->fighters);
  return json;
}
