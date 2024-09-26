#pragma once
#include "game_state_memory.hpp"
#include "canal.hpp"
#include "land.hpp"
#include "map_cache.hpp"
#include "sea.hpp"
#include "units/fighter.hpp"
#include <string>
#include <sys/types.h>

using Actions = std::array<uint, ACTION_COUNT>;
using PtrSeaunittypes = std::array<uint*, SEA_UNIT_TYPES_COUNT>;
using PtrAirunittypes = std::array<uint*, AIR_UNIT_TYPES_COUNT>;
using PtrLandunittypes = std::array<uint*, LAND_UNIT_TYPES_COUNT>;
using Seaunittypes = std::array<uint, SEA_UNIT_TYPES_COUNT>;
using Playersbuf = std::array<uint, PLAYERS_COUNT_P1>;
using PlayersbufLandArray = std::array<LandArray, PLAYERS_COUNT_P1>;
using PlayersbufSeaArray = std::array<SeaArray, PLAYERS_COUNT_P1>;
using PtrLandUTArray = std::array<PtrLandunittypes, LANDS_COUNT>;
using PtrSeaUTArray = std::array<PtrSeaunittypes, SEAS_COUNT>;
using PtrAirUTArray = std::array<PtrAirunittypes, AIRS_COUNT>;
using PlayersbufLandUTArray = std::array<LandUTArray, PLAYERS_COUNT_P1>;
using PlayersbufSeaUTArray = std::array<SeaUTArray, PLAYERS_COUNT_P1>;
using BoolPlayersbuf = std::array<bool, PLAYERS_COUNT_P1>;

void initialize_random_numbers();
void initialize_land_pointers();
void initialize_sea_pointers();

void apply_skip(uint src_air, uint dst_air);

void load_game_data(const std::string& filename);

void play_full_turn();


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

GameStateMemory* clone_state(GameStateMemory* game_state);

void get_possible_actions(GameStateMemory* game_state, uint* num_actions, Actions actions);
void apply_action(GameStateMemory* game_state, uint action);
bool is_terminal_state(GameStateMemory* game_state);
double evaluate_state(GameStateMemory* game_state);
double random_play_until_terminal(GameStateMemory* game_state);
GameStateMemory* get_game_state_copy();

void load_single_game();
bool end_turn();

uint get_attacker_hits(uint attacker_damage);
uint get_defender_hits(uint defender_damage);
