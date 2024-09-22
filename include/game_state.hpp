#pragma once
#include "land.hpp"
#include "player.hpp"
#include "sea.hpp"
#include "units/units.hpp"
#include <array>

//#define STRING_BUFFER_SIZE 64
#define AIRS_COUNT LANDS_COUNT + SEAS_COUNT
#define MAX_AIR_TO_AIR_CONNECTIONS 7

#define ACTIONS_COUNT 20
using Actions = std::array<uint, ACTIONS_COUNT>;
using ActionsPtr = Actions*;

using AirArray = std::array<uint, AIRS_COUNT>;
using A2AConn = std::array<uint, MAX_AIR_TO_AIR_CONNECTIONS>;
using BoolAirArray = std::array<bool, AIRS_COUNT>;

using BitField = struct { bool bit : 1; };

constexpr int ALIGNMENT_128 = 128;
using Fighterstates = std::array<uint, FIGHTER_STATES>;
using Bomberlandstates = std::array<uint, BOMBER_LAND_STATES>;
using Infantrystates = std::array<uint, INFANTRY_STATES>;
using Artillerystates = std::array<uint, ARTILLERY_STATES>;
using Tankstates = std::array<uint, TANK_STATES>;
using AAgunstates = std::array<uint, AA_GUN_STATES>;
using Bomberseastates = std::array<uint, BOMBER_SEA_STATES>;
using TransEmptystates = std::array<uint, TRANS_EMPTY_STATES>;
using Trans1istates = std::array<uint, TRANS_1I_STATES>;
using Trans1astates = std::array<uint, TRANS_1A_STATES>;
using Trans1tstates = std::array<uint, TRANS_1T_STATES>;
using Trans2istates = std::array<uint, TRANS_2I_STATES>;
using Trans1i1astates = std::array<uint, TRANS_1I_1A_STATES>;
using Trans1i1tstates = std::array<uint, TRANS_1I_1T_STATES>;
using Submarinestates = std::array<uint, SUB_STATES>;
using Destroyerstates = std::array<uint, DESTROYER_STATES>;
using Carrierstates = std::array<uint, CARRIER_STATES>;
using Cruiserstates = std::array<uint, CRUISER_STATES>;
using Battleshipstates = std::array<uint, BATTLESHIP_STATES>;

struct LandState {
  uint owner_idx; // rotates
  uint factory_dmg;
  uint factory_max;
  uint bombard_max;          // bombarded, resets
  Fighterstates fighters;    // rotates
  Bomberlandstates bombers;  // rotates
  Infantrystates infantry;   // rotates
  Artillerystates artillery; // rotates
  Tankstates tanks;          // rotates
  AAgunstates aa_guns;       // rotates
} __attribute__((aligned(ALIGNMENT_128)));

struct UnitsSea {
  Fighterstates fighters; // 0 = done moving, 1 = 1 mov left, 2 = 2 mov left, 3 = needs staging
  TransEmptystates trans_empty; // 0 = done moving, 1=0mov can unload, 2 = 1 mov left, 3 = 2 mov
                                // left, 4 = needs staging
  Trans1istates trans_1i;
  Trans1astates trans_1a;
  Trans1tstates trans_1t; // 0 = done moving, 1=0mov can unload, 2 = 1 mov left, 3 = 2 mov left
  Trans2istates trans_2i;
  Trans1i1astates trans_1i_1a;
  Trans1i1tstates trans_1i_1t;
  Submarinestates submarines;
  Destroyerstates destroyers;
  Carrierstates carriers;
  Cruiserstates cruisers;       // 0=no att, 1=0 mov can bombard, 2 = 2 mov
  Battleshipstates battleships; // 0=no att, 1=0 mov can bombard, 2 = 2 mov
  Battleshipstates bs_damaged;  // 0=no att, 1=0 mov can bombard, 2 = 2 mov
  Bomberseastates bombers;      // move remain 1,2,3,4,5
} __attribute__((aligned(ALIGNMENT_128)));

using PlayersArray = std::array<uint, PLAYERS_COUNT>;
using LandStateArray = std::array<LandState, LANDS_COUNT>;
using SeaStateArray = std::array<UnitsSea, SEAS_COUNT>;
using LandUTArray = std::array<Landunittypes, LANDS_COUNT>;
using SeaUTArray = std::array<Seaunittypes, SEAS_COUNT>;
using OplayerLandUTArray = std::array<LandUTArray, PLAYERS_COUNT - 1>;
using OplayerSeaUTArray = std::array<SeaUTArray, PLAYERS_COUNT - 1>;
using BfAirArray = std::array<BitField, AIRS_COUNT>;
using BfAirAirArray = std::array<BfAirArray, AIRS_COUNT>;

struct GameState {
  uint player_index;  // rotates
  PlayersArray money; // rotates
  AirArray builds_left;
  LandStateArray land_state;
  SeaStateArray units_sea;
  OplayerLandUTArray other_land_units; // rotates
  OplayerSeaUTArray other_sea_units;
  AirArray combat_status; // track retreats
  BfAirAirArray skipped_moves;
} __attribute__((aligned(ALIGNMENT_128)));
