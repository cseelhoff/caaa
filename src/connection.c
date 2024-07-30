#include "connection.h"
#include "config.h"
#include "json_data_loader.h"
#include "territory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void loadJsonConnections(char* json_path) {
  cJSON* connections_cjson = loadJsonPath(json_path, "connections");
  int json_count = getJsonArrayLength(connections_cjson);
  if (json_count != CONNECTIONS_COUNT) {
    printf("CONNECTIONS_COUNT (%d) unequal to json_count (%d)\n",
           CONNECTIONS_COUNT, json_count);
    exit(1);
  }

  int index = 0;
  cJSON* conn;
  cJSON_ArrayForEach(conn, connections_cjson) {
    Connection c = c;
    char* dest_territory = getJsonString(conn, "dest_territory");
    c.dest_territory = getTerritoryByName(dest_territory);
    index++;
  }

  cJSON_Delete(connections_cjson);
}