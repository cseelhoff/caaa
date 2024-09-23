#pragma once
#include "array_functions.hpp"
#include "land.hpp"
#include "player.hpp"
#include "sea.hpp"
#include "units/units.hpp"
#include <array>
#include <cmath>

constexpr uint AIRS_COUNT = LANDS_COUNT + SEAS_COUNT;
constexpr uint MAX_AIR_TO_AIR_CONNECTIONS = 7;

using AirArray = std::array<uint, AIRS_COUNT>;
using PlayerArray = std::array<uint, PLAYERS_COUNT>;
using A2AConn = std::array<uint, MAX_AIR_TO_AIR_CONNECTIONS>;
using BoolAirArray = std::array<bool, AIRS_COUNT>;
using ActiveLandFighters = Uint2DArray<LANDS_COUNT, FIGHTER_STATES>;
using ActiveLandBombers = Uint2DArray<LANDS_COUNT, BOMBER_LAND_STATES>;
using ActiveLandInfantry = Uint2DArray<LANDS_COUNT, INFANTRY_STATES>;
using ActiveLandArtillery = Uint2DArray<LANDS_COUNT, ARTILLERY_STATES>;
using ActiveLandTanks = Uint2DArray<LANDS_COUNT, TANK_STATES>;
using ActiveLandAAGuns = Uint2DArray<LANDS_COUNT, AA_GUN_STATES>;
using ActiveSeaFighters = Uint2DArray<SEAS_COUNT, FIGHTER_STATES>;
using ActiveSeaTransEmpty = Uint2DArray<SEAS_COUNT, TRANS_EMPTY_STATES>;
using ActiveSeaTrans1I = Uint2DArray<SEAS_COUNT, TRANS_1I_STATES>;
using ActiveSeaTrans1A = Uint2DArray<SEAS_COUNT, TRANS_1A_STATES>;
using ActiveSeaTrans1T = Uint2DArray<SEAS_COUNT, TRANS_1T_STATES>;
using ActiveSeaTrans2I = Uint2DArray<SEAS_COUNT, TRANS_2I_STATES>;
using ActiveSeaTrans1I_1A = Uint2DArray<SEAS_COUNT, TRANS_1I_1A_STATES>;
using ActiveSeaTrans1I_1T = Uint2DArray<SEAS_COUNT, TRANS_1I_1T_STATES>;
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
  ActiveLandAAGuns active_land_aa_guns{};
  ActiveSeaFighters active_sea_fighters{};
  ActiveSeaTransEmpty active_sea_trans_empty{};
  ActiveSeaTrans1I active_sea_trans_1i{};
  ActiveSeaTrans1A active_sea_trans_1a{};
  ActiveSeaTrans1T active_sea_trans_1t{};
  ActiveSeaTrans2I active_sea_trans_2i{};
  ActiveSeaTrans1I_1A active_sea_trans_1i_1a{};
  ActiveSeaTrans1I_1T active_sea_trans_1i_1t{};
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
  PlayerLandArray idle_land_aa_guns{};
  PlayerSeaArray idle_sea_fighters{};
  PlayerSeaArray idle_sea_trans_empty{};
  PlayerSeaArray idle_sea_trans_1i{};
  PlayerSeaArray idle_sea_trans_1a{};
  PlayerSeaArray idle_sea_trans_1t{};
  PlayerSeaArray idle_sea_trans_2i{};
  PlayerSeaArray idle_sea_trans_1i_1a{};
  PlayerSeaArray idle_sea_trans_1i_1t{};
  PlayerSeaArray idle_sea_submarines{};
  PlayerSeaArray idle_sea_destroyers{};
  PlayerSeaArray idle_sea_carriers{};
  PlayerSeaArray idle_sea_cruisers{};
  PlayerSeaArray idle_sea_battleships{};
  PlayerSeaArray idle_sea_bs_damaged{};
} __attribute__((aligned(ALIGNMENT_128)));
