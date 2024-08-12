#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "land.h"
#include "player.h"
#include "sea.h"
#include "team.h"
#include "units/units.h"

#define STRING_BUFFER_SIZE 64

const char* PHASES[2] = {"Combat", "Landing, Purchase"};

typedef struct {
  uint8_t owner_index; // rotates
  uint8_t builds_left; // resets
  uint8_t factory_hp;
  uint8_t factory_max;
  bool conquered;                                          // resets
  uint8_t fighters[FIGHTER_MOVES_MAX + 1];                 // rotates
  uint8_t bombers[BOMBER_MOVES_MAX + 1];                   // rotates
  uint8_t infantry[INFANTRY_MOVES_MAX + 1];                // rotates
  uint8_t artillery[ARTILLERY_MOVES_MAX + 1];              // rotates
  uint8_t tanks[TANK_MOVES_MAX + 1];                       // rotates
  uint8_t aa_guns[AA_GUN_MOVES_MAX + 1];                   // rotates
  uint8_t other_units[PLAYERS_COUNT - 1][LAND_UNIT_TYPES]; // rotates
} LandState;

typedef struct {
  uint8_t fighters[FIGHTER_MOVES_MAX + 1];
  uint8_t trans_empty[TRANSPORT_MOVES_MAX + 2]; // move remain 0,1,2,s
  uint8_t trans_1i[TRANSPORT_MOVES_MAX + 2];
  uint8_t trans_1a[TRANSPORT_MOVES_MAX + 2];
  uint8_t trans_1t[TRANSPORT_MOVES_MAX + 2];
  uint8_t trans_2i[TRANSPORT_MOVES_MAX + 1]; // move remain 0,1,2
  uint8_t trans_1i_1a[TRANSPORT_MOVES_MAX + 1];
  uint8_t trans_1i_1t[TRANSPORT_MOVES_MAX + 1];
  uint8_t submarines[SUB_MOVES_MAX + 1];
  uint8_t destroyers[DESTROYER_MOVES_MAX + 1];
  uint8_t carriers[CARRIER_MOVES_MAX + 1];
  uint8_t battleships[BATTLESHIP_MOVES_MAX + 1];
  uint8_t bs_damaged[BATTLESHIP_MOVES_MAX + 1];
  uint8_t
      bombers[6]; // move remain 1,2,3,4,5 (also including 0 for easier coding)
  uint8_t other_units[PLAYERS_COUNT - 1]
                     [SEA_UNIT_TYPES - 1]; // no parking bombers at sea
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

void getUserInput();
void getAIInput();
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
