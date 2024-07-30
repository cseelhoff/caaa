#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "connection.h"
#include "player.h"
#include "territory.h"
#include "unit_type.h"
#include <stdint.h>

typedef struct GameData {
  Player* players;//[TOTAL_PLAYERS];
  Territory* territories;//[TERRITORY_COUNT];
  uint8_t *gameState;//[]
} GameData;

GameData* initializeGameData(int a);

#endif
