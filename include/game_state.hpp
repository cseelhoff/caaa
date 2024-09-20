#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "land.h"
#include "player.h"
#include "sea.h"
#include "units/units.h"
#include <array>
#include <stdint.h>

//#define STRING_BUFFER_SIZE 64
#define AIRS_COUNT LANDS_COUNT + SEAS_COUNT
#define MAX_AIR_TO_AIR_CONNECTIONS 7

#define ACTIONS_COUNT 20
using Actions = std::array<int, ACTIONS_COUNT>;
using ActionsPtr = Actions *;

using AirArray = std::array<int, AIRS_COUNT>;
using A2AConn = std::array<int, MAX_AIR_TO_AIR_CONNECTIONS>;

using BitField = struct {
    bool bit : 1;
};

constexpr int ALIGNMENT_128 = 128;

struct LandState {
  int owner_idx; // rotates
  int factory_hp;
  int factory_max;
  int bombard_max; // bombarded, resets
  std::array<int, FIGHTER_STATES> fighters;                        // rotates
  std::array<int, BOMBER_LAND_STATES> bombers;                     // rotates
  std::array<int, INFANTRY_STATES> infantry;                       // rotates
  std::array<int, ARTILLERY_STATES> artillery;                     // rotates
  std::array<int, TANK_STATES> tanks;                              // rotates
  std::array<int, AA_GUN_STATES> aa_guns;                          // rotates
} __attribute__((aligned(ALIGNMENT_128)));

struct UnitsSea {
  std::array<int, FIGHTER_STATES> fighters;
  // 0 = done moving, 1 = 1 mov left, 2 = 2 mov left, 3 = needs staging
  std::array<int, TRANS_EMPTY_STATES> trans_empty;
  // 0 = done moving, 1=0mov can unload, 2 = 1 mov left, 3 = 2 mov left, 4 = needs staging
  std::array<int, TRANS_1I_STATES> trans_1i;
  std::array<int, TRANS_1A_STATES> trans_1a;
  std::array<int, TRANS_1T_STATES> trans_1t;
  // 0 = done moving, 1=0mov can unload, 2 = 1 mov left, 3 = 2 mov left
  std::array<int, TRANS_2I_STATES> trans_2i;
  std::array<int, TRANS_1I_1A_STATES> trans_1i_1a;
  std::array<int, TRANS_1I_1T_STATES> trans_1i_1t;
  std::array<int, SUB_STATES> submarines;
  std::array<int, DESTROYER_STATES> destroyers;
  std::array<int, CARRIER_STATES> carriers;
  std::array<int, CRUISER_STATES> cruisers;       // 0=no att, 1=0 mov can bombard, 2 = 2 mov
  std::array<int, BATTLESHIP_STATES> battleships; // 0=no att, 1=0 mov can bombard, 2 = 2 mov
  std::array<int, BATTLESHIP_STATES> bs_damaged;  // 0=no att, 1=0 mov can bombard, 2 = 2 mov
  std::array<int, BOMBER_SEA_STATES> bombers;     // move remain 1,2,3,4,5
} __attribute__((aligned(ALIGNMENT_128)));

struct GameState {
  int player_index; // rotates
  std::array<int, PLAYERS_COUNT> money; // rotates
  std::array<int, AIRS_COUNT> builds_left; 
  std::array<LandState, LANDS_COUNT> land_state;
  std::array<UnitsSea, SEAS_COUNT> units_sea;
  std::array<std::array<std::array<int, LAND_UNIT_TYPES_COUNT>, LANDS_COUNT>, PLAYERS_COUNT - 1> other_land_units; // rotates
  std::array<std::array<std::array<int, SEA_UNIT_TYPES_COUNT>, SEAS_COUNT>, PLAYERS_COUNT - 1> other_sea_units; 
  std::array<int, AIRS_COUNT> flagged_for_combat; // track retreats
  std::array<std::array<BitField, AIRS_COUNT>, AIRS_COUNT> skipped_moves;
} __attribute__((aligned(ALIGNMENT_128)));
#endif
