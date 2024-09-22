#pragma once
#include "game_state_json.hpp"
#include "units/transport.hpp"
#include <cmath>

using ActiveLandFighters = std::array<uint, static_cast<std::size_t>(LANDS_COUNT* FIGHTER_STATES)>;
using ActiveLandBombers =
    std::array<uint, static_cast<std::size_t>(LANDS_COUNT* BOMBER_LAND_STATES)>;
using ActiveLandInfantry = std::array<uint, static_cast<std::size_t>(LANDS_COUNT* INFANTRY_STATES)>;
using ActiveLandArtillery =
    std::array<uint, static_cast<std::size_t>(LANDS_COUNT* ARTILLERY_STATES)>;
using ActiveLandTanks = std::array<uint, static_cast<std::size_t>(LANDS_COUNT* TANK_STATES)>;
using ActiveLandAAGuns = std::array<uint, static_cast<std::size_t>(LANDS_COUNT* AA_GUN_STATES)>;

using ActiveSeaFighters = std::array<uint, static_cast<std::size_t>(SEAS_COUNT* FIGHTER_STATES)>;
using ActiveSeaTransEmpty =
    std::array<uint, static_cast<std::size_t>(SEAS_COUNT* TRANS_EMPTY_STATES)>;
using ActiveSeaTrans1I = std::array<uint, static_cast<std::size_t>(SEAS_COUNT* TRANS_1I_STATES)>;
using ActiveSeaTrans1A = std::array<uint, static_cast<std::size_t>(SEAS_COUNT* TRANS_1A_STATES)>;
using ActiveSeaTrans1T = std::array<uint, static_cast<std::size_t>(SEAS_COUNT* TRANS_1T_STATES)>;
using ActiveSeaTrans2I = std::array<uint, static_cast<std::size_t>(SEAS_COUNT* TRANS_2I_STATES)>;
using ActiveSeaTrans1I_1A =
    std::array<uint, static_cast<std::size_t>(SEAS_COUNT* TRANS_1I_1A_STATES)>;
using ActiveSeaTrans1I_1T =
    std::array<uint, static_cast<std::size_t>(SEAS_COUNT* TRANS_1I_1T_STATES)>;
using ActiveSeaSubmarines = std::array<uint, static_cast<std::size_t>(SEAS_COUNT* SUB_STATES)>;
using ActiveSeaDestroyers =
    std::array<uint, static_cast<std::size_t>(SEAS_COUNT* DESTROYER_STATES)>;
using ActiveSeaCarriers = std::array<uint, static_cast<std::size_t>(SEAS_COUNT* CARRIER_STATES)>;
using ActiveSeaCruisers = std::array<uint, static_cast<std::size_t>(SEAS_COUNT* CRUISER_STATES)>;
using ActiveSeaBattleships =
    std::array<uint, static_cast<std::size_t>(SEAS_COUNT* BATTLESHIP_STATES)>;
using ActiveSeaBSDamaged =
    std::array<uint, static_cast<std::size_t>(SEAS_COUNT* BATTLESHIP_STATES)>;
using ActiveSeaBombers = std::array<uint, static_cast<std::size_t>(SEAS_COUNT* BOMBER_SEA_STATES)>;

using PlayerLandArray = std::array<uint, static_cast<std::size_t>(PLAYERS_COUNT* LANDS_COUNT)>;
using PlayerSeaArray = std::array<uint, static_cast<std::size_t>(PLAYERS_COUNT* SEAS_COUNT)>;

struct GameStateMemory {
  uint current_turn{};
  uint seed{};
  PlayersArray money{};
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
  PlayerSeaArray idle_trans_empty{};
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
