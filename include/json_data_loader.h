#ifndef JSON_DATA_LOADER_H
#define JSON_DATA_LOADER_H

#include "cJSON.h"
#include <stdbool.h>
#include <stdint.h>

cJSON* loadJsonPath(char* json_path, char* object_item_string);
char* readFileToString(const char* filename);
int getJsonArrayLength(cJSON* cjson_array);
char* getJsonString(cJSON* cjson, char* key, char* default_value);
uint8_t getJsonUint8_t(cJSON* cjson, char* key, uint8_t default_value);
bool getJsonBool(cJSON* cjson, char* key, bool default_value);

#endif
