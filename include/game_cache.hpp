#pragma once
#include "array_functions.hpp"
#include "game_state_memory.hpp"

using TeamAirArray = Uint2DArray<TEAMS_COUNT, AIRS_COUNT>;
using PlayerAirArray = Uint2DArray<PLAYERS_COUNT, AIRS_COUNT>;
struct GameCache {
  TeamAirArray team_units_count;
  PlayerArray income_per_turn;
  PlayerArray total_factory_count;
  PlayerLandArray factory_locations;
  PlayerAirArray total_player_units;
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
void refresh_sea_path_blocked(GameStateMemory& state, GameCache& cache);
