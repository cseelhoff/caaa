#pragma once
#pragma once
#include "land.hpp"
#include "game_state_memory.hpp"
#include "player.hpp"
#include "sea.hpp"
#include "units/units.hpp"
#include <array>
#include <nlohmann/json.hpp>
#include <vector>

using json = nlohmann::json;

using BitField = struct { bool bit : 1; };

using Fighterstates = std::array<uint, FIGHTER_STATES>;
using Bomberlandstates = std::array<uint, BOMBER_LAND_STATES>;
using Infantrystates = std::array<uint, INFANTRY_STATES>;
using Artillerystates = std::array<uint, ARTILLERY_STATES>;
using Tankstates = std::array<uint, TANK_STATES>;
using AAgunstates = std::array<uint, AA_GUN_STATES>;
using Bomberseastates = std::array<uint, BOMBER_SEA_STATES>;
using TransEmptystates = std::array<uint, TRANSEMPTY_STATES>;
using Trans1istates = std::array<uint, TRANS_1I_STATES>;
using Trans1astates = std::array<uint, TRANS_1A_STATES>;
using Trans1tstates = std::array<uint, TRANS_1T_STATES>;
using Trans2istates = std::array<uint, TRANS_2I_STATES>;
using Trans1i1astates = std::array<uint, TRANS1I1A_STATES>;
using Trans1i1tstates = std::array<uint, TRANS_1I_1T_STATES>;
using Submarinestates = std::array<uint, SUB_STATES>;
using Destroyerstates = std::array<uint, DESTROYER_STATES>;
using Carrierstates = std::array<uint, CARRIER_STATES>;
using Cruiserstates = std::array<uint, CRUISER_STATES>;
using Battleshipstates = std::array<uint, BATTLESHIP_STATES>;
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
  AAgunstates aaguns;
} __attribute__((aligned(ALIGNMENT_128)));

struct IdleLandUnits {
  uint owner_idx;
  uint fighters;
  uint bombers;
  uint infantry;
  uint artillery;
  uint tanks;
  uint aaguns;
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
  TransEmptystates transempty; // 0 = done moving, 1 = 1 mov left, 2 = 2 mov left, 3 = needs staging
  Trans1istates trans1i;// 0 = done moving, 1=0mov can unload, 2 = 1 mov left, 3 = 2 mov, 4 = needs staging
  Trans1astates trans1a;
  Trans1tstates trans1t;
  Trans2istates trans2i;// 0 = done moving, 1=0mov can unload, 2 = 1 mov left, 3 = 2 mov left
  Trans1i1astates trans1i1a;
  Trans1i1tstates trans1i1t;
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
  uint transempty;
  uint trans1i;
  uint trans1a;
  uint trans1t;
  uint trans2i;
  uint trans1i1a;
  uint trans1i1t;
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
  PlayerArray money{};
  LandStateArray land_states;
  SeaStateArray sea_states;
} __attribute__((aligned(ALIGNMENT_128)));

void convert_json_to_memory(GameStateJson jsonState, GameStateMemory memState);

// Serialization and deserialization functions for GameState
bool load_game_state_from_json(const std::string& filename, GameStateJson& gameState);
void to_json(json& jsonData, const GameStateJson& gameState);
void from_json(const json& jsonData, GameStateJson& gameState);
