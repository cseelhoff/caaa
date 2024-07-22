#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "connection.h"
#include "game_data_sizes.h"
#include "player.h"
#include "territory.h"
#include "unittype.h"
#include <stdint.h>

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

#endif
