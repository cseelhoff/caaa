#pragma once
#include "land.hpp"
#include "player.hpp"
#include "sea.hpp"
#include "units/units.hpp"
#include <array>
#include <vector>

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

constexpr uint ALIGNMENT_128 = 128;
constexpr uint ALIGNMENT_64 = 64;
constexpr uint ALIGNMENT_32 = 32;
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
using PlayersArray = std::array<uint, PLAYERS_COUNT>;
using LandUTArray = std::array<Landunittypes, LANDS_COUNT>;
using SeaUTArray = std::array<Seaunittypes, SEAS_COUNT>;
using OplayerLandUTArray = std::array<LandUTArray, PLAYERS_COUNT - 1>;
using OplayerSeaUTArray = std::array<SeaUTArray, PLAYERS_COUNT - 1>;
using BfAirArray = std::array<BitField, AIRS_COUNT>;
using SkippedAirVector = std::vector<uint>;

struct ActiveLandUnits {
  Fighterstates fighters;
  Bomberlandstates bombers;
  Infantrystates infantry;
  Artillerystates artillery;
  Tankstates tanks;
  AAgunstates aa_guns;
} __attribute__((aligned(ALIGNMENT_128)));

struct IdleLandUnits {
  uint owner_idx;
  uint fighters;
  uint bombers;
  uint infantry;
  uint artillery;
  uint tanks;
  uint aa_guns;
} __attribute__((aligned(ALIGNMENT_32)));

struct LandStateJson {
  uint owner_idx;
  uint factory_dmg;
  uint factory_max;
  uint bombard_max;
  uint builds_left;
  uint combat_status;
  SkippedAirVector skipped_moves;
  ActiveLandUnits active_land_units;
  IdleLandUnits idle_land_units;
} __attribute__((aligned(ALIGNMENT_128)));

struct ActiveSeaUnits {
  Fighterstates fighters;
  TransEmptystates trans_empty; // 0 = done moving, 1 = 1 mov left, 2 = 2 mov left, 3 = needs staging
  Trans1istates trans_1i;// 0 = done moving, 1=0mov can unload, 2 = 1 mov left, 3 = 2 mov, 4 = needs staging
  Trans1astates trans_1a;
  Trans1tstates trans_1t;
  Trans2istates trans_2i;// 0 = done moving, 1=0mov can unload, 2 = 1 mov left, 3 = 2 mov left
  Trans1i1astates trans_1i_1a;
  Trans1i1tstates trans_1i_1t;
  Submarinestates submarines;
  Destroyerstates destroyers;
  Carrierstates carriers;
  Cruiserstates cruisers;// 0=no att, 1=0 mov can bombard, 2 = 2 mov
  Battleshipstates battleships;
  Battleshipstates bs_damaged;
  Bomberseastates bombers;// move remain 1,2,3,4,5
} __attribute__((aligned(ALIGNMENT_128)));

struct IdleSeaUnits {
  uint owner_idx;
  uint fighters;
  uint trans_empty;
  uint trans_1i;
  uint trans_1a;
  uint trans_1t;
  uint trans_2i;
  uint trans_1i_1a;
  uint trans_1i_1t;
  uint submarines;
  uint destroyers;
  uint carriers;
  uint cruisers;
  uint battleships;
  uint bs_damaged;
}__attribute__((aligned(ALIGNMENT_64)));

using IdleSeaUnitsVector = std::vector<IdleSeaUnits>;

struct SeaStateJson {
  uint builds_left;
  uint combat_status;
  SkippedAirVector skipped_moves;
  ActiveSeaUnits active_sea_units;
  IdleSeaUnitsVector idle_sea_units;
} __attribute__((aligned(ALIGNMENT_128)));

using LandStateArray = std::array<LandStateJson, LANDS_COUNT>;
using SeaStateArray = std::array<SeaStateJson, SEAS_COUNT>;

struct GameStateJson {
  uint current_turn{};
  uint seed{};
  PlayersArray money{};
  LandStateArray land_states;
  SeaStateArray sea_states;
} __attribute__((aligned(ALIGNMENT_128)));
