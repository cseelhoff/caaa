#include "json_data_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    exit(1);
  }
  return cjson_array;
}

int getJsonArrayLength(cJSON* cjson_array) {
  int count = cJSON_GetArraySize(cjson_array);
  if (count != 0) {
    return count;
  }
  printf("No items found in array\n");
  exit(1);
}

char* getJsonString(cJSON* cjson, char* key) {
  cJSON* value_cjson = cJSON_GetObjectItemCaseSensitive(cjson, key);
  if (value_cjson && cJSON_IsString(value_cjson))
    return strdup(value_cjson->valuestring);
  printf("Failed to get %s\n", key);
  exit(1);
}

uint8_t getJsonUint8_t(cJSON* cjson, char* key) {
  cJSON* value_cjson = cJSON_GetObjectItemCaseSensitive(cjson, key);
  if (value_cjson && cJSON_IsNumber(value_cjson))
    return value_cjson->valueint;
  printf("Failed to get %s\n", key);
  exit(1);
}

bool getJsonBool(cJSON* cjson, char* key) {
  cJSON* value_cjson = cJSON_GetObjectItemCaseSensitive(cjson, key);
  if (value_cjson)
    return cJSON_IsTrue(value_cjson);
  printf("Failed to get %s\n", key);
  exit(1);
}
