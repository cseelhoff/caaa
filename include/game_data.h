#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "cJSON.h"
#include "connection.h"
#include "game_data_sizes.h"
#include "player.h"
#include "territory.h"
#include "unittype.h"

typedef struct GameData {
  /*
  struct UnitType unitTypes[UNIT_TYPE_COUNT];
  struct Player players[PLAYER_COUNT];
  struct Territory territories[TERRITORY_COUNT];
  struct Connection connections[CONNECTION_COUNT];
  */
  int a;
} GameData;

extern GameData gameDataInstance;

GameData* initializeGameData(int a);
cJSON* get_cjson_from_path(char* json_path, char* object_item_string);
char* readFileToString(const char* filename);
int get_count_from_cjson_array(cJSON* cJSON_ptr);
UnitType* get_unitTypes_from_cjson(cJSON* unitTypes_cjson, int unitType_count);
char* get_string_from_cjson(cJSON* cjson, char* key, char* default_value);
unsigned char get_char_from_cjson(cJSON* cjson, char* key, unsigned char default_value);
bool get_bool_from_cjson(cJSON* cjson, char* key, bool default_value);

#endif
