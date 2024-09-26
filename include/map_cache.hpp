#pragma once

//#include "game_state.hpp"
#include "canal.hpp"
#include "land.hpp"
#include "player.hpp"
#include "sea.hpp"
#include "units/units.hpp"
#include <array>
#include <limits>
#include <sys/types.h>
#include <vector>

constexpr uint MIN_AIR_HOPS =2;
constexpr uint MAX_AIR_HOPS= 6;
constexpr uint MIN_SEA_HOPS= 1;
constexpr uint MAX_SEA_HOPS =2;
constexpr uint MIN_LAND_HOPS =1;
constexpr uint MAX_LAND_HOPS= 2;
constexpr uint AIR_MOVE_SIZE =1 + MAX_AIR_HOPS - MIN_AIR_HOPS;
constexpr uint SEA_MOVE_SIZE =1 + MAX_SEA_HOPS - MIN_SEA_HOPS;
constexpr uint LAND_MOVE_SIZE = 1 + MAX_LAND_HOPS - MIN_LAND_HOPS;
constexpr uint PLAYERS_COUNT_P1 =PLAYERS_COUNT + 1;
constexpr uint AIRS_COUNT = LANDS_COUNT + SEAS_COUNT;
constexpr uint MAX_AIR_TO_AIR_CONNECTIONS = 7;
constexpr uint ACTION_COUNT = std::max<uint>(AIRS_COUNT, SEA_UNIT_TYPES_COUNT + 1);

using A2AConn = std::array<uint, MAX_AIR_TO_AIR_CONNECTIONS>;
using AirArray = std::array<uint, AIRS_COUNT>;
using LandAirArray = std::array<AirArray, LANDS_COUNT>;
using AirA2AArray = std::array<A2AConn, AIRS_COUNT>;
using AirAirArray = std::array<AirArray, AIRS_COUNT>;
using LandLandArray = std::array<LandArray, LANDS_COUNT>;
using LandSeaArray = std::array<SeaArray, LANDS_COUNT>;
using CanalSeaArray = std::array<SeaArray, CANAL_STATES>;
using CanalSeaSeaArray = std::array<SeaSeaArray, CANAL_STATES>;
using AirhopAirAirArray = std::array<AirAirArray, MAX_AIR_HOPS>;
using AirhopAirArray = std::array<AirArray, MAX_AIR_HOPS>;
using AirLandArray = std::array<LandArray, AIRS_COUNT>;
using AirhopAirLandArray = std::array<AirLandArray, MAX_AIR_HOPS>;
using AirhopLandArray = std::array<LandArray, MAX_AIR_HOPS>;
using LandL2LArray = std::array<L2LConn, LANDS_COUNT>;
using LandL2SArray = std::array<L2SConn, LANDS_COUNT>;
using SeaS2SArray = std::array<S2SConn, SEAS_COUNT>;
using SeaS2LArray = std::array<S2LConn, SEAS_COUNT>;
using FightermovesAirArray = std::array<AirArray, FIGHTER_MOVES_MAX>;
using FightermovesAirAirArray = std::array<AirAirArray, FIGHTER_MOVES_MAX>;
using PlayersPlayersVector = std::array<std::vector<uint>, PLAYERS_COUNT>;
using AirAirAirArray = std::array<AirAirArray, AIRS_COUNT>;
using PlayerArray = std::array<uint, PLAYERS_COUNT>;

extern LandArray LAND_VALUE;
extern LandAirArray LAND_DIST;
extern AirArray AIR_CONN_COUNT;
extern AirA2AArray AIR_CONNECTIONS;
extern AirAirArray AIR_DIST;
extern LandAirArray LAND_PATH;
extern LandAirArray LAND_PATH_ALT;
extern LandLandArray LANDS_WITHIN_2_MOVES;
extern LandArray LANDS_WITHIN_2_MOVES_COUNT;
extern LandSeaArray LOAD_WITHIN_2_MOVES;
extern LandArray LOAD_WITHIN_2_MOVES_COUNT;
extern CanalSeaSeaArray SEAS_WITHIN_1_MOVE;
extern CanalSeaArray SEAS_WITHIN_1_MOVE_COUNT;
extern CanalSeaSeaArray SEAS_WITHIN_2_MOVES;
extern CanalSeaArray SEAS_WITHIN_2_MOVES_COUNT;
extern AirhopAirAirArray AIR_WITHIN_X_MOVES;
extern AirhopAirArray AIR_WITHIN_X_MOVES_COUNT;
extern AirhopAirLandArray AIR_TO_LAND_WITHIN_X_MOVES;
extern AirhopLandArray AIR_TO_LAND_WITHIN_X_MOVES_COUNT;
extern CanalSeaSeaArray SEA_DIST;
extern CanalSeaSeaArray SEA_PATH;
extern CanalSeaSeaArray SEA_PATH_ALT;
extern LandArray LAND_TO_LAND_COUNT;
extern LandL2LArray LAND_TO_LAND_CONN;
extern LandArray LAND_TO_SEA_COUNT;
extern LandL2SArray LAND_TO_SEA_CONN;
extern SeaArray SEA_TO_SEA_COUNT;
extern SeaS2SArray SEA_TO_SEA_CONN;
extern SeaArray SEA_TO_LAND_COUNT;
extern SeaS2LArray SEA_TO_LAND_CONN;
extern FightermovesAirAirArray AIRS_X_TO_4_MOVES_AWAY;
extern FightermovesAirArray AIRS_X_TO_4_MOVES_AWAY_COUNT;
extern PlayersPlayersVector ENEMIES;
extern PlayerArray PLAYER_TEAM;

void initialize_map_constants();

void initialize_land_dist();
void initialize_l2l_connections(uint src_land);
void initialize_l2s_connections(uint src_land);
void initialize_land_dist_zero(uint src_land);
void set_l2l_land_dist_to_one(uint src_land);
void set_l2s_land_dist_to_one(uint src_land);
void land_dist_floyd_warshall();

void initialize_sea_dist();
void initialize_s2s_connections(uint src_sea);
void initialize_s2l_connections(uint src_sea);
void initialize_sea_dist_zero(uint canal_idx);
void set_s2s_sea_dist_to_one(uint canal_idx);
void initialize_canals(uint canal_idx);
void sea_dist_floyd_warshall(uint canal_idx);

void initialize_air_dist();
void initialize_air_dist_zero();
void set_l2l_air_dist_to_one(uint src_land);
void set_l2s_air_dist_to_one(uint src_land);
void set_s2l_air_dist_to_one(uint src_sea);
void set_s2s_air_dist_to_one(uint src_sea);
void air_dist_floyd_warshall();

void initialize_land_path();
void set_land_path_for_l2l(uint src_land, uint intermediate_land, LandAirArray land_path);
void set_land_path_for_l2s(uint src_land, uint intermediate_land, LandAirArray land_path);

void initialize_sea_path();
void initialize_sea_path_2(uint mid_sea_idx, uint src_sea, uint canal_state_idx, CanalSeaSeaArray* sea_path_ptr);

void initialize_within_x_moves();
void initialize_land_within_2_moves(uint src_land);
void initialize_load_within_2_moves(uint src_land);
void initialize_sea_within_x_moves();
void initialize_air_within_x_moves();
void initialize_air_to_land_within_x_moves();

void intialize_airs_x_to_4_moves_away();
void initialize_skip_4air_precals();
void initialize_enemies();
