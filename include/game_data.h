#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "game_data_sizes.h"
#include "unit_type.h"
#include "player.h"
#include "territory.h"
#include "connection.h"

typedef struct {
    UnitType unitTypes[UNIT_TYPE_COUNT];
    Player players[PLAYER_COUNT];
    Territory territories[TERRITORY_COUNT];
    Connection connections[CONNECTION_COUNT];
} GameData;

extern GameData gameDataInstance;

GameData* initializeGameData();

#endif