#ifndef SERIALIZE_DATA_H
#define SERIALIZE_DATA_H

#include <cjson/cJSON.h>
#include "game_state.hpp"

void load_game_data_from_json(char const* filename, GameState* data);
cJSON* read_json_from_file(const char *filename);
void write_json_to_file(const char *filename, cJSON *json);
cJSON* serialize_game_data_to_json(GameState *data);
void deserialize_game_data_from_json(cJSON *json, GameState *data);
cJSON* serialize_land_state_to_json(LandState* land_state);
void deserialize_land_state_from_json(LandState* land_state, cJSON* json);
cJSON* serialize_units_sea_to_json(UnitsSea* units_sea);
void deserialize_units_sea_from_json(UnitsSea* units_sea, cJSON* json);
void set_land_state_field(cJSON* json, const char* key, uint* field);
void set_land_state_signed_field(cJSON* json, const char* key, int8_t* field);
void extract_and_assign(cJSON* json, const char* key, uint* target_array);

#endif
