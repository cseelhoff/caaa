#include "connection.h"
#include "json_data_loader.h"
#include "player.h"
#include "territory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Connection* get_connections_from_cjson(cJSON* connections_cjson,
                                       int connection_count,
                                       Territory* territories,
                                       int territory_count) {
  // Allocate memory for the array of Connection structures
  Connection* connections = malloc(connection_count * sizeof(Connection));
  if (!connections) {
    printf("Memory allocation failed\n");
    cJSON_Delete(connections_cjson);
    return connections;
  }
  int index = 0;
  cJSON* connection;
  cJSON_ArrayForEach(connection, connections_cjson) {
    Connection c = c;
    char* dest_territory = getJsonString(connection, "dest_territory", DEF_DEST_TER);
    c.dest_territory = getTerritoryByName(territories, territory_count, dest_territory);
    index++;
  }
  return connections;
}