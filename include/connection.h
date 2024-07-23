#ifndef CONNECTION_H
#define CONNECTION_H

#define DEF_DEST_TER "DEFAULT DESTINATION TERRITORY"

#include <stdbool.h>
#include "player.h"
#include "territory.h"

typedef struct Connection {
  Territory* dest_territory;
  Territory* req_territory[2];
  bool* is_passable_for_player; //[TOTAL_PLAYERS];
} Connection;

#endif
