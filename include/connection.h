#ifndef CONNECTION_H
#define CONNECTION_H

#define DEF_DEST_TER "DEFAULT DESTINATION TERRITORY"

#include "player.h"
#include "territory.h"
#include <stdbool.h>

typedef struct Connection {
  Territory* dest_territory;
  Territory* req_territory[2];
  bool* is_passable_for_player; //[TOTAL_PLAYERS];
} Connection;

typedef struct Connections {
  Connection* array;
  unsigned int count;
} Connections;

Connection* getJsonConnections(cJSON* connections_cjson, int connection_count,
                               Territory* territories, int territory_count);

#endif
