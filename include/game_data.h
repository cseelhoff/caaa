#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "config.h"
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

typedef struct {
  uint8_t owner_index;
  uint8_t factory_max;
  uint8_t factory_hp;
  uint8_t builds_left;  
} LandState;

typedef struct {
  uint8_t infantry;
  uint8_t artillery;
  uint8_t tanks;
  uint8_t aa_guns;
  uint8_t fighters;
  uint8_t bombers;
} UnitsLandStatic;

typedef struct {
  uint8_t transports_empty;
  uint8_t transports_1i;
  uint8_t transports_1a;
  uint8_t transports_1t;
  uint8_t transports_2i;
  uint8_t transports_1i_1a;
  uint8_t transports_1i_1t;
  uint8_t destroyers;
  uint8_t carriers;
  uint8_t battleships;
  uint8_t battleships_damaged;
  uint8_t submarines;
  uint8_t fighters;
  uint8_t bombers;
} UnitsSeaStatic;

typedef struct {
  uint8_t infantry_0;
  uint8_t artillery_0;
  uint8_t tanks_0;
  uint8_t aa_guns_0;
  uint8_t fighters_0;
  uint8_t bombers_0;
  uint8_t infantry_1;
  uint8_t artillery_1;
  uint8_t tanks_1;
  uint8_t aa_guns_1;
  uint8_t fighters_1;
  uint8_t bombers_1;
  uint8_t tanks_2;
  uint8_t fighters_2;
  uint8_t bombers_2;
  uint8_t fighters_3;
  uint8_t bombers_3;
  uint8_t fighters_4;
  uint8_t bombers_4;
  uint8_t bombers_5;
  uint8_t bombers_6;
} UnitsLandMobile;

typedef struct { //limited transports
  uint8_t transports_empty_0;
  uint8_t transports_1i_0;
  uint8_t transports_1a_0;
  uint8_t transports_1t_0;
  uint8_t transports_2i_0;
  uint8_t transports_1i_1a_0;
  uint8_t transports_1i_1t_0;
  uint8_t destroyers_0;
  uint8_t carriers_0;
  uint8_t battleships_0;
  uint8_t battleships_damaged_0;
  uint8_t submarines_0;
  uint8_t fighters_0;
  //uint8_t bombers_0;
  uint8_t transports_empty_1;
  uint8_t transports_1i_1;
  uint8_t transports_1a_1;
  uint8_t transports_1t_1;
  uint8_t transports_2i_1;
  uint8_t transports_1i_1a_1;
  uint8_t transports_1i_1t_1;
  uint8_t destroyers_1;
  uint8_t carriers_1;
  uint8_t battleships_1;
  uint8_t battleships_damaged_1;
  uint8_t submarines_1;
  uint8_t fighters_1;
  uint8_t bombers_1;
  uint8_t transports_empty_2;
  uint8_t transports_1i_2;
  uint8_t transports_1a_2;
  uint8_t transports_1t_2;
  uint8_t transports_2i_2;
  uint8_t transports_1i_1a_2;
  uint8_t transports_1i_1t_2;
  uint8_t destroyers_2;
  uint8_t carriers_2;
  uint8_t battleships_2;
  uint8_t battleships_damaged_2;
  uint8_t submarines_2;
  uint8_t fighters_2;
  uint8_t bombers_2;
  uint8_t fighters_3;
  uint8_t bombers_3;
  uint8_t fighters_4;
  uint8_t bombers_4;
  uint8_t bombers_5;
  //uint8_t bombers_6;
} UnitsSeaMobile;

typedef struct {
  uint8_t player_index;
  uint8_t phase;
  LandState land_state[LANDS_COUNT];
  bool conquered[LANDS_COUNT];
  uint8_t money[PLAYERS_COUNT];
  UnitsLandStatic units_land_static[PLAYERS_COUNT - 1][LANDS_COUNT];
  UnitsSeaStatic units_sea_static[PLAYERS_COUNT - 1][SEAS_COUNT];
  UnitsLandMobile units_land_mobile[LANDS_COUNT];
  UnitsSeaMobile units_sea_mobile[SEAS_COUNT];
} GameData;

GameData initializeGameData();

#endif
