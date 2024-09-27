#pragma once
#include "array_functions.hpp"
#include "land.hpp"
#include "map_cache.hpp"
#include "player.hpp"
#include "sea.hpp"
#include "units/units.hpp"
#include <array>
#include <cmath>
#include <string>

constexpr uint MAX_LAND_UNIT_STATES = std::max({FIGHTER_STATES, BOMBER_LAND_STATES, INFANTRY_STATES,
                                                ARTILLERY_STATES, TANK_STATES, AA_GUN_STATES});
using LandVector = std::array<std::vector<uint>, LANDS_COUNT>;
using SeaVector = std::array<std::vector<uint>, SEAS_COUNT>;
using BoolAirArray = std::array<bool, AIRS_COUNT>;
using BoolAirAirArray = std::array<BoolAirArray, AIRS_COUNT>;
using PlayerLandArray = Uint2DArray<PLAYERS_COUNT, LANDS_COUNT>;
using PlayerSeaArray = Uint2DArray<PLAYERS_COUNT, SEAS_COUNT>;
using TeamAirArray = Uint2DArray<TEAMS_COUNT, AIRS_COUNT>;
using PlayerAirArray = Uint2DArray<PLAYERS_COUNT, AIRS_COUNT>;
using BoolSeaArray = std::array<bool, SEAS_COUNT>;
using BoolLandArray = std::array<bool, LANDS_COUNT>;
using BoolLandLandArray = Bool2DArray<LANDS_COUNT, LANDS_COUNT>;
using BoolSeaSeaArray = Bool2DArray<SEAS_COUNT, SEAS_COUNT>;

struct GameCache {
  TeamAirArray team_units_count;
  PlayerArray income_per_turn;
  PlayerArray total_factory_count;
  PlayerLandArray factory_locations;
  PlayerAirArray total_player_units;
  SeaArray enemy_destroyers_total;
  SeaArray enemy_blockade_total;
  SeaArray allied_carriers;
  SeaArray transports_with_large_cargo_space;
  SeaArray transports_with_small_cargo_space;
  BoolLandLandArray land_path_blocked;
  BoolSeaSeaArray sea_path_blocked;
  BoolSeaSeaArray sub_path_blocked;
  BoolAirArray canFighterLandHere;
  BoolAirArray canFighterLandIn1Move;
  BoolAirArray canBomberLandHere;
  BoolAirArray canBomberLandIn1Move;
  BoolAirArray canBomberLandIn2Moves;
  uint canal_state;
  std::vector<uint> valid_moves;
  uint step_id;
  uint answers_remaining;
  uint selected_action;
  uint unlucky_player_idx;
  uint max_loops;
  bool actually_print;
} __attribute__((aligned(ALIGNMENT_128)));

enum CombatStatus : uint {
  NO_COMBAT = 0,
  MID_COMBAT = 1,
  PRE_COMBAT = 2,
};

using CombatStatusArray = std::array<CombatStatus, AIRS_COUNT>;

struct GameState {
  uint current_turn{};
  uint seed{};
  PlayerArray money{};
  LandArray land_owners{};
  LandArray factory_dmg{};
  LandArray factory_max{};
  LandArray bombard_max{};
  AirArray builds_left{};
  CombatStatusArray combat_status{};
  BoolAirAirArray skipped_moves{};
  LandVector active_land_fighters{};
  LandVector active_land_bombers{};
  LandVector active_land_infantry{};
  LandVector active_land_artillery{};
  LandVector active_land_tanks{};
  LandVector active_land_aaguns{};
  SeaVector active_sea_fighters{};
  SeaVector active_sea_transempty{};
  SeaVector active_sea_trans1i{};
  SeaVector active_sea_trans1a{};
  SeaVector active_sea_trans1t{};
  SeaVector active_sea_trans2i{};
  SeaVector active_sea_trans1i1a{};
  SeaVector active_sea_trans1i1t{};
  SeaVector active_sea_submarines{};
  SeaVector active_sea_destroyers{};
  SeaVector active_sea_carriers{};
  SeaVector active_sea_cruisers{};
  SeaVector active_sea_battleships{};
  SeaVector active_sea_bs_damaged{};
  SeaVector active_sea_bombers{};
  PlayerLandArray idle_land_fighters{};
  PlayerLandArray idle_land_bombers{};
  PlayerLandArray idle_land_infantry{};
  PlayerLandArray idle_land_artillery{};
  PlayerLandArray idle_land_tanks{};
  PlayerLandArray idle_land_aaguns{};
  PlayerSeaArray idle_sea_fighters{};
  PlayerSeaArray idle_sea_transempty{};
  PlayerSeaArray idle_sea_trans1i{};
  PlayerSeaArray idle_sea_trans1a{};
  PlayerSeaArray idle_sea_trans1t{};
  PlayerSeaArray idle_sea_trans2i{};
  PlayerSeaArray idle_sea_trans1i1a{};
  PlayerSeaArray idle_sea_trans1i1t{};
  PlayerSeaArray idle_sea_submarines{};
  PlayerSeaArray idle_sea_destroyers{};
  PlayerSeaArray idle_sea_carriers{};
  PlayerSeaArray idle_sea_cruisers{};
  PlayerSeaArray idle_sea_battleships{};
  PlayerSeaArray idle_sea_bs_damaged{};
  GameCache cache{};
} __attribute__((aligned(ALIGNMENT_128)));

constexpr std::array<PlayerLandArray*, LAND_UNIT_TYPES_COUNT>
get_idle_land_units(GameState& state) {
  return {&state.idle_land_fighters,  &state.idle_land_bombers, &state.idle_land_infantry,
          &state.idle_land_artillery, &state.idle_land_tanks,   &state.idle_land_aaguns};
}

constexpr std::array<PlayerSeaArray*, SEA_UNIT_TYPES_COUNT - 1>
get_idle_sea_units(GameState& state) {
  return {&state.idle_sea_fighters,    &state.idle_sea_transempty, &state.idle_sea_trans1i,
          &state.idle_sea_trans1a,     &state.idle_sea_trans1t,    &state.idle_sea_trans2i,
          &state.idle_sea_trans1i1a,   &state.idle_sea_trans1i1t,  &state.idle_sea_submarines,
          &state.idle_sea_destroyers,  &state.idle_sea_carriers,   &state.idle_sea_cruisers,
          &state.idle_sea_battleships, &state.idle_sea_bs_damaged};
}

constexpr uint& get_idle_fighter_units(GameState& state, uint player_idx, uint air_idx) {
  if (air_idx < LANDS_COUNT) {
    return state.idle_land_fighters.ref(player_idx, air_idx);
  }
  return state.idle_sea_fighters.ref(player_idx, air_idx - LANDS_COUNT);
}

constexpr std::array<LandVector*, LAND_UNIT_TYPES_COUNT> get_active_land_units(GameState& state) {
  return {&state.active_land_fighters,  &state.active_land_bombers, &state.active_land_infantry,
          &state.active_land_artillery, &state.active_land_tanks,   &state.active_land_aaguns};
}

constexpr std::array<SeaVector*, SEA_UNIT_TYPES_COUNT> get_active_sea_units(GameState& state) {
  return {&state.active_sea_fighters,    &state.active_sea_transempty, &state.active_sea_trans1i,
          &state.active_sea_trans1a,     &state.active_sea_trans1t,    &state.active_sea_trans2i,
          &state.active_sea_trans1i1a,   &state.active_sea_trans1i1t,  &state.active_sea_submarines,
          &state.active_sea_destroyers,  &state.active_sea_carriers,   &state.active_sea_cruisers,
          &state.active_sea_battleships, &state.active_sea_bs_damaged, &state.active_sea_bombers};
}

constexpr std::vector<uint>& get_active_fighter_units(GameState& state, uint air_idx) {
  if (air_idx < LANDS_COUNT) {
    return state.active_land_fighters.at(air_idx);
  }
  return state.active_sea_fighters.at(air_idx - LANDS_COUNT);
}

constexpr std::vector<uint>& get_active_bomber_units(GameState& state, uint air_idx) {
  if (air_idx < LANDS_COUNT) {
    return state.active_land_bombers.at(air_idx);
  }
  return state.active_sea_bombers.at(air_idx - LANDS_COUNT);
}

void refresh_economy(GameState& state);
void refresh_full_cache(GameState& state);
void refresh_eot_cache(GameState& state);
void refresh_land_armies(GameState& state);
void refresh_sea_navies(GameState& state);
void refresh_allies(GameState& state);
void refresh_canals(GameState& state);
void refresh_fleets(GameState& state);
void refresh_land_path_blocked(GameState& state);
void refresh_sea_path_blocked(GameCache& cache);
void refresh_transports_with_cargo_space(GameState& state, uint player_idx,
                                         uint sea_idx);

std::string get_printable_status(const GameState& state);
std::string get_printable_status_lands(const GameState& state);
std::string get_printable_status_seas(const GameState& state);
