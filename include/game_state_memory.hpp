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

struct GameStateMemory {
  uint current_turn{};
  uint seed{};
  PlayerArray money{};
  LandArray land_owners{};
  LandArray factory_dmg{};
  LandArray factory_max{};
  LandArray bombard_max{};
  AirArray builds_left{};
  AirArray combat_status{};
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
} __attribute__((aligned(ALIGNMENT_128)));

constexpr std::array<PlayerLandArray*, LAND_UNIT_TYPES_COUNT>
get_idle_land_units(GameStateMemory& state) {
  return {&state.idle_land_fighters,  &state.idle_land_bombers, &state.idle_land_infantry,
          &state.idle_land_artillery, &state.idle_land_tanks,   &state.idle_land_aaguns};
}

constexpr std::array<PlayerSeaArray*, SEA_UNIT_TYPES_COUNT - 1>
get_idle_sea_units(GameStateMemory& state) {
  return {&state.idle_sea_fighters,    &state.idle_sea_transempty, &state.idle_sea_trans1i,
          &state.idle_sea_trans1a,     &state.idle_sea_trans1t,    &state.idle_sea_trans2i,
          &state.idle_sea_trans1i1a,   &state.idle_sea_trans1i1t,  &state.idle_sea_submarines,
          &state.idle_sea_destroyers,  &state.idle_sea_carriers,   &state.idle_sea_cruisers,
          &state.idle_sea_battleships, &state.idle_sea_bs_damaged};
}

constexpr std::array<LandVector*, LAND_UNIT_TYPES_COUNT>
get_active_land_units(GameStateMemory& state) {
  return {&state.active_land_fighters,  &state.active_land_bombers, &state.active_land_infantry,
          &state.active_land_artillery, &state.active_land_tanks,   &state.active_land_aaguns};
}

constexpr std::array<SeaVector*, SEA_UNIT_TYPES_COUNT>
get_active_sea_units(GameStateMemory& state) {
  return {&state.active_sea_fighters,    &state.active_sea_transempty, &state.active_sea_trans1i,
          &state.active_sea_trans1a,     &state.active_sea_trans1t,    &state.active_sea_trans2i,
          &state.active_sea_trans1i1a,   &state.active_sea_trans1i1t,  &state.active_sea_submarines,
          &state.active_sea_destroyers,  &state.active_sea_carriers,   &state.active_sea_cruisers,
          &state.active_sea_battleships, &state.active_sea_bs_damaged, &state.active_sea_bombers};
}
