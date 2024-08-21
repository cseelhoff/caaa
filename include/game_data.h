#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "canal.h"
#include "land.h"
#include "player.h"
#include "sea.h"
#include "team.h"
#include "units/units.h"
#include <stdint.h>

//#define STRING_BUFFER_SIZE 64
#define AIRS_COUNT LANDS_COUNT + SEAS_COUNT
#define MAX_AIR_TO_AIR_CONNECTIONS 7

typedef struct {
  uint8_t owner_idx; // rotates
  int8_t factory_hp;
  uint8_t factory_max;
  uint8_t bombard_max; // bombarded, resets
  // bool no_airstrip;                                 // resets
  // bool flagged_for_combat;                                // resets
  uint8_t fighters[FIGHTER_STATES];                        // rotates
  uint8_t bombers[BOMBER_LAND_STATES];                     // rotates
  uint8_t infantry[INFANTRY_STATES];                       // rotates
  uint8_t artillery[ARTILLERY_STATES];                     // rotates
  uint8_t tanks[TANK_STATES];                              // rotates
  uint8_t aa_guns[AA_GUN_STATES];                          // rotates
  uint8_t other_units[PLAYERS_COUNT - 1][LAND_UNIT_TYPES]; // rotates
} LandState;

typedef struct {
  uint8_t fighters[FIGHTER_STATES];
  // 0 = done moving, 1 = 1 mov left, 2 = 2 mov left, 3 = needs staging
  uint8_t trans_empty[TRANS_EMPTY_STATES];
  // 0 = done moving, 1=0mov can unload, 2 = 1 mov left, 3 = 2 mov left, 4 = needs staging
  uint8_t trans_1i[TRANS_1I_STATES];
  uint8_t trans_1a[TRANS_1A_STATES];
  uint8_t trans_1t[TRANS_1T_STATES];
  // 0 = done moving, 1=0mov can unload, 2 = 1 mov left, 3 = 2 mov left
  uint8_t trans_2i[TRANS_2I_STATES];
  uint8_t trans_1i_1a[TRANS_1I_1A_STATES];
  uint8_t trans_1i_1t[TRANS_1I_1T_STATES];
  uint8_t submarines[SUB_STATES];
  uint8_t destroyers[DESTROYER_STATES];
  uint8_t carriers[CARRIER_STATES];
  uint8_t cruisers[CRUISER_STATES];       // 0=no att, 1=0 mov can bombard, 2 = 2 mov
  uint8_t battleships[BATTLESHIP_STATES]; // 0=no att, 1=0 mov can bombard, 2 = 2 mov
  uint8_t bs_damaged[BATTLESHIP_STATES];  // 0=no att, 1=0 mov can bombard, 2 = 2 mov
  uint8_t bombers[BOMBER_SEA_STATES];     // move remain 1,2,3,4,5
  uint8_t other_units[PLAYERS_COUNT - 1][SEA_UNIT_TYPES - 1]; // no parking bombers at sea
} UnitsSea;

typedef struct {
  uint8_t player_index;
  // uint8_t phase;
  uint8_t money[PLAYERS_COUNT];
  uint8_t builds_left[AIRS_COUNT];
  LandState land_state[LANDS_COUNT];
  UnitsSea units_sea[SEAS_COUNT];
  bool flagged_for_combat[AIRS_COUNT]; // track retreats
} GameData;

typedef struct {
  // uint8_t units_land_type_total[LANDS_COUNT][LAND_UNIT_TYPES];
  uint8_t units_land_player_total[LANDS_COUNT][PLAYERS_COUNT];
  uint8_t units_land_grand_total[LANDS_COUNT];
  uint8_t* units_land_ptr[LANDS_COUNT][LAND_UNIT_TYPES];
  // uint8_t units_sea_type_total[SEAS_COUNT][SEA_UNIT_TYPES];
  uint8_t units_sea_player_total[SEAS_COUNT][PLAYERS_COUNT];
  uint8_t units_sea_blockade_total[SEAS_COUNT][PLAYERS_COUNT];
  uint8_t units_sea_grand_total[SEAS_COUNT];
  uint8_t* units_sea_ptr[SEAS_COUNT][SEA_UNIT_TYPES];
  uint8_t* units_air_ptr[AIRS_COUNT][AIR_UNIT_TYPES_COUNT];
  uint8_t income_per_turn[PLAYERS_COUNT];
  uint8_t enemies[PLAYERS_COUNT - 1];
  uint8_t enemies_count;
  uint8_t canal_state;
  char* player_names[PLAYERS_COUNT];
  char* player_colors[PLAYERS_COUNT];
} GameCache;

#endif
