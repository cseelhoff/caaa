#pragma once
#include "game_state.h"
#include "typedefs.h"
#include <stdbool.h>
#include <stdint.h>

typedef uint8_t PlayerIndexCount;
typedef uint8_t AirMilitaryCount;
typedef LandUnitStateSum* LandUnitStateSums[MAX_LAND_UNIT_STATES];
typedef LandUnitStateSums LandUnitStates[LAND_UNIT_TYPES_COUNT];
typedef SeaUnitStateSum* SeaUnitStateSums[MAX_SEA_UNIT_STATES];
typedef SeaUnitStateSums SeaUnitStates[SEA_UNIT_TYPES_COUNT];
typedef AirUnitStateSum* AirUnitStateSums[MAX_AIR_UNIT_STATES];
typedef AirUnitStateSums AirUnitStates[AIR_UNIT_TYPES_COUNT];

void initializeGameData();
void load_game_data();
void play_full_turn();
double get_score();
void set_seed(uint16_t seed);
GameState* clone_state(GameState* game_state);
void free_state(GameState* game_state);
ActionArray* get_possible_actions(GameState* game_state, int* num_actions);
void apply_action(GameState* game_state, Action action);
bool is_terminal_state(GameState* game_state);
double evaluate_state(GameState* game_state);
double random_play_until_terminal(GameState* game_state);
GameState* get_game_state_copy();
void set_ally(PlayerIndex ally_idx);
bool is_allied(PlayerIndex player_idx);
void add_enemy(PlayerIndex player_idx);
PlayerIndex get_land_owner(LandIndex land_idx);
bool is_canal_controlled(CanalState canal_idx);
PlayerIndex get_enemy_player(EnemyIndex enemy_idx);
ArmySum get_player_armies(PlayerIndex player_idx, LandIndex land_idx);
void acc_enemy_units_count(AirIndex air_idx, AirMilitaryCount sum);
void set_is_land_path_blocked(LandIndex src_land, LandIndex dst_land);
SeaUnitSumArray* get_my_sea_unit_types(SeaIndex sea_idx);
void set_allied_carriers(SeaIndex sea_idx, NavySum carriers);
SeaUnitSumArray* get_player_sea_unit_types_ref(PlayerIndex player_idx, SeaIndex sea_idx);
NavySum get_player_navies(PlayerIndex player_idx, SeaIndex sea_idx);
void acc_enemy_destroyers(SeaIndex sea_idx, SeaUnitSumArray* sea_units);
void acc_enemy_blockade(SeaIndex sea_idx, SeaUnitSumArray* sea_units);
void acc_allied_carriers(SeaIndex sea_idx, SeaUnitSumArray* sea_units);
void recalc_transports_cargo_space(SeaIndex sea_idx, SeaUnitSumArray* sea_units);
NavySum get_enemy_blockade(SeaIndex sea_idx);
void set_is_sea_path_blocked(SeaIndex src_sea, SeaIndex dst_sea, SeaIndex nextSeaMovement,
                             SeaIndex nextSeaMovementAlt);
AirMilitaryCount get_enemy_units_count(AirIndex air_idx);
HitPoints get_factory_max(LandIndex land_idx);
void set_is_sub_path_blocked(SeaIndex src_sea, SeaIndex dst_sea, SeaIndex nextSeaMovement,
                             SeaIndex nextSeaMovementAlt);
NavySum get_enemy_destroyers(SeaIndex sea_idx);
LandTerr* get_land_terr(LandIndex land_idx);
LandUnitSumArray* get_my_land_unit_types(LandIndex land_idx);
LandUnitStates* get_my_land_unit_states(LandIndex land_idx);
AirUnitStates* get_my_air_unit_states(AirIndex air_idx);
void add_factory_location(PlayerIndex player_idx, LandIndex land_idx);
void acc_income_from_land(PlayerIndex player_idx, LandIndex land_idx);
ArmySum* get_player_armies_ref(PlayerIndex player_idx, LandIndex land_idx);
void acc_LandUnitSumArray(LandUnitSumArray* land_units, LandUnitType unit_type,
                          LandUnitStates* land_unit_states, GenericLandUnitState land_unit_state);
void acc_ArmySumArray(ArmySum* army_sum, LandUnitSumArray* land_units, LandUnitType unit_type);
LandUnitSumArray* get_player_land_unit_types_ref(PlayerIndex player_idx, LandIndex land_idx);
SeaTerr* get_sea_terr(SeaIndex sea_idx);
SeaUnitStates* get_my_sea_unit_states(SeaIndex sea_idx);
NavySum* get_player_navies_ref(PlayerIndex player_idx, SeaIndex sea_idx);
LandUnitSum get_land_unit_sum(LandUnitSumArray* land_units, LandUnitType unit_type);
void acc_SeaUnitSumArray(SeaUnitSumArray* sea_units, SeaUnitType unit_type,
                         SeaUnitStates* sea_unit_states, GenericSeaUnitState sea_unit_state);
void acc_NavySumArray(NavySum* navy_sum, SeaUnitSumArray* sea_units, SeaUnitType unit_type);
SeaUnitSum get_sea_unit_sum(SeaUnitSumArray* sea_units, SeaUnitType unit_type);
void set_income_per_turn(PlayerIndex player_idx, Dollars income);
void set_factory_count(PlayerIndex player_idx, LandIndex count);
Player get_player_state_offset(PlayerIndex player_idx);
HitPoints get_factory_hp(LandIndex land_idx);
bool is_flagged_for_combat(AirIndex air_idx);
LandUnitStateSums* get_land_unit_state_sums(LandUnitStates* land_unit_states,
                                            LandUnitType unit_type);
LandUnitStateSum get_land_unit_state_sum_at(LandUnitStateSums* landUnitStateSums,
                                            GenericLandUnitState unit_state);
SeaUnitStateSums* get_sea_unit_state_sums(SeaUnitStates* sea_unit_states, SeaUnitType unit_type);
SeaUnitStateSum get_sea_unit_state_sum_at(SeaUnitStateSums* seaUnitStateSums,
                                          GenericSeaUnitState unit_state);
Dollars get_money(PlayerIndex player_idx);
AirIndexArray* get_source_territories(LandIndex land_idx);
AirIndex get_source_territory(AirIndexArray* source_territories,
                              SourceTerritoryIndex source_terr_idx);
void check_territory(bool* checked_territories, AirIndex src_air);
bool was_terr_skipped(AirIndex src_air, AirIndex dst_air);
bool has_checked_territory(bool* checked_territories, AirIndex src_air);
AirIndex get_source_terr_count(LandIndex land_idx);
SeaIndex get_sea_from_l2s_conn(SeaConnections* land_to_sea_conn, SeaConnIndex conn_idx);
LandIndex get_land_from_array(LandArray* land_array, LandConnIndex land_conn_idx);
char* get_air_name(AirIndex air_idx);
void replace_transport(SeaUnitStates* sea_unit_states, SeaUnitSumArray* sea_units,
                       SeaUnitType new_trans_type, SeaUnitType old_trans_type,
                       GenericSeaUnitState trans_state);
void remove_my_land_unit_state(LandIndex land_idx, LandUnitType unit_type,
                               GenericLandUnitState land_unit_state);
bool is_non_combat_unit(LandUnitType unit_type);
bool is_non_loadable_unit(LandUnitType unit_type);
bool is_heavy_unit(LandUnitType unit_type);
void acc_sea_state_sums(SeaUnitStateSums* sea_unit_states, GenericSeaUnitState sea_unit_state,
                        SeaUnitStateSum* sum);
SeaIndex convert_air_to_sea(AirIndex air_idx);
void flag_for_combat(AirIndex air_idx);
void unflag_for_combat(AirIndex air_idx);
LandUnitType get_order_of_land_defenders(uint8_t idx);
LandUnitType get_order_of_land_attackers_1(uint8_t idx);
LandUnitType get_order_of_land_attackers_2(uint8_t idx);
SeaUnitType get_order_of_sea_defenders(uint8_t idx);
SeaUnitType get_order_of_sea_attackers_1(uint8_t idx);
SeaUnitType get_order_of_sea_attackers_2(uint8_t idx);
SeaUnitType get_order_of_sea_attackers_3(uint8_t idx);
LandIndex convert_air_to_land(AirIndex air_idx);
AirUnitStateSums* get_air_unit_state_sums(AirIndex air_idx, AirUnitType unit_type);
AirUnitStateSum* get_air_unit_state_sum_at(AirUnitStateSums* air_unit_state_sums,
                                           FighterState cur_state);
AirUnitStateSum* get_bomber_state_sum_at(AirUnitStateSums* air_unit_state_sums,
                                         BomberLandState cur_state);
void set_builds_left(LandIndex land_idx, HitPoints value);
PlayerIndex convert_state_player_offset(PlayerIndex player_idx);
