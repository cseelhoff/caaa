#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "config.h"
#include "player.h"
#include "territory.h"
#include <stdint.h>

typedef struct {
  Player* players;
  Territory* territories;
  uint8_t *gameState;
} GameData;

typedef struct {
  uint8_t factory_max;
  uint8_t factory_hp;
  uint8_t builds_left;
  bool newly_conquered;
} TerritoryState;

GameData* initializeGameData(int a);

#endif
