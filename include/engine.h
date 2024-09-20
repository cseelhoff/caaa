#ifndef ENGINE_H
#define ENGINE_H
#include "game_state.hpp"
#include "canal.h"
#include "land.h"
#include "sea.h"
#include <sys/types.h>

#define STRING_BUFFER_SIZE 64
#define MIN_AIR_HOPS 2
#define MAX_AIR_HOPS 6
#define MIN_SEA_HOPS 1
#define MAX_SEA_HOPS 2
#define MIN_LAND_HOPS 1
#define MAX_LAND_HOPS 2
#define AIR_MOVE_SIZE 1 + MAX_AIR_HOPS - MIN_AIR_HOPS
#define SEA_MOVE_SIZE 1 + MAX_SEA_HOPS - MIN_SEA_HOPS
#define LAND_MOVE_SIZE 1 + MAX_LAND_HOPS - MIN_LAND_HOPS

#define PRINTABLE_GAME_STATUS_SIZE 4096
#define RANDOM_NUMBERS_SIZE 65536
#define PLAYERS_COUNT_P1 PLAYERS_COUNT + 1

using PtrSeaunittypes = std::array<uint*, SEA_UNIT_TYPES_COUNT>;
using PtrAirunittypes = std::array<uint*, AIR_UNIT_TYPES_COUNT>;
using PtrLandunittypes = std::array<uint*, LAND_UNIT_TYPES_COUNT>;
using Seaunittypes = std::array<uint, SEA_UNIT_TYPES_COUNT>;
using RandomNumberArray = std::array<uint, RANDOM_NUMBERS_SIZE>;
using Playersbuf = std::array<uint, PLAYERS_COUNT_P1>;
using SeaS2SArray = std::array<S2SConn, SEAS_COUNT>;
using LandAirArray = std::array<AirArray, LANDS_COUNT>;
using AirA2AArray = std::array<A2AConn, AIRS_COUNT>;
using AirAirArray = std::array<AirArray, AIRS_COUNT>;
using AirLandArray = std::array<LandArray, AIRS_COUNT>;
using LandLandArray = std::array<LandArray, LANDS_COUNT>;
using LandSeaArray = std::array<SeaArray, LANDS_COUNT>;
using CanalSeaArray = std::array<SeaArray, CANAL_STATES>;
using AirhopAirAirArray = std::array<AirAirArray, MAX_AIR_HOPS>;
using AirhopAirArray = std::array<AirArray, MAX_AIR_HOPS>;
using AirhopAirLandArray = std::array<AirLandArray, MAX_AIR_HOPS>;
using AirhopLandArray = std::array<LandArray, MAX_AIR_HOPS>;
using LandL2LArray = std::array<L2LConn, LANDS_COUNT>;
using LandL2SArray = std::array<L2SConn, LANDS_COUNT>;
using SeaS2LArray = std::array<S2LConn, SEAS_COUNT>;
using PlayersbufLandArray = std::array<LandArray, PLAYERS_COUNT_P1>;
using PlayersbufSeaArray = std::array<SeaArray, PLAYERS_COUNT_P1>;
using LandUTArray = std::array<Landunittypes, LANDS_COUNT>;
using SeaUTArray = std::array<Seaunittypes, SEAS_COUNT>;
using PtrLandUTArray = std::array<PtrLandunittypes, LANDS_COUNT>;
using PtrSeaUTArray = std::array<PtrSeaunittypes, SEAS_COUNT>;
using PtrAirUTArray = std::array<PtrAirunittypes, AIRS_COUNT>;
using CanalSeaSeaArray = std::array<SeaSeaArray, CANAL_STATES>;
using PlayersbufLandUTArray = std::array<LandUTArray, PLAYERS_COUNT_P1>;
using PlayersbufSeaUTArray = std::array<SeaUTArray, PLAYERS_COUNT_P1>;


void initialize_constants();

void initialize_land_dist();
void initialize_l2l_connections(uint src_land);
void initialize_l2s_connections(uint src_land);
void initialize_land_dist_zero(uint src_land);
void set_l2l_land_dist_to_one(uint src_land);
void set_l2s_land_dist_to_one(uint src_land);
void land_dist_floyd_warshall();
//void floyd_warshall(int* dist, int terr_count, int dist_count);

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
void initialize_sea_path_2(uint mid_sea, uint src_sea, uint canal_state_idx, CanalSeaSeaArray* sea_path_ptr);

void initialize_within_x_moves();
void initialize_land_within_2_moves(uint src_land);
void initialize_load_within_2_moves(uint src_land);
void initialize_sea_within_x_moves();
void initialize_air_within_x_moves();
void initialize_air_to_land_within_x_moves();

void initialize_random_numbers();
void initialize_land_pointers();
void initialize_sea_pointers();

void intialize_airs_x_to_4_moves_away();
void initialize_skip_4air_precals();
void apply_skip(uint src_air, uint dst_air);

void load_game_data(char const* filename);

void play_full_turn();
void refresh_full_cache();
void refresh_eot_cache();
void refresh_economy();
void refresh_land_armies();
void refresh_sea_navies();
void refresh_allies();
void refresh_canals();
void refresh_enemy_armies();
void refresh_fleets();
void refresh_land_path_blocked();
void refresh_sea_path_blocked();

void setPrintableStatus();
void setPrintableStatusLands();
void setPrintableStatusSeas();

uint getUserInput();
uint getAIInput();

void build_landMove2Destination();
void build_landMove1Destination();
void build_landMove1DestinationAlt();
void build_airMove2Destination();
void build_airMove3Destination();
void build_airMove4Destination();
void build_airMove5Destination();
void build_airMove6Destination();

uint get_user_purchase_input(uint src_air);
uint get_user_move_input(uint unit_type, uint src_air);
void update_move_history(uint user_input, uint src_air);
bool load_transport(uint unit_type, uint src_land, uint dst_sea, uint land_unit_state);
void add_valid_land_moves(uint src_land, uint moves_remaining, uint unit_type);
void add_valid_sea_moves(uint src_sea, uint moves_remaining);
void add_valid_sub_moves(uint src_sea, uint moves_remaining);
bool stage_transport_units();
void pre_move_fighter_units();
bool move_fighter_units();
bool move_bomber_units();
void conquer_land(uint dst_land);
bool move_land_unit_type(uint unit_type);
bool move_transport_units();
void skip_empty_transports();
bool move_subs();
bool move_destroyers_battleships();
void carry_allied_fighters(uint src_sea, uint dst_sea);
bool resolve_sea_battles();
uint ask_to_retreat();
void remove_land_defenders(uint src_land, uint hits);
void remove_land_attackers(uint src_land, uint hits);
void remove_sea_defenders(uint src_sea, uint hits, bool defender_submerged);
void remove_sea_attackers(uint src_sea, uint hits);
bool unload_transports();
bool resolve_land_battles();
void add_valid_unload_moves(uint src_sea);
void add_valid_fighter_moves(uint src_air, uint remaining_moves);
void add_valid_fighter_landing(uint src_air, uint remaining_moves);
void add_valid_bomber_moves(uint src_air, uint remaining_moves);
bool land_fighter_units();
void add_valid_bomber_landing(uint src_air, uint movement_remaining);
bool land_bomber_units();
bool buy_units();
void crash_air_units();
void reset_units_fully();
void buy_factory();
void collect_money();
void rotate_turns();
double get_score();
void debug_checks();
void sea_retreat(uint src_sea, uint retreat);
void set_seed(int16_t seed);

GameState* clone_state(GameState* game_state);

void get_possible_actions(GameState* game_state, int* num_actions, ActionsPtr actions);
void apply_action(GameState* game_state, uint action);
bool is_terminal_state(GameState* game_state);
double evaluate_state(GameState* game_state);
double random_play_until_terminal(GameState* game_state);
GameState* get_game_state_copy();

void load_single_game();
bool end_turn();

uint get_attacker_hits(uint attacker_damage);
uint get_defender_hits(uint defender_damage);
#endif
