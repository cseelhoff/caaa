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

using BoolAirArray = std::array<bool, AIRS_COUNT>;
using ActiveLandFighters = Uint2DArray<LANDS_COUNT, FIGHTER_STATES>;
using ActiveLandBombers = Uint2DArray<LANDS_COUNT, BOMBER_LAND_STATES>;
using ActiveLandInfantry = Uint2DArray<LANDS_COUNT, INFANTRY_STATES>;
using ActiveLandArtillery = Uint2DArray<LANDS_COUNT, ARTILLERY_STATES>;
using ActiveLandTanks = Uint2DArray<LANDS_COUNT, TANK_STATES>;
using ActiveLandAAGuns = Uint2DArray<LANDS_COUNT, AA_GUN_STATES>;
using ActiveSeaFighters = Uint2DArray<SEAS_COUNT, FIGHTER_STATES>;
using ActiveSeaTransEmpty = Uint2DArray<SEAS_COUNT, TRANSEMPTY_STATES>;
using ActiveSeaTrans1I = Uint2DArray<SEAS_COUNT, TRANS_1I_STATES>;
using ActiveSeaTrans1A = Uint2DArray<SEAS_COUNT, TRANS_1A_STATES>;
using ActiveSeaTrans1T = Uint2DArray<SEAS_COUNT, TRANS_1T_STATES>;
using ActiveSeaTrans2I = Uint2DArray<SEAS_COUNT, TRANS_2I_STATES>;
using ActiveSeaTrans1I1A = Uint2DArray<SEAS_COUNT, TRANS1I1A_STATES>;
using ActiveSeaTrans1I1T = Uint2DArray<SEAS_COUNT, TRANS_1I_1T_STATES>;
using ActiveSeaSubmarines = Uint2DArray<SEAS_COUNT, SUB_STATES>;
using ActiveSeaDestroyers = Uint2DArray<SEAS_COUNT, DESTROYER_STATES>;
using ActiveSeaCarriers = Uint2DArray<SEAS_COUNT, CARRIER_STATES>;
using ActiveSeaCruisers = Uint2DArray<SEAS_COUNT, CRUISER_STATES>;
using ActiveSeaBattleships = Uint2DArray<SEAS_COUNT, BATTLESHIP_STATES>;
using ActiveSeaBSDamaged = Uint2DArray<SEAS_COUNT, BATTLESHIP_STATES>;
using ActiveSeaBombers = Uint2DArray<SEAS_COUNT, BOMBER_SEA_STATES>;
using PlayerLandArray = Uint2DArray<PLAYERS_COUNT, LANDS_COUNT>;
using PlayerSeaArray = Uint2DArray<PLAYERS_COUNT, SEAS_COUNT>;
using LandUTArray = std::array<Landunittypes, LANDS_COUNT>;
using SeaUTArray = std::array<Seaunittypes, SEAS_COUNT>;

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
  BoolAirArray skipped_moves{};
  ActiveLandFighters active_land_fighters{};
  ActiveLandBombers active_land_bombers{};
  ActiveLandInfantry active_land_infantry{};
  ActiveLandArtillery active_land_artillery{};
  ActiveLandTanks active_land_tanks{};
  ActiveLandAAGuns active_land_aaguns{};
  ActiveSeaFighters active_sea_fighters{};
  ActiveSeaTransEmpty active_sea_transempty{};
  ActiveSeaTrans1I active_sea_trans1i{};
  ActiveSeaTrans1A active_sea_trans1a{};
  ActiveSeaTrans1T active_sea_trans1t{};
  ActiveSeaTrans2I active_sea_trans2i{};
  ActiveSeaTrans1I1A active_sea_trans1i1a{};
  ActiveSeaTrans1I1T active_sea_trans1i1t{};
  ActiveSeaSubmarines active_sea_submarines{};
  ActiveSeaDestroyers active_sea_destroyers{};
  ActiveSeaCarriers active_sea_carriers{};
  ActiveSeaCruisers active_sea_cruisers{};
  ActiveSeaBattleships active_sea_battleships{};
  ActiveSeaBSDamaged active_sea_bs_damaged{};
  ActiveSeaBombers active_sea_bombers{};
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

constexpr std::array<const void*, LAND_UNIT_TYPES_COUNT> get_idle_land_units(const GameStateMemory& state) {
  return {&state.idle_land_fighters, &state.idle_land_bombers, &state.idle_land_infantry,
          &state.idle_land_artillery, &state.idle_land_tanks, &state.idle_land_aaguns};
}

constexpr std::array<const void*, SEA_UNIT_TYPES_COUNT> get_idle_sea_units(const GameStateMemory& state) {
  return {&state.idle_sea_fighters, &state.idle_sea_transempty, &state.idle_sea_trans1i, &state.idle_sea_trans1a,
          &state.idle_sea_trans1t, &state.idle_sea_trans2i, &state.idle_sea_trans1i1a, &state.idle_sea_trans1i1t,
          &state.idle_sea_submarines, &state.idle_sea_destroyers, &state.idle_sea_carriers, &state.idle_sea_cruisers,
          &state.idle_sea_battleships, &state.idle_sea_bs_damaged};
}

constexpr std::array<const void*, LAND_UNIT_TYPES_COUNT> get_active_land_units(const GameStateMemory& state) {
  auto aaa =
      std::array<const void*, LAND_UNIT_TYPES_COUNT>{
          &state.active_land_fighters,  &state.active_land_bombers, &state.active_land_infantry,
          &state.active_land_artillery, &state.active_land_tanks,   &state.active_land_aaguns};
  std::initializer_list<const std::array<unsigned int, LANDS_COUNT * BOMBER_LAND_STATES>*> bbb = {
      static_cast<const std::array<uint, LANDS_COUNT * BOMBER_LAND_STATES>*>(aaa[0]),
      static_cast<const std::array<uint, LANDS_COUNT * BOMBER_LAND_STATES>*>(aaa[1])};
}

constexpr std::array<const void*, SEA_UNIT_TYPES_COUNT - 1> get_active_sea_units(const GameStateMemory& state) {
  return {&state.active_sea_fighters, &state.active_sea_transempty, &state.active_sea_trans1i, &state.active_sea_trans1a,
          &state.active_sea_trans1t, &state.active_sea_trans2i, &state.active_sea_trans1i1a, &state.active_sea_trans1i1t,
          &state.active_sea_submarines, &state.active_sea_destroyers, &state.active_sea_carriers, &state.active_sea_cruisers,
          &state.active_sea_battleships, &state.active_sea_bs_damaged};
}

std::string get_printable_status(const GameStateMemory& state);
std::string get_printable_status_lands(const GameStateMemory& state);
std::string get_printable_status_seas(const GameStateMemory& state);
