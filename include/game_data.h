#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "config.h"
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include "cJSON.h"

#define STRING_BUFFER_SIZE 64

typedef struct {
  uint8_t infantry;
  uint8_t artillery;
  uint8_t tanks;
  uint8_t aa_guns;
  uint8_t fighters;
  uint8_t bombers;
} UnitsLandStatic;

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

typedef struct {
  UnitsLandMobile units_land_mobile;
  UnitsLandStatic units_land_static[PLAYERS_COUNT - 1];
} UnitsLand;

typedef struct {
  uint8_t owner_index;
  uint8_t builds_left;
  uint8_t factory_hp;
  uint8_t factory_max;
  bool conquered;
  UnitsLand units_land;
} LandState;

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
} UnitsSeaStatic;

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
} UnitsSeaMobileTotal;

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
  UnitsSeaMobile units_sea_mobile;
  UnitsSeaStatic units_sea_static[PLAYERS_COUNT - 1];
} UnitsSea;

typedef struct {
  uint8_t player_index;
  uint8_t phase;
  uint8_t money[PLAYERS_COUNT];
  LandState land_state[LANDS_COUNT];
  UnitsSea units_sea[SEAS_COUNT];  
} GameData;

typedef struct {
  UnitsLandStatic units_land_static[LANDS_COUNT];
  UnitsSeaMobileTotal units_sea_mobile_total[SEAS_COUNT];
  uint8_t units_land_total[LANDS_COUNT][PLAYERS_COUNT];
  uint8_t units_sea_total[SEAS_COUNT][PLAYERS_COUNT];
  uint8_t units_land_grand_total[LANDS_COUNT];
  uint8_t units_sea_grand_total[SEAS_COUNT];
} GameCache;

extern char* phases[2];

void initializeGameData();
void setPrintableStatus();
void buildCache();
void setPrintableStatusLands();
void setPrintableStatusSeas();

#endif
