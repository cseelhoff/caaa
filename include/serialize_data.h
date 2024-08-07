#ifndef SERIALIZE_DATA_H
#define SERIALIZE_DATA_H

#include "game_data.h"
#include "cJSON.h"

cJSON* read_json_from_file(const char *filename);
void write_json_to_file(const char *filename, cJSON *json);
cJSON* serialize_game_data_to_json(GameData *data);
void deserialize_game_data_from_json(GameData *data, cJSON *json);
cJSON* serialize_land_state_to_json(LandState* land_state);
void deserialize_land_state_from_json(LandState* land_state, cJSON* json);
cJSON* serialize_units_land_to_json(UnitsLand* units_land);
void deserialize_units_land_from_json(UnitsLand* units_land, cJSON* json);
cJSON* serialize_units_land_mobile_to_json(UnitsLandMobile* units_land_mobile);
void deserialize_units_land_mobile_from_json(UnitsLandMobile* units_land_mobile, cJSON* json);
cJSON* serialize_units_land_static_to_json(UnitsLandStatic* units_land_static);
void deserialize_units_land_static_from_json(UnitsLandStatic* units_land_static, cJSON* json);
cJSON* serialize_units_sea_to_json(UnitsSea* units_sea);
void deserialize_units_sea_from_json(UnitsSea* units_sea, cJSON* json);
cJSON* serialize_units_sea_mobile_to_json(UnitsSeaMobile* units_sea_mobile);
void deserialize_units_sea_mobile_from_json(UnitsSeaMobile* units_sea_mobile, cJSON* json);
cJSON* serialize_units_sea_static_to_json(UnitsSeaStatic* units_sea_static);
void deserialize_units_sea_static_from_json(UnitsSeaStatic* units_sea_static, cJSON* json);

void set_land_state_field(cJSON* json, const char* key, uint8_t* field);
void extract_and_assign(cJSON* json, const char* key, uint8_t* target_array);

#endif
