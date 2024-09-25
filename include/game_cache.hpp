#pragma once
#include "array_functions.hpp"
#include "engine.hpp"
#include "game_state_memory.hpp"
#include "land.hpp"
#include <array>

using TeamAirArray = Uint2DArray<TEAMS_COUNT, AIRS_COUNT>;
using PlayerAirArray = Uint2DArray<PLAYERS_COUNT, AIRS_COUNT>;
using BoolSeaArray = std::array<bool, SEAS_COUNT>;
using BoolLandArray = std::array<bool, LANDS_COUNT>;
using BoolLandLandArray = Bool2DArray<LANDS_COUNT, LANDS_COUNT>;
using BoolSeaSeaArray = Bool2DArray<SEAS_COUNT, SEAS_COUNT>;
// using BoolLandArray = std::array<bool, LANDS_COUNT>;
struct GameCache {
  TeamAirArray team_units_count;
  PlayerArray income_per_turn;
  PlayerArray total_factory_count;
  PlayerLandArray factory_locations;
  PlayerAirArray total_player_units;
  AirArray enemy_units_count;
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
} __attribute__((aligned(ALIGNMENT_128)));

void refresh_economy(GameStateMemory& state, GameCache& cache);
void refresh_full_cache(GameStateMemory& state, GameCache& cache);
void refresh_eot_cache(GameStateMemory& state, GameCache& cache);
void refresh_land_armies(GameStateMemory& state, GameCache& cache);
void refresh_sea_navies(GameStateMemory& state, GameCache& cache);
void refresh_allies(GameStateMemory& state, GameCache& cache);
void refresh_canals(GameStateMemory& state, GameCache& cache);
void refresh_fleets(GameStateMemory& state, GameCache& cache);
void refresh_land_path_blocked(GameStateMemory& state, GameCache& cache);
void refresh_sea_path_blocked(GameCache& cache);
