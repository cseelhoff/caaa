#pragma once
#include "canal.hpp"
#include "game_state.hpp"
#include "land.hpp"
#include "map_cache.hpp"
#include "sea.hpp"
#include "units/fighter.hpp"
#include "units/units.hpp"
#include <string>
#include <sys/types.h>
#include <vector>

void cause_breakpoint();
void load_game_data(GameState& state, const std::string& filename);
void play_full_turn(GameState& state);
bool move_air_units(GameState& state, AirUnitTypeEnum unit_type);
void refresh_can_planes_land_here(GameState& state, AirUnitTypeEnum unit_type);
void refresh_can_fighters_land_here(GameState& state);
void add_valid_air_moves(GameState& state, uint src_air, AirUnitTypeEnum unit_type);
void add_valid_fighter_moves(GameState& state, uint src_air);
void update_move_history_4air(GameState& state, uint src_air, uint dst_air);
void clear_move_history(GameState& state);
void apply_skip(GameState& state, uint src_air, uint dst_air);
bool move_bomber_units(GameState& state);
void refresh_can_bombers_land_here(GameState& state);
void add_valid_bomber_moves(GameState& state, uint src_air);
bool stage_transport_units(GameState& state, SeaUnitTypesEnum unit_type);
bool move_land_unit_type(GameState& state, uint unit_type);
bool move_transport_units(GameState& state);
bool move_subs(GameState& state);
bool move_destroyers_battleships(GameState& state);
bool resolve_sea_battles(GameState& state);
bool unload_transports(GameState& state);
bool resolve_land_battles(GameState& state);
bool land_fighter_units(GameState& state);
bool land_bomber_units(GameState& state);
bool buy_units(GameState& state);
void crash_air_units(GameState& state);
void reset_units_fully(GameState& state);
void buy_factory(GameState& state);
void collect_money(GameState& state);
void rotate_turns(GameState& state);

uint getUserInput(GameState& state, std::vector<uint>& valid_moves);
uint getAIInput(GameState& state, std::vector<uint>& valid_moves);
uint get_user_purchase_input(GameState& state, uint src_air);
uint get_user_move_input(GameState& state, uint unit_type, uint src_air);
void update_move_history(GameState& state, uint user_input, uint src_air);
bool load_transport(GameState& state, uint unit_type, uint src_land, uint dst_sea,
                    uint land_unit_state);
void add_valid_land_moves(GameState& state, uint src_land, uint moves_remaining, uint unit_type);
void add_valid_sea_moves(GameState& state, uint src_sea, uint moves_remaining);
void add_valid_sub_moves(GameState& state, uint src_sea, uint moves_remaining);
void conquer_land(GameState& state, uint dst_land);
void skip_empty_transports(GameState& state);
void carry_allied_fighters(GameState& state, uint src_sea, uint dst_sea);
uint ask_to_retreat(GameState& state);
void remove_land_defenders(GameState& state, uint src_land, uint hits);
void remove_land_attackers(GameState& state, uint src_land, uint hits);
void remove_sea_defenders(GameState& state, uint src_sea, uint hits, bool defender_submerged);
void remove_sea_attackers(GameState& state, uint src_sea, uint hits);
void add_valid_unload_moves(GameState& state, uint src_sea);
void add_valid_fighter_landing(GameState& state, uint src_air, uint remaining_moves);
void add_valid_bomber_moves(GameState& state, uint src_air, uint remaining_moves);
void add_valid_bomber_landing(GameState& state, uint src_air, uint movement_remaining);
double get_score(GameState& state);
void debug_checks(GameState& state);
void sea_retreat(GameState& state, uint src_sea, uint retreat);
void set_seed(GameState& state, uint seed);
std::vector<uint> get_possible_actions(GameState* state);
void apply_action(GameState* state, uint action);
bool is_terminal_state(GameState* state);
double evaluate_state(GameState* state);
double random_play_until_terminal(GameState* state);
GameState* clone_state(GameState* state);
GameState* get_game_state_copy(GameState& state);
void load_single_game(GameState& state, const std::string& filename);
bool end_turn(GameState& state);
uint get_attacker_hits(GameState& state, uint attacker_damage);
uint get_defender_hits(GameState& state, uint defender_damage);
