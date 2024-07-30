#include "connection.h"
#include "cJSON.h"
#include "json_data_loader.h"
#include "player.h"
#include "territory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Connections getJsonConnections(char* json_path, Territories territories) {

  cJSON* connections_cjson = loadJsonPath(json_path, "connections");
  Connections connections = {};
  connections.count = getJsonArrayLength(connections_cjson);
  
  if (connections.count == 0) {
    printf("No connections found\n");
    cJSON_Delete(connections_cjson);
    return connections;
  }

  connections.array = malloc(connections.count * sizeof(Connection));
  if (!connections.array) {
    printf("Memory allocation failed\n");
    cJSON_Delete(connections_cjson);
    return connections;
  }

  int index = 0;
  cJSON* conn;
  cJSON_ArrayForEach(conn, connections_cjson) {
    Connection c = c;
    char* dest_territory = getJsonString(conn, "dest_territory", DEF_DEST_TER);
    c.dest_territory = getTerritoryByName(territories, dest_territory);
    index++;
  }
  
  cJSON_Delete(connections_cjson);
  return connections;
}