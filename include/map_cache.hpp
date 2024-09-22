#pragma once

#include "game_state_json.hpp"
#include "canal.hpp"
#include <array>
#include <limits>
#include <sys/types.h>

using LandAirArray = std::array<AirArray, LANDS_COUNT>;
using AirA2AArray = std::array<A2AConn, AIRS_COUNT>;
using AirAirArray = std::array<AirArray, AIRS_COUNT>;
using LandLandArray = std::array<LandArray, LANDS_COUNT>;
using LandSeaArray = std::array<SeaArray, LANDS_COUNT>;
using CanalSeaArray = std::array<SeaArray, CANAL_STATES>;
using CanalSeaSeaArray = std::array<SeaSeaArray, CANAL_STATES>;

LandArray LAND_VALUE = {0};
LandAirArray LAND_DIST = {{{0}}};
AirArray AIR_CONN_COUNT = {0};
AirA2AArray AIR_CONNECTIONS = {{{0}}};
AirAirArray AIR_DIST = {{{0}}};
LandAirArray LAND_PATH = {{{MAX_INT}}};
LandAirArray LAND_PATH_ALT = {{{MAX_INT}}};
LandLandArray LANDS_WITHIN_2_MOVES = {{{0}}};
LandArray LANDS_WITHIN_2_MOVES_COUNT = {0};
LandSeaArray LOAD_WITHIN_2_MOVES = {{{0}}};
LandArray LOAD_WITHIN_2_MOVES_COUNT = {0};
CanalSeaSeaArray SEAS_WITHIN_1_MOVE = {{{{{0}}}}};
CanalSeaArray SEAS_WITHIN_1_MOVE_COUNT = {{{0}}};
CanalSeaSeaArray SEAS_WITHIN_2_MOVES = {{{{{0}}}}};
CanalSeaArray SEAS_WITHIN_2_MOVES_COUNT = {{{0}}};
AirhopAirAirArray AIR_WITHIN_X_MOVES = {{{{{0}}}}};
AirhopAirArray AIR_WITHIN_X_MOVES_COUNT = {{{0}}};
AirhopAirLandArray AIR_TO_LAND_WITHIN_X_MOVES = {{{{0}}}};
AirhopLandArray AIR_TO_LAND_WITHIN_X_MOVES_COUNT = {{{0}}};
CanalSeaSeaArray SEA_DIST = {{{{{0}}}}};
CanalSeaSeaArray SEA_PATH = {{{{{MAX_INT}}}}};
CanalSeaSeaArray SEA_PATH_ALT = {{{{{MAX_INT}}}}};
LandArray LAND_TO_LAND_COUNT = {0};
LandL2LArray LAND_TO_LAND_CONN = {{{0}}};
LandArray LAND_TO_SEA_COUNT = {0};
LandL2SArray LAND_TO_SEA_CONN = {{{0}}};
SeaArray SEA_TO_SEA_COUNT = {0};
SeaS2SArray SEA_TO_SEA_CONN = {{{0}}};
SeaArray SEA_TO_LAND_COUNT = {0};
SeaS2LArray SEA_TO_LAND_CONN = {{{0}}};
