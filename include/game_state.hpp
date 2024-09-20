#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "land.h"
#include "player.h"
#include "sea.h"
#include "units/units.h"
#include <array>

//#define STRING_BUFFER_SIZE 64
#define AIRS_COUNT LANDS_COUNT + SEAS_COUNT
#define MAX_AIR_TO_AIR_CONNECTIONS 7

#define ACTIONS_COUNT 20
using Actions = std::array<uint, ACTIONS_COUNT>;
using ActionsPtr = Actions *;

using AirArray = std::array<uint, AIRS_COUNT>;
using A2AConn = std::array<uint, MAX_AIR_TO_AIR_CONNECTIONS>;

using BitField = struct {
    bool bit : 1;
};

constexpr int ALIGNMENT_128 = 128;

struct LandState {
  uint owner_idx; // rotates
  uint factory_dmg;
  uint factory_max;
  uint bombard_max; // bombarded, resets
  std::array<uint, FIGHTER_STATES> fighters;                        // rotates
  std::array<uint, BOMBER_LAND_STATES> bombers;                     // rotates
  std::array<uint, INFANTRY_STATES> infantry;                       // rotates
  std::array<uint, ARTILLERY_STATES> artillery;                     // rotates
  std::array<uint, TANK_STATES> tanks;                              // rotates
  std::array<uint, AA_GUN_STATES> aa_guns;                          // rotates
} __attribute__((aligned(ALIGNMENT_128)));

struct UnitsSea {
  std::array<uint, FIGHTER_STATES> fighters;
  // 0 = done moving, 1 = 1 mov left, 2 = 2 mov left, 3 = needs staging
  std::array<uint, TRANS_EMPTY_STATES> trans_empty;
  // 0 = done moving, 1=0mov can unload, 2 = 1 mov left, 3 = 2 mov left, 4 = needs staging
  std::array<uint, TRANS_1I_STATES> trans_1i;
  std::array<uint, TRANS_1A_STATES> trans_1a;
  std::array<uint, TRANS_1T_STATES> trans_1t;
  // 0 = done moving, 1=0mov can unload, 2 = 1 mov left, 3 = 2 mov left
  std::array<uint, TRANS_2I_STATES> trans_2i;
  std::array<uint, TRANS_1I_1A_STATES> trans_1i_1a;
  std::array<uint, TRANS_1I_1T_STATES> trans_1i_1t;
  std::array<uint, SUB_STATES> submarines;
  std::array<uint, DESTROYER_STATES> destroyers;
  std::array<uint, CARRIER_STATES> carriers;
  std::array<uint, CRUISER_STATES> cruisers;       // 0=no att, 1=0 mov can bombard, 2 = 2 mov
  std::array<uint, BATTLESHIP_STATES> battleships; // 0=no att, 1=0 mov can bombard, 2 = 2 mov
  std::array<uint, BATTLESHIP_STATES> bs_damaged;  // 0=no att, 1=0 mov can bombard, 2 = 2 mov
  std::array<uint, BOMBER_SEA_STATES> bombers;     // move remain 1,2,3,4,5
} __attribute__((aligned(ALIGNMENT_128)));

struct GameState {
  uint player_index; // rotates
  std::array<uint, PLAYERS_COUNT> money; // rotates
  std::array<uint, AIRS_COUNT> builds_left; 
  std::array<LandState, LANDS_COUNT> land_state;
  std::array<UnitsSea, SEAS_COUNT> units_sea;
  std::array<std::array<std::array<uint, LAND_UNIT_TYPES_COUNT>, LANDS_COUNT>, PLAYERS_COUNT - 1> other_land_units; // rotates
  std::array<std::array<std::array<uint, SEA_UNIT_TYPES_COUNT>, SEAS_COUNT>, PLAYERS_COUNT - 1> other_sea_units; 
  std::array<uint, AIRS_COUNT> flagged_for_combat; // track retreats
  std::array<std::array<BitField, AIRS_COUNT>, AIRS_COUNT> skipped_moves;
} __attribute__((aligned(ALIGNMENT_128)));
#endif
