#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "cJSON.h"
#include "config.h"
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#define STRING_BUFFER_SIZE 64

#define SEA_UNIT_TYPES 14
#define FIGHTERS 0
#define TRANS_EMPTY 1
#define TRANS_1I 2
#define TRANS_1A 3
#define TRANS_1T 4
#define TRANS_2I 5
#define TRANS_1I_1A 6
#define TRANS_1I_1T 7
#define SUBMARINES 8
#define DESTROYERS 9
#define CARRIERS 10
#define BATTLESHIPS 11
#define BS_DAMAGED 12
#define BOMBERS_SEA 13

#define LAND_UNIT_TYPES 6
#define BOMBERS_LAND 1
#define INFANTRY 2
#define ARTILLERY 3
#define TANKS 4
#define AA_GUNS 5

typedef struct {
  uint8_t fighters;
  uint8_t bombers;
  uint8_t infantry;
  uint8_t artillery;
  uint8_t tanks;
  uint8_t aa_guns;
} UnitsLandStatic;

typedef struct {
  uint8_t fighters[5];
  uint8_t bombers[7];
  uint8_t infantry[2];
  uint8_t artillery[2];
  uint8_t tanks[3];
  uint8_t aa_guns[2];
} UnitsLandMobile;

typedef struct {
  uint8_t fighters[5];
  uint8_t bombers[7];
  uint8_t infantry[2];
  uint8_t artillery[2];
  uint8_t tanks[3];
  uint8_t aa_guns[2];
  uint8_t other_units[PLAYERS_COUNT - 1][LAND_UNIT_TYPES];
} UnitsLand;

typedef struct {
  uint8_t owner_index; // rotates
  uint8_t builds_left;
  uint8_t factory_hp;
  uint8_t factory_max;
  bool conquered;
  uint8_t fighters[5];
  uint8_t bombers[7];
  uint8_t infantry[2];
  uint8_t artillery[2];
  uint8_t tanks[3];
  uint8_t aa_guns[2];
  uint8_t other_units[PLAYERS_COUNT - 1][LAND_UNIT_TYPES];
} LandState;

typedef struct {
  uint8_t fighters;
  uint8_t transports_empty;
  uint8_t transports_1i;
  uint8_t transports_1a;
  uint8_t transports_1t;
  uint8_t transports_2i;
  uint8_t transports_1i_1a;
  uint8_t transports_1i_1t;
  uint8_t submarines;
  uint8_t destroyers;
  uint8_t carriers;
  uint8_t battleships;
  uint8_t battleships_damaged;
} UnitsSeaStatic;

typedef struct {
  uint8_t fighters;
  uint8_t transports_empty;
  uint8_t transports_1i;
  uint8_t transports_1a;
  uint8_t transports_1t;
  uint8_t transports_2i;
  uint8_t transports_1i_1a;
  uint8_t transports_1i_1t;
  uint8_t submarines;
  uint8_t destroyers;
  uint8_t carriers;
  uint8_t battleships;
  uint8_t battleships_damaged;
  uint8_t bombers;
} UnitsSeaMobileTotal;

typedef struct { // limited transports
  uint8_t fighters[5];
  uint8_t transports_empty[4]; // move remain 0,1,2,s
  uint8_t transports_1i[4];
  uint8_t transports_1a[4];
  uint8_t transports_1t[4];
  uint8_t transports_2i[3];
  uint8_t transports_1i_1a[3]; // move remain 0,1,2
  uint8_t transports_1i_1t[3];
  uint8_t submarines[3];
  uint8_t destroyers[3];
  uint8_t carriers[3];
  uint8_t battleships[3];
  uint8_t battleships_damaged[3];
  uint8_t bombers[5]; //move remain 1,2,3,4,5
} UnitsSeaMobile;

typedef struct {
  uint8_t fighters[5];
  uint8_t trans_empty[4]; // move remain 0,1,2,s
  uint8_t trans_1i[4];
  uint8_t trans_1a[4];
  uint8_t trans_1t[4];
  uint8_t trans_2i[3];
  uint8_t trans_1i_1a[3]; // move remain 0,1,2
  uint8_t trans_1i_1t[3];
  uint8_t submarines[3];
  uint8_t destroyers[3];
  uint8_t carriers[3];
  uint8_t battleships[3];
  uint8_t bs_damaged[3];
  uint8_t bombers[6]; //move remain 1,2,3,4,5 (also including 0 for easier coding)
  uint8_t other_units[PLAYERS_COUNT - 1][SEA_UNIT_TYPES - 1]; //no parking bombers at sea
} UnitsSea;

typedef struct {
  uint8_t player_index;
  uint8_t phase;
  uint8_t money[PLAYERS_COUNT];
  LandState land_state[LANDS_COUNT];
  UnitsSea units_sea[SEAS_COUNT];
} GameData;

typedef struct {
  uint8_t units_land_type_total[LANDS_COUNT][LAND_UNIT_TYPES];
  uint8_t units_land_player_total[LANDS_COUNT][PLAYERS_COUNT];
  uint8_t units_land_grand_total[LANDS_COUNT];
  uint8_t* units_land_ptr[LANDS_COUNT][LAND_UNIT_TYPES];
  uint8_t units_sea_type_total[SEAS_COUNT][SEA_UNIT_TYPES];
  uint8_t units_sea_player_total[SEAS_COUNT][PLAYERS_COUNT];
  uint8_t units_sea_blockade_total[SEAS_COUNT][PLAYERS_COUNT];
  uint8_t units_sea_grand_total[SEAS_COUNT];
  uint8_t* units_sea_ptr[SEAS_COUNT][SEA_UNIT_TYPES];
  uint8_t income_per_turn[PLAYERS_COUNT];
  uint8_t enemies[PLAYERS_COUNT - 1];
  uint8_t enemies_count;
  char* player_names[PLAYERS_COUNT];
  char* player_colors[PLAYERS_COUNT];
} GameCache;

extern char* phases[2];
void getUserInput();
void initializeGameData();
void setPrintableStatus();
void buildCache();
void setPrintableStatusLands();
void setPrintableStatusSeas();
void stage_transport_units();
void move_land_units();
void move_transport_units();
void move_sea_units();
void move_fighter_units();
void move_bomber_units();
void resolve_sea_battles();
void unload_transports();
void bomb_factories();
void bombard_shores();
void fire_aa_guns();
void resolve_land_battles();
void land_air_units();
void move_aa_guns();
void reset_units_fully();
void buy_units();
void crash_air_units();
void collect_money();
#endif
