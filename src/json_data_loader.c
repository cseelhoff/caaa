#include "json_data_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

cJSON* loadJsonPath(char* path, char* string) {
  cJSON* cjson;
  char* jsonString = readFileToString(path);
  if (jsonString == NULL) {
    printf("Failed to read %s JSON file\n", path);
    return cjson;
  }

  cjson = cJSON_Parse(jsonString);
  free(jsonString);
  if (cjson == NULL) {
    printf("Failed to parse %s JSON\n", string);
    return cjson;
  }

  cjson = getJsonArray(cjson, string);
  return cjson;
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

cJSON* getJsonArray(cJSON* cjson, char* string) {
  cJSON* cjson_array = cJSON_GetObjectItemCaseSensitive(cjson, string);
  if (!cJSON_IsArray(cjson_array)) {
    printf("%s is not an array\n", string);
    cJSON_Delete(cjson);
  }
  return cjson_array;
}

int getJsonArrayLength(cJSON* cjson_array) {
  int count = cJSON_GetArraySize(cjson_array);
  if (count == 0) {
    printf("No items found in array\n");
    cJSON_Delete(cjson_array);
  }
  return count;
}

char* getJsonString(cJSON* cjson, char* key, char* default_value) {
  char* value = default_value;
  cJSON* value_cjson = cJSON_GetObjectItemCaseSensitive(cjson, key);
  if (value_cjson && cJSON_IsString(value_cjson))
    value = strdup(value_cjson->valuestring);
  return value;
}

uint8_t getJsonUint8_t(cJSON* cjson, char* key, uint8_t default_value) {
  uint8_t value = default_value;
  cJSON* value_cjson = cJSON_GetObjectItemCaseSensitive(cjson, key);
  if (value_cjson && cJSON_IsNumber(value_cjson))
    value = value_cjson->valueint;
  return value;
}

bool getJsonBool(cJSON* cjson, char* key, bool default_value) {
  bool value = default_value;
  cJSON* value_cjson = cJSON_GetObjectItemCaseSensitive(cjson, key);
  if (value_cjson)
    value = cJSON_IsTrue(value_cjson);
  return value;
}
