#include "engine.h"
#include "air.h"
#include "game_state.h"
#include "land.h"
#include "mcts.h"
#include "player.h"
#include "rand.h"
#include "sea.h"
#include "serialize_data.h"
#include "typedefs.h"
#include "units/aa_gun.h"
#include "units/artillery.h"
#include "units/battleship.h"
#include "units/bomber.h"
#include "units/carrier.h"
#include "units/cruiser.h"
#include "units/destroyer.h"
#include "units/fighter.h"
#include "units/infantry.h"
#include "units/sub.h"
#include "units/tank.h"
#include "units/transport.h"
#include "units/units.h"
#include <cjson/cJSON.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/*
typedef struct {
  FighterStateSum* fighters;
  BomberLandStateSum* bombers;
  InfantryStateSum* infantry;
  ArtilleryStateSum* artillery;
  TanksStateSum* tanks;
  AAGunsStateSum* aa_guns;
} LandUnitStateSums;
*/
// typedef LandUnitStateSums AllLandUnitStateSums[LANDS_COUNT];

#define BASE10 10
#define DICE_SIDES 6
#define STRING_BUFFER_SIZE 4096
char buffer[STRING_BUFFER_SIZE];

#define PRINTABLE_GAME_STATUS_SIZE 4096
#define NEAR_UINT8_T_OVERFLOW 240

u_short random_number_index = 0;
u_short seed = 0;
char printableGameStatus[PRINTABLE_GAME_STATUS_SIZE] = "";
GameState sea_state = {0};
cJSON* json;

LandUnitTypesSumArrayLands my_land_unit_types;   // temp
SeaUnitTypesSumArraySeas my_sea_unit_types;      // temp
LandUnitTypesSumArrayLands temp_land_unit_types; // temp
SeaUnitTypesSumArraySeas temp_sea_unit_types;    // temp
LandUnitTypesSumArrayLands* player_land_unit_types[PLAYERS_COUNT] = {
    &my_land_unit_types, &sea_state.other_land_units[0], &sea_state.other_land_units[1],
    &sea_state.other_land_units[2], &sea_state.other_land_units[3]};
SeaUnitTypesSumArraySeas* player_sea_unit_types[PLAYERS_COUNT] = {
    &my_sea_unit_types, &sea_state.other_sea_units[0], &sea_state.other_sea_units[1],
    &sea_state.other_sea_units[2], &sea_state.other_sea_units[3]};
LandUnitStates my_land_unit_states[LANDS_COUNT] = {0};
SeaUnitStates my_sea_unit_states[SEAS_COUNT] = {0};
AirUnitStates my_air_unit_states[AIRS_COUNT] = {0};

PlayerIndex* owner_idx[LANDS_COUNT];
HitPoints* factory_max[LANDS_COUNT];
NavySum* bombard_max[LANDS_COUNT];
HitPoints* factory_hp[LANDS_COUNT]; // allow negative
Dollars income_per_turn[PLAYERS_COUNT + 1];
LandConnIndex factory_count[PLAYERS_COUNT + 1] = {0};

typedef LandIndex FactoryLocations[LANDS_COUNT];
FactoryLocations factory_locations[PLAYERS_COUNT + 1] = {0};

typedef ArmySum LandArmySumArray[LANDS_COUNT];
LandArmySumArray player_armies[PLAYERS_COUNT + 1];
typedef NavySum SeaNavySumArray[SEAS_COUNT];
SeaNavySumArray player_navies[PLAYERS_COUNT + 1];
LandArmySumArray* my_armies = &player_armies[0];
SeaNavySumArray* my_navies = &player_navies[0];

typedef AirIndex AirIndexArray[AIRS_COUNT];
AirIndexArray hist_skipped_airs[AIRS_COUNT] = {0};
AirIndexArray hist_source_territories[AIRS_COUNT] = {0};
AirIndex hist_source_territories_count[AIRS_COUNT] = {0};

// int units_sea_blockade_total[PLAYERS_COUNT][SEAS_COUNT];
NavySum enemy_blockade[SEAS_COUNT] = {0};
NavySum enemy_destroyers[SEAS_COUNT] = {0};

typedef bool BoolLandArray[LANDS_COUNT];
BoolLandArray is_land_path_blocked[LANDS_COUNT] = {0};
typedef bool BoolSeaArray[SEAS_COUNT];
BoolSeaArray is_sea_path_blocked[SEAS_COUNT] = {0};
BoolSeaArray is_sub_path_blocked[SEAS_COUNT] = {0};
NavySum transports_with_large_cargo_space[SEAS_COUNT];
NavySum transports_with_small_cargo_space[SEAS_COUNT];

PlayerIndex enemies_0[PLAYERS_COUNT - 1] = {0};
bool is_allied_0[PLAYERS_COUNT] = {0};
PlayerIndexCount enemies_count_0 = 0;
CanalState canal_state = 0;
NavySum allied_carriers[SEAS_COUNT] = {0};
AirMilitaryCount enemy_units_count[AIRS_COUNT] = {0};
bool canFighterLandHere[AIRS_COUNT] = {0};
bool canFighterLandIn1Move[AIRS_COUNT] = {0};
bool canBomberLandHere[AIRS_COUNT] = {0};
bool canBomberLandIn1Move[AIRS_COUNT] = {0};
bool canBomberLandIn2Moves[AIRS_COUNT] = {0};
int step_id = 0;
ushort answers_remaining = 0;
Action selected_answer = 0;
ActionArray valid_actions = {0};
ActionCount valid_actions_count = 0;

inline void set_income_per_turn(PlayerIndex player_idx, Dollars income) {
  income_per_turn[player_idx] = income;
}

inline void set_factory_count(PlayerIndex player_idx, LandIndex count) {
  factory_count[player_idx] = count;
}

inline void set_ally(PlayerIndex ally_idx) {
  PlayerIndex player_idx = sea_state.player_index;
  is_allied_0[ally_idx] = are_players_allied(player_idx, (player_idx + ally_idx) % PLAYERS_COUNT);
}
inline bool is_allied(PlayerIndex player_idx) { return is_allied_0[player_idx]; }
inline void add_enemy(PlayerIndex player_idx) { enemies_0[enemies_count_0++] = player_idx; }
inline PlayerIndex get_land_owner(LandIndex land_idx) { return *owner_idx[land_idx]; }

inline bool is_canal_controlled(CanalState canal_idx) {
  return is_allied(get_land_owner(get_canal_land(canal_idx, 0))) &&
         is_allied(get_land_owner(get_canal_land(canal_idx, 1)));
}
inline PlayerIndex get_enemy_player(EnemyIndex enemy_idx) { return enemies_0[enemy_idx]; }
inline ArmySum get_player_armies(PlayerIndex player_idx, LandIndex land_idx) {
  return player_armies[player_idx][land_idx];
}
// inline AirIndex convert_land_to_air(LandIndex land_idx) { return (AirIndex)land_idx; }
inline void acc_enemy_units_count(AirIndex air_idx, AirMilitaryCount sum) {
  enemy_units_count[air_idx] += sum;
}
inline AirMilitaryCount get_enemy_units_count(AirIndex air_idx) {
  return enemy_units_count[air_idx];
}

inline HitPoints get_factory_max(LandIndex land_idx) { return (*factory_max)[land_idx]; }

inline void set_is_land_path_blocked(LandIndex src_land, LandIndex dst_land) {
  LandIndex nextLandMovement = get_land_path1(src_land, dst_land);
  LandIndex nextLandMovementAlt = get_land_path_alt(src_land, dst_land);
  is_land_path_blocked[src_land][dst_land] =
      (get_enemy_units_count(nextLandMovement) > 0 || get_factory_max(nextLandMovement) > 0) &&
      (get_enemy_units_count(nextLandMovementAlt) > 0 || get_factory_max(nextLandMovementAlt) > 0);
}

inline SeaUnitSumArray* get_my_sea_unit_types(SeaIndex sea_idx) {
  return &my_sea_unit_types[sea_idx];
}
inline void set_allied_carriers(SeaIndex sea_idx, SeaUnitSum carriers) {
  allied_carriers[sea_idx] = carriers;
}
// inline AirIndex convert_sea_to_air(SeaIndex sea_idx) { return sea_idx + LANDS_COUNT; }
typedef SeaUnitSum SeaUnitSumArray[SEA_UNIT_TYPES_COUNT];
inline SeaUnitSumArrayPtr get_player_sea_unit_types_ref(PlayerIndex player_idx, SeaIndex sea_idx) {
  return player_sea_unit_types[player_idx][sea_idx];
}
inline NavySum get_player_navies(PlayerIndex player_idx, SeaIndex sea_idx) {
  return player_navies[player_idx][sea_idx];
}
inline void acc_enemy_destroyers(SeaIndex sea_idx, SeaUnitSumArray* sea_units) {
  enemy_destroyers[sea_idx] += (*sea_units)[DESTROYERS];
}

inline void acc_enemy_blockade(SeaIndex sea_idx, SeaUnitSumArray* sea_units) {
  enemy_blockade[sea_idx] += (*sea_units)[DESTROYERS] + (*sea_units)[CARRIERS] +
                             (*sea_units)[CRUISERS] + (*sea_units)[BATTLESHIPS] +
                             (*sea_units)[BS_DAMAGED];
}

inline void acc_allied_carriers(SeaIndex sea_idx, SeaUnitSum carriers) {
  allied_carriers[sea_idx] += carriers;
}

inline SeaUnitSum get_allied_carriers(SeaIndex sea_idx) { return allied_carriers[sea_idx]; }

inline void recalc_transports_cargo_space(SeaIndex sea_idx, SeaUnitSumArray* sea_units) {
  transports_with_large_cargo_space[sea_idx] = (*sea_units)[TRANS_EMPTY] + (*sea_units)[TRANS_1I];
  transports_with_small_cargo_space[sea_idx] =
      transports_with_large_cargo_space[sea_idx] + (*sea_units)[TRANS_1A] + (*sea_units)[TRANS_1T];
}

inline NavySum get_enemy_blockade(SeaIndex sea_idx) { return enemy_blockade[sea_idx]; }

inline void set_is_sea_path_blocked(SeaIndex src_sea, SeaIndex dst_sea, SeaIndex nextSeaMovement,
                                    SeaIndex nextSeaMovementAlt) {
  is_sea_path_blocked[src_sea][dst_sea] =
      get_enemy_blockade(nextSeaMovement) > 0 && get_enemy_blockade(nextSeaMovementAlt) > 0;
}

inline NavySum get_enemy_destroyers(SeaIndex sea_idx) { return enemy_destroyers[sea_idx]; }

inline void set_is_sub_path_blocked(SeaIndex src_sea, SeaIndex dst_sea, SeaIndex nextSeaMovement,
                                    SeaIndex nextSeaMovementAlt) {
  is_sub_path_blocked[src_sea][dst_sea] =
      get_enemy_destroyers(nextSeaMovement) > 0 && get_enemy_destroyers(nextSeaMovementAlt) > 0;
}

inline void add_factory_location(PlayerIndex player_idx, LandIndex land_idx) {
  factory_locations[player_idx][factory_count[player_idx]++] = land_idx;
}

inline void acc_income_from_land(PlayerIndex player_idx, LandIndex land_idx) {
  income_per_turn[player_idx] += get_land_value(land_idx);
}

inline LandUnitSumArray* get_my_land_unit_types(LandIndex land_idx) {
  return &my_land_unit_types[land_idx];
}

inline LandTerr* get_land_terr(LandIndex land_idx) { return &sea_state.land_terr[land_idx]; }

inline ArmySum* get_player_armies_ref(PlayerIndex player_idx, LandIndex land_idx) {
  return &player_armies[player_idx][land_idx];
}

inline LandUnitSumArrayPtr get_player_land_unit_types_ref(PlayerIndex player_idx,
                                                          LandIndex land_idx) {
  return player_land_unit_types[player_idx][land_idx];
}

inline void acc_ArmySum(ArmySum* sum, LandUnitSumArray* land_units, LandUnitType unit_type) {
  *sum += (*land_units)[unit_type];
}

inline LandUnitSum get_land_unit_sum(LandUnitSumArray* land_units, LandUnitType unit_type) {
  return (*land_units)[unit_type];
}

inline void acc_ArmySumArray(ArmySum* army_sum, LandUnitSumArray* land_units,
                             LandUnitType unit_type) {
  *army_sum += get_land_unit_sum(land_units, unit_type);
}

inline void acc_LandUnitSumArray(LandUnitSumArray* land_units, LandUnitType unit_type,
                                 LandUnitStates* land_unit_states,
                                 GenericLandUnitState land_unit_state) {
  (*land_units)[unit_type] += (*(*land_unit_states)[unit_type][land_unit_state]);
}
inline LandUnitStates* get_my_land_unit_states(LandIndex land_idx) {
  return &my_land_unit_states[land_idx];
}

inline AirUnitStates* get_my_air_unit_states(AirIndex air_idx) {
  return &my_air_unit_states[air_idx];
}

inline void remove_my_land_unit_state(LandIndex land_idx, LandUnitType unit_type,
                                      GenericLandUnitState land_unit_state) {
  (*(my_land_unit_states[land_idx][unit_type]))[land_unit_state]--;
  my_land_unit_types[land_idx][unit_type]--;
  (*my_armies)[land_idx]--;
}

inline void replace_transport(SeaUnitStates* sea_unit_states, SeaUnitSumArray* sea_units,
                              SeaUnitType new_trans_type, GenericSeaUnitState new_trans_state,
                              SeaUnitType old_trans_type, GenericSeaUnitState trans_state) {
  (*sea_unit_states)[new_trans_type][trans_state]--;
  (*sea_unit_states)[new_trans_type][new_trans_state]++;
  (*sea_units)[old_trans_type]--;
  (*sea_units)[new_trans_type]++;
}

inline NavySum* get_player_navies_ref(PlayerIndex player_idx, SeaIndex sea_idx) {
  return &player_navies[player_idx][sea_idx];
}

inline void set_sea_unit_type_sum(SeaUnitSumArray* sea_units, SeaUnitType unit_type,
                                  SeaUnitStateSum sum) {
  (*sea_units)[unit_type] = sum;
}

inline SeaTerr* get_sea_terr(SeaIndex sea_idx) { return &sea_state.sea_terr[sea_idx]; }

inline void acc_SeaNavySum(NavySum* sum, SeaUnitSumArray* seaUnitTypesSumArray,
                           SeaUnitType sea_unit_type) {
  *sum += (*seaUnitTypesSumArray)[sea_unit_type];
}

inline void acc_SeaUnitSumArray(SeaUnitSumArray* sea_units, SeaUnitType unit_type,
                                SeaUnitStates* sea_unit_states,
                                GenericSeaUnitState sea_unit_state) {
  (*sea_units)[unit_type] += (*(*sea_unit_states)[unit_type])[sea_unit_state];
}

inline SeaUnitSum get_sea_unit_sum(SeaUnitSumArray* sea_units, SeaUnitType unit_type) {
  return (*sea_units)[unit_type];
}

inline void acc_NavySumArray(NavySum* navy_sum, SeaUnitSumArray* sea_units, SeaUnitType unit_type) {
  *navy_sum += get_sea_unit_sum(sea_units, unit_type);
}

inline SeaUnitStates* get_my_sea_unit_states(SeaIndex sea_idx) {
  return &my_sea_unit_states[sea_idx];
}

inline Dollars get_money(PlayerIndex player_idx) { return sea_state.money[player_idx]; }

inline HitPoints get_factory_hp(LandIndex land_idx) { return *factory_hp[land_idx]; }

inline bool is_flagged_for_combat(AirIndex air_idx) {
  return sea_state.flagged_for_combat[air_idx];
}

inline LandUnitStateSumsPtr get_land_unit_state_sums(LandUnitStatesPtr land_unit_states,
                                                     LandUnitType unit_type) {
  return land_unit_states[unit_type];
}

inline LandUnitStateSum get_land_unit_state_sum_at(LandUnitStateSums* landUnitStateSums,
                                                   GenericLandUnitState unit_state) {
  return (*(*landUnitStateSums)[unit_state]);
}

inline SeaUnitStateSumsPtr get_sea_unit_state_sums(SeaUnitStatesPtr sea_unit_states,
                                                   SeaUnitType unit_type) {
  return sea_unit_states[unit_type];
}

inline SeaUnitStateSum get_sea_unit_state_sum_at(SeaUnitStateSums* seaUnitStateSums,
                                                 GenericSeaUnitState unit_state) {
  return (*(*seaUnitStateSums)[unit_state]);
}

inline AirIndex get_source_terr_count(LandIndex land_idx) {
  return hist_source_territories_count[land_idx];
}

inline AirIndexArray* get_source_territories(LandIndex land_idx) {
  return &hist_source_territories[land_idx];
}

inline AirIndex get_source_territory(AirIndexArray* source_territories,
                                     SourceTerritoryIndex source_terr_idx) {
  return (*source_territories)[source_terr_idx];
}

inline bool has_checked_territory(const CheckedTerritories checked_territories, AirIndex src_air) {
  return checked_territories[src_air];
}

inline void check_territory(bool* checked_territories, AirIndex src_air) {
  checked_territories[src_air] = true;
}

inline bool was_terr_skipped(AirIndex src_air, AirIndex dst_air) {
  return hist_skipped_airs[src_air][dst_air];
}
// inline LandConnections* get_l2l_conn(LandIndex land_idx) { return &LAND_TO_LAND_CONN[land_idx]; }
// inline LandIndex get_land_from_conn(LandConnections* land_to_land_conn, LandConnIndex conn_idx) {
//   return (*land_to_land_conn)[conn_idx];
// }
// inline SeaConnections* get_l2s_conn(LandIndex land_idx) { return &LAND_TO_SEA_CONN[land_idx]; }
// inline SeaConnIndex get_l2s_count(LandIndex land_idx) { return LAND_TO_SEA_COUNT[land_idx]; }

// inline LandArray* get_lands_within_2_moves(LandIndex land_idx) {
//   return &LANDS_WITHIN_2_MOVES[land_idx];
// }
inline LandIndex get_land_from_array(LandArray* land_array, LandConnIndex land_conn_idx) {
  return (*land_array)[land_conn_idx];
}
inline SeaIndex convert_air_to_sea(AirIndex air_idx) { return air_idx - LANDS_COUNT; }

inline char* get_air_name(AirIndex air_idx) {
  if (air_idx < LANDS_COUNT) {
    return get_land_name(air_idx);
  }
  return get_sea_name(convert_air_to_sea(air_idx));
}
inline LandIndex convert_air_to_land(AirIndex air_idx) { return air_idx; }

inline AirIndex get_air_from_a2a(AirConnection* air_conn, AirConnIndex air_conn_idx) {
  return (*air_conn)[air_conn_idx];
}

inline AirIndex get_air_from_array(AirIndexArray* air_array, AirConnIndex air_array_idx) {
  return (*air_array)[air_array_idx];
}

// inline SeaConnections* get_load_within_2_moves(LandIndex land_idx) {
//   return &LOAD_WITHIN_2_MOVES[land_idx];
// }

inline SeaIndex get_sea_from_array(SeaArray* sea_array, SeaConnIndex conn_idx) {
  return (*sea_array)[conn_idx];
}

inline bool is_non_combat_unit(LandUnitType unit_type) { return ATTACK_UNIT_LAND[unit_type] == 0; }

#define TRANSPORT_CARGO_SIZE 5

inline bool is_non_loadable_unit(LandUnitType unit_type) {
  return UNIT_WEIGHTS[unit_type] > TRANSPORT_CARGO_SIZE;
}

inline bool is_heavy_unit(LandUnitType unit_type) {
  return UNIT_WEIGHTS[unit_type] > INFANTRY_WEIGHT;
}

inline LandUnitType get_order_of_land_defenders(uint8_t idx) {
  return ORDER_OF_LAND_DEFENDERS[idx];
}

inline LandUnitType get_order_of_land_attackers_1(uint8_t idx) {
  return ORDER_OF_LAND_ATTACKERS_1[idx];
}
inline LandUnitType get_order_of_land_attackers_2(uint8_t idx) {
  return ORDER_OF_LAND_ATTACKERS_2[idx];
}

inline SeaUnitType get_order_of_sea_defenders(uint8_t idx) { return ORDER_OF_SEA_DEFENDERS[idx]; }

inline SeaUnitType get_order_of_sea_attackers_1(uint8_t idx) {
  return ORDER_OF_SEA_ATTACKERS_1[idx];
}

inline SeaUnitType get_order_of_sea_attackers_2(uint8_t idx) {
  return ORDER_OF_SEA_ATTACKERS_2[idx];
}
inline SeaUnitType get_order_of_sea_attackers_3(uint8_t idx) {
  return ORDER_OF_SEA_ATTACKERS_3[idx];
}

inline void acc_sea_state_sums(SeaUnitStateSums* sea_unit_states,
                               GenericSeaUnitState sea_unit_state, SeaUnitStateSum sum) {
  (*sea_unit_states)[sea_unit_state] += sum;
}

inline AirUnitStateSums* get_air_unit_state_sums(AirIndex air_idx, AirUnitType unit_type) {
  return &my_air_unit_states[air_idx][unit_type];
}

inline AirUnitStateSum* get_air_unit_state_sum_at(AirUnitStateSums* air_unit_state_sums,
                                                  FighterState cur_state) {
  return (*air_unit_state_sums)[cur_state];
}
inline AirUnitStateSum* get_bomber_state_sum_at(AirUnitStateSums* air_unit_state_sums,
                                                BomberLandState cur_state) {
  return (*air_unit_state_sums)[cur_state];
}

inline void flag_for_combat(AirIndex air_idx) { sea_state.flagged_for_combat[air_idx] = true; }
inline void unflag_for_combat(AirIndex air_idx) { sea_state.flagged_for_combat[air_idx] = false; }

inline void set_builds_left(LandIndex land_idx, BuildsLeft value) {
  sea_state.builds_left[land_idx] = value;
}

void generate_land_info_pointers() {
#pragma unroll
  for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    owner_idx[land_idx] = &sea_state.land_terr[land_idx].owner_idx;
    bombard_max[land_idx] = &sea_state.land_terr[land_idx].bombard_max;
    factory_hp[land_idx] = &sea_state.land_terr[land_idx].factory_hp;
    factory_max[land_idx] = &sea_state.land_terr[land_idx].factory_max;
  }
}

void initializeGameData() {
  generate_land_value_array();
  populate_initial_distances();
  floyd_warshall();
  initialize_sea_connections();
#pragma unroll
  for (CanalState canal_idx = 0; canal_idx < CANAL_STATES; canal_idx++) {
    initialize_sea_dist(canal_idx);
    populate_initial_distances_sea(canal_idx);
    process_canals(canal_idx);
    floyd_warshall_sea(canal_idx);
  }
  initialize_air_dist();
  populate_initial_distances();
  floyd_warshall_air();
  generate_landMoveAllDestination();
  generate_seaMoveAllDestination();
  generate_airMoveAllDestination();
  generate_within_x_moves();
  generate_random_numbers();
  generate_land_info_pointers();
  json = serialize_game_data_to_json(&sea_state);
  // write_json_to_file("game_data_0.json", json);
  cJSON_Delete(json);
}

void refresh_cache_alliance() {
  // copy for quick cache lookups
  enemies_count_0 = 0;
#pragma unroll
  for (PlayerIndex player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    set_ally(player_idx);
    if (!is_allied(player_idx)) {
      add_enemy(player_idx);
    }
  }
}
void refresh_cache_canals() {
  canal_state = 0;
#pragma unroll
  for (CanalState canal_idx = 0; canal_idx < CANALS_COUNT; canal_idx++) {
    if (is_canal_controlled(canal_idx)) {
      canal_state += 1U << canal_idx;
    }
  }
}

void refresh_cache_enemy_armies() {  
  // Replace memset with memset_s
  if (memset_s(&enemy_units_count, sizeof(enemy_units_count), 0, sizeof(enemy_units_count)) != 0) {
      // Handle error
      fprintf(stderr, "memset_s failed\n");
  }  for (LandIndex src_land = 0; src_land < LANDS_COUNT; src_land++) {
#pragma unroll 3
    for (EnemyIndex enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
      acc_enemy_units_count(convert_land_to_air(src_land),
                            get_player_armies(get_enemy_player(enemy_idx), src_land));
    }
#pragma unroll
    for (LandIndex dst_land = 0; dst_land < LANDS_COUNT; dst_land++) {
      set_is_land_path_blocked(src_land, dst_land);
    }
  }
}

void refresh_cache_enemy_navies() {
  memset(&enemy_destroyers, 0, sizeof(enemy_destroyers));
  memset(&enemy_blockade, 0, sizeof(enemy_blockade));
  for (SeaIndex src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    SeaUnitSumArray* sea_units_0 = get_my_sea_unit_types(src_sea);
    set_allied_carriers(src_sea, (*sea_units_0)[CARRIERS]);
    AirIndex air_idx = convert_sea_to_air(src_sea);
#pragma unroll
    for (PlayerIndex player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      SeaUnitSumArray* sea_units = get_player_sea_unit_types_ref(player_idx, src_sea);
      if (!is_allied(player_idx)) {
        acc_enemy_units_count(air_idx, get_player_navies(player_idx, src_sea));
        acc_enemy_destroyers(src_sea, sea_units);
        acc_enemy_blockade(src_sea, sea_units);
      } else {
        acc_allied_carriers(src_sea, (*sea_units)[CARRIERS]);
      }
    }
    recalc_transports_cargo_space(src_sea, sea_units_0);
#pragma unroll
    for (SeaIndex dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
      SeaIndex nextSeaMovement = get_sea_path1(canal_state, src_sea, dst_sea);
      SeaIndex nextSeaMovementAlt = get_sea_path1_alt(canal_state, src_sea, dst_sea);
      set_is_sea_path_blocked(src_sea, dst_sea, nextSeaMovement, nextSeaMovementAlt);
      set_is_sub_path_blocked(src_sea, dst_sea, nextSeaMovement, nextSeaMovementAlt);
    }
  }
}

void refresh_cache() {
  refresh_cache_alliance();
  refresh_cache_canals();
  refresh_cache_enemy_armies();
  refresh_cache_enemy_navies();
}

void generate_quick_state_land() {
  for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    LandTerr* land_terr = get_land_terr(land_idx);
    // LandUnitSumArray* land_units = get_my_land_unit_types(land_idx);
    LandUnitStates* land_unit_states = get_my_land_unit_states(land_idx);
    AirUnitStates* air_unit_states = get_my_air_unit_states(convert_land_to_air(land_idx));
    GenericLandUnitState land_unit_state = 0;
#pragma unroll 5
    for (land_unit_state = 0; land_unit_state < FIGHTER_STATES; land_unit_state++) {
      (*land_unit_states)[FIGHTERS_LAND][land_unit_state] = &land_terr->fighters[land_unit_state];
      (*air_unit_states)[FIGHTERS_AIR][land_unit_state] = &land_terr->fighters[land_unit_state];
    }
#pragma unroll 7
    for (land_unit_state = 0; land_unit_state < BOMBER_LAND_STATES; land_unit_state++) {
      (*land_unit_states)[BOMBERS_LAND][land_unit_state] = &land_terr->bombers[land_unit_state];
      (*air_unit_states)[BOMBERS_AIR][land_unit_state] = &land_terr->bombers[land_unit_state];
    }
#pragma unroll 2
    for (land_unit_state = 0; land_unit_state < INFANTRY_STATES; land_unit_state++) {
      (*land_unit_states)[INFANTRY][land_unit_state] = &land_terr->infantry[land_unit_state];
    }
#pragma unroll 2
    for (land_unit_state = 0; land_unit_state < ARTILLERY_STATES; land_unit_state++) {
      (*land_unit_states)[ARTILLERY][land_unit_state] = &land_terr->artillery[land_unit_state];
    }
#pragma unroll 3
    for (land_unit_state = 0; land_unit_state < TANK_STATES; land_unit_state++) {
      (*land_unit_states)[TANKS][land_unit_state] = &land_terr->tanks[land_unit_state];
    }
#pragma unroll 2
    for (land_unit_state = 0; land_unit_state < AA_GUN_STATES; land_unit_state++) {
      (*land_unit_states)[AA_GUNS][land_unit_state] = &land_terr->aa_guns[land_unit_state];
    }
  }
}

void set_seed(u_short new_seed) {
  seed = new_seed;
  random_number_index = new_seed;
}

void refresh_quick_totals_land() {
  memset(&my_land_unit_types, 0, sizeof(my_land_unit_types));
  memset(&player_armies, 0, sizeof(player_armies));
  for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    PlayerIndex land_owner = get_land_owner(land_idx);
    if (get_factory_max(land_idx) > 0) {
      add_factory_location(land_owner, land_idx);
    }
    acc_income_from_land(land_owner, land_idx);
    ArmySum* army_sum = get_player_armies_ref(0, land_idx);
    // LandTerr* land_terr = get_land_terr(land_idx);
    LandUnitSumArray* land_units = get_my_land_unit_types(land_idx);
    LandUnitStates* land_unit_states = get_my_land_unit_states(land_idx);

    for (LandUnitType unit_type = 0; unit_type < LAND_UNIT_TYPES_COUNT; unit_type++) {
#pragma unroll 7
      for (GenericLandUnitState land_unit_state = 0; land_unit_state < STATES_MOVE_LAND[unit_type];
           land_unit_state++) {
        acc_LandUnitSumArray(land_units, unit_type, land_unit_states, land_unit_state);
      }
      acc_ArmySumArray(army_sum, land_units, unit_type);
    }

    for (PlayerIndex player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      army_sum = get_player_armies_ref(player_idx, land_idx);
      land_units = get_player_land_unit_types_ref(player_idx, land_idx);
#pragma unroll
      for (LandUnitType unit_type = 0; unit_type < LAND_UNIT_TYPES_COUNT; unit_type++) {
        acc_ArmySumArray(army_sum, land_units, unit_type);
      }
    }
  }
}
void setFighterStates(SeaTerr* sea_terr, SeaUnitStates* sea_unit_states,
                      AirUnitStates* air_unit_states) {
#pragma unroll 5
  for (FighterState fighter_state = 0; fighter_state < FIGHTER_STATES; fighter_state++) {
    (*sea_unit_states)[FIGHTERS_SEA][fighter_state] = &sea_terr->fighters[fighter_state];
    (*air_unit_states)[FIGHTERS_AIR][fighter_state] = &sea_terr->fighters[fighter_state];
  }
}

void setTransportStates(SeaTerr* sea_terr, SeaUnitStates* sea_unit_states) {
  GenericSeaUnitState sea_state1 = 0;
#pragma unroll 4
  for (sea_state1 = 0; sea_state1 < TRANS_EMPTY_STATES; sea_state1++) {
    (*sea_unit_states)[TRANS_EMPTY][sea_state1] = &sea_terr->trans_empty[sea_state1];
  }
#pragma unroll 5
  for (sea_state1 = 0; sea_state1 < TRANS_1I_STATES; sea_state1++) {
    (*sea_unit_states)[TRANS_1I][sea_state1] = &sea_terr->trans_1i[sea_state1];
  }
#pragma unroll 5
  for (sea_state1 = 0; sea_state1 < TRANS_1A_STATES; sea_state1++) {
    (*sea_unit_states)[TRANS_1A][sea_state1] = &sea_terr->trans_1a[sea_state1];
  }
#pragma unroll 5
  for (sea_state1 = 0; sea_state1 < TRANS_1T_STATES; sea_state1++) {
    (*sea_unit_states)[TRANS_1T][sea_state1] = &sea_terr->trans_1t[sea_state1];
  }
#pragma unroll 4
  for (sea_state1 = 0; sea_state1 < TRANS_2I_STATES; sea_state1++) {
    (*sea_unit_states)[TRANS_2I][sea_state1] = &sea_terr->trans_2i[sea_state1];
  }
#pragma unroll 4
  for (sea_state1 = 0; sea_state1 < TRANS_1I_1A_STATES; sea_state1++) {
    (*sea_unit_states)[TRANS_1I_1A][sea_state1] = &sea_terr->trans_1i_1a[sea_state1];
  }
#pragma unroll 4
  for (sea_state1 = 0; sea_state1 < TRANS_1I_1T_STATES; sea_state1++) {
    (*sea_unit_states)[TRANS_1I_1T][sea_state1] = &sea_terr->trans_1i_1t[sea_state1];
  }
}

void setOtherUnitStates(SeaTerr* sea_terr, SeaUnitStates* sea_unit_states,
                        AirUnitStates* air_unit_states) {
  GenericSeaUnitState sea_state2 = 0;
#pragma unroll 3
  for (sea_state2 = 0; sea_state2 < SUBMARINE_STATES; sea_state2++) {
    (*sea_unit_states)[SUBMARINES][sea_state2] = &sea_terr->submarines[sea_state2];
  }
#pragma unroll 2
  for (sea_state2 = 0; sea_state2 < DESTROYER_STATES; sea_state2++) {
    (*sea_unit_states)[DESTROYERS][sea_state2] = &sea_terr->destroyers[sea_state2];
  }
#pragma unroll 2
  for (sea_state2 = 0; sea_state2 < CARRIER_STATES; sea_state2++) {
    (*sea_unit_states)[CARRIERS][sea_state2] = &sea_terr->carriers[sea_state2];
  }
#pragma unroll 2
  for (sea_state2 = 0; sea_state2 < CRUISER_STATES; sea_state2++) {
    (*sea_unit_states)[CRUISERS][sea_state2] = &sea_terr->cruisers[sea_state2];
  }
#pragma unroll 2
  for (sea_state2 = 0; sea_state2 < BATTLESHIP_STATES; sea_state2++) {
    (*sea_unit_states)[BATTLESHIPS][sea_state2] = &sea_terr->battleships[sea_state2];
  }
#pragma unroll 2
  for (sea_state2 = 0; sea_state2 < BATTLESHIP_STATES; sea_state2++) {
    (*sea_unit_states)[BS_DAMAGED][sea_state2] = &sea_terr->bs_damaged[sea_state2];
  }
#pragma unroll 5
  for (sea_state2 = 0; sea_state2 < BOMBER_SEA_STATES; sea_state2++) {
    (*sea_unit_states)[BOMBERS_SEA][sea_state2] = &sea_terr->bombers[sea_state2];
    (*air_unit_states)[BOMBERS_AIR][sea_state2] = &sea_terr->bombers[sea_state2];
  }
}

void generate_quick_state_sea() {
#pragma unroll
  for (SeaIndex sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    SeaTerr* sea_terr = get_sea_terr(sea_idx);
    SeaUnitStates* sea_unit_states = get_my_sea_unit_states(sea_idx);
    AirUnitStates* air_unit_states = get_my_air_unit_states(convert_sea_to_air(sea_idx));

    setFighterStates(sea_terr, sea_unit_states, air_unit_states);
    setTransportStates(sea_terr, sea_unit_states);
    setOtherUnitStates(sea_terr, sea_unit_states, air_unit_states);
  }
}

void refresh_quick_totals_sea() {
  memset(&my_sea_unit_types, 0, sizeof(my_sea_unit_types));
  memset(&player_navies, 0, sizeof(player_navies));
  for (SeaIndex sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    NavySum* navy_sum = get_player_navies_ref(0, sea_idx);
    SeaUnitSumArray* sea_units = get_my_sea_unit_types(sea_idx);
    SeaUnitStates* sea_unit_states = get_my_sea_unit_states(sea_idx);

    for (SeaUnitType unit_type = 0; unit_type < SEA_UNIT_TYPES_COUNT; unit_type++) {
#pragma unroll 5
      for (GenericSeaUnitState sea_unit_state = 0; sea_unit_state < STATES_MOVE_SEA[unit_type];
           sea_unit_state++) {
        acc_SeaUnitSumArray(sea_units, unit_type, sea_unit_states, sea_unit_state);
      }
      acc_NavySumArray(navy_sum, sea_units, unit_type);
    }

    for (PlayerIndex player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      navy_sum = get_player_navies_ref(player_idx, sea_idx);
      sea_units = get_player_sea_unit_types_ref(player_idx, sea_idx);
#pragma unroll
      for (SeaUnitType unit_type = 0; unit_type < LAND_UNIT_TYPES_COUNT; unit_type++) {
        *navy_sum += get_sea_unit_sum(sea_units, unit_type);
      }
    }
  }
}

void refresh_quick_totals() {
#pragma unroll
  for (PlayerIndex player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    set_income_per_turn(player_idx, 0);
    set_factory_count(player_idx, 0);
  }
  refresh_quick_totals_land();
  refresh_quick_totals_sea();
}

void load_game_data(char* filename) {
  if (getcwd(buffer, sizeof(buffer)) != NULL) {
    printf("Current working directory: ");
    printf("%s", buffer);
  } else {
    perror("getcwd() error");
  }
  json = read_json_from_file(filename);
  memset(&sea_state, 0, sizeof(sea_state));
  deserialize_game_data_from_json(&sea_state, json);
  cJSON_Delete(json);
  generate_quick_state_land();
  generate_quick_state_sea();
  refresh_quick_totals();
  refresh_cache();
  printf("Exiting load_game_data");
}
#ifdef DEBUG
void checkLandUnits() {
  for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    for (LandUnitType land_unit_idx = 0; land_unit_idx < LAND_UNIT_TYPES_COUNT; land_unit_idx++) {
      LandUnitStateSum temp_land_unit_total = 0;
#pragma unroll 7
      for (GenericLandUnitState cur_land_unit_state = 0;
           cur_land_unit_state < STATES_MOVE_LAND[land_unit_idx]; cur_land_unit_state++) {
        temp_land_unit_total +=
            *(my_land_unit_states[land_idx][land_unit_idx][cur_land_unit_state]);
      }
      if (temp_land_unit_total != my_land_unit_types[land_idx][land_unit_idx]) {
        printf("temp_unit_type_total %d != current_player_land_unit_types[land_idx][unit_idx] %d",
               temp_land_unit_total, my_land_unit_types[land_idx][land_unit_idx]);
      }
    }
  }
}

void checkSeaUnits() {
  for (SeaIndex sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    for (SeaUnitType sea_unit_idx = 0; sea_unit_idx < SEA_UNIT_TYPES_COUNT; sea_unit_idx++) {
      SeaUnitStateSum temp_sea_unit_total = 0;
#pragma unroll 5
      for (GenericSeaUnitState cur_sea_unit_state = 0;
           cur_sea_unit_state < STATES_MOVE_SEA[sea_unit_idx]; cur_sea_unit_state++) {
        temp_sea_unit_total += *(my_sea_unit_states[sea_idx][sea_unit_idx][cur_sea_unit_state]);
      }
      if (temp_sea_unit_total != my_sea_unit_types[sea_idx][sea_unit_idx]) {
        printf("temp_unit_type_total %d != current_player_sea_unit_types[sea_idx][unit_idx] %d",
               temp_sea_unit_total, my_sea_unit_types[sea_idx][sea_unit_idx]);
      }
    }
  }
}
void sumLandUnits(PlayerIndex player_idx, LandIndex land_idx, LandUnitSum* land_unit_sum,
                  LandUnitSum* total_land_data) {
#pragma unroll 6
  for (LandUnitType land_unit_idx = 0; land_unit_idx < LAND_UNIT_TYPES_COUNT; land_unit_idx++) {
    if (player_idx == 0 || land_unit_idx < LAND_UNIT_TYPES_COUNT) {
      *land_unit_sum += (*(player_land_unit_types[player_idx]))[land_idx][land_unit_idx];
    }
    if (player_idx == 0) {
      *total_land_data += my_land_unit_types[land_idx][land_unit_idx];
    } else {
      if (land_unit_idx < LAND_UNIT_TYPES_COUNT) {
        *total_land_data += sea_state.other_land_units[player_idx - 1][land_idx][land_unit_idx];
      }
    }
  }
}

void checkLandUnitSums(PlayerIndex player_idx, LandIndex land_idx, LandUnitSum land_unit_sum,
                       LandUnitSum total_land_data) {
  if (land_unit_sum != player_armies[player_idx][land_idx] || land_unit_sum != total_land_data) {
    printf("total %d != total_player_land_units[player_idx][land_idx] %d != total_data %d",
           land_unit_sum, player_armies[player_idx][land_idx], total_land_data);
  }

  if (player_armies[player_idx][land_idx] < 0 ||
      player_armies[player_idx][land_idx] > NEAR_UINT8_T_OVERFLOW) {
    printf("units_land_player_total[player_idx][land_idx] < 0");
  }
  if (enemy_units_count[land_idx] < 0 || enemy_units_count[land_idx] > NEAR_UINT8_T_OVERFLOW) {
    printf("enemy_units_count[land_idx] < 0");
  }
}

void checkLandUnitStates(PlayerIndex player_idx, LandIndex land_idx) {
  LandUnitSum land_unit_sum = 0;
#pragma unroll 6
  for (LandUnitType land_unit_idx = 0; land_unit_idx < LAND_UNIT_TYPES_COUNT; land_unit_idx++) {
    if ((*(player_land_unit_types[player_idx]))[land_idx][land_unit_idx] < 0 ||
        (*(player_land_unit_types[player_idx]))[land_idx][land_unit_idx] > NEAR_UINT8_T_OVERFLOW) {
      printf("total_player_land_unit_types[player_idx][land_idx][unit_idx] < 0");
    }
    land_unit_sum += (*(player_land_unit_types[player_idx]))[land_idx][land_unit_idx];
  }
  if (land_unit_sum != player_armies[player_idx][land_idx]) {
    printf("total %d != total_player_land_units[player_idx][land_idx] %d", land_unit_sum,
           player_armies[player_idx][land_idx]);
  }
}

void checkEnemyUnits(LandIndex land_idx) {
  ArmySum enemy_total = 0;
#pragma unroll 3
  for (EnemyIndex enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
    PlayerIndex enemy_player_idx = enemies_0[enemy_idx];
    enemy_total += player_armies[enemy_player_idx][land_idx];
  }
  if (enemy_total != enemy_units_count[land_idx]) {
    printf("enemy_total %d != enemy_units_count[land_idx] %d", enemy_total,
           enemy_units_count[land_idx]);
  }
}

void checkLandUnitsForPlayer(PlayerIndex player_idx) {
#pragma unroll
  for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    LandUnitSum land_unit_sum = 0;
    LandUnitSum total_land_data = 0;
    sumLandUnits(player_idx, land_idx, &land_unit_sum, &total_land_data);
    checkLandUnitSums(player_idx, land_idx, land_unit_sum, total_land_data);
    checkLandUnitStates(player_idx, land_idx);
    checkEnemyUnits(land_idx);
  }
}
void checkPlayerNavies(PlayerIndex player_idx, SeaIndex sea_idx) {
  if (player_navies[player_idx][sea_idx] < 0 ||
      player_navies[player_idx][sea_idx] > NEAR_UINT8_T_OVERFLOW) {
    printf("units_sea_player_total[player_idx][sea_idx] < 0");
  }
}

void checkSeaUnitTypes(PlayerIndex player_idx, SeaIndex sea_idx) {
#pragma unroll 15
  for (SeaUnitType sea_unit_idx = 0; sea_unit_idx < SEA_UNIT_TYPES_COUNT; sea_unit_idx++) {
    if ((*(player_sea_unit_types[player_idx]))[sea_idx][sea_unit_idx] < 0 ||
        (*(player_sea_unit_types[player_idx]))[sea_idx][sea_unit_idx] > NEAR_UINT8_T_OVERFLOW) {
      printf("total_player_sea_unit_types[player_idx][sea_idx][unit_idx] < 0");
    }
  }
}

void checkEnemyUnitsCount(SeaIndex sea_idx) {
  if (enemy_units_count[sea_idx + LANDS_COUNT] < 0 ||
      enemy_units_count[sea_idx + LANDS_COUNT] > NEAR_UINT8_T_OVERFLOW) {
    printf("enemy_units_count[land_idx] < 0");
  }
}

void sumSeaUnits(PlayerIndex player_idx, SeaIndex sea_idx, int* total, int* total_data) {
#pragma unroll
  for (int unit_idx = 0; unit_idx < SEA_UNIT_TYPES_COUNT; unit_idx++) {
    if (player_idx == 0 || unit_idx < SEA_UNIT_TYPES_COUNT - 1) {
      *total += (*(player_sea_unit_types[player_idx]))[sea_idx][unit_idx];
    }
    if (player_idx == 0) {
      *total_data += my_sea_unit_types[sea_idx][unit_idx];
    } else {
      if (unit_idx < SEA_UNIT_TYPES_COUNT - 1) {
        *total_data += sea_state.other_sea_units[player_idx - 1][sea_idx][unit_idx];
      }
    }
  }
}

void checkSeaUnitSums(PlayerIndex player_idx, SeaIndex sea_idx, int total, int total_data) {
  if (total != player_navies[player_idx][sea_idx] || total != total_data) {
    printf("total %d != total_player_sea_units[player_idx][sea_idx] %d != total_data %d", total,
           player_navies[player_idx][sea_idx], total_data);
  }
}

void checkEnemySeaUnits(SeaIndex sea_idx) {
  int enemy_total = 0;
#pragma unroll 3
  for (EnemyIndex enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
    PlayerIndex enemy_player_idx = enemies_0[enemy_idx];
    enemy_total += player_navies[enemy_player_idx][sea_idx];
  }
  if (enemy_total != enemy_units_count[sea_idx + LANDS_COUNT]) {
    printf("enemy_total %d != enemy_units_count[sea_idx + LANDS_COUNT] %d", enemy_total,
           enemy_units_count[sea_idx + LANDS_COUNT]);
  }
}

void checkSeaUnitsForPlayer(PlayerIndex player_idx) {
#pragma unroll
  for (SeaIndex sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    checkPlayerNavies(player_idx, sea_idx);
    checkSeaUnitTypes(player_idx, sea_idx);
    checkEnemyUnitsCount(sea_idx);

    int total = 0;
    int total_data = 0;
    sumSeaUnits(player_idx, sea_idx, &total, &total_data);
    checkSeaUnitSums(player_idx, sea_idx, total, total_data);
    checkEnemySeaUnits(sea_idx);
  }
}
void checkPlayerArmies() {
#pragma unroll
  for (PlayerIndex player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    checkLandUnitsForPlayer(player_idx);
    checkSeaUnitsForPlayer(player_idx);
  }
}
void debug_checks() {
  step_id++;
  printf("step_id: %d  seed: %d\n", step_id, seed);
  checkLandUnits();
  checkSeaUnits();
  checkPlayerArmies();
}
#endif

void appendLandStatus(LandIndex land_idx, char* buffer) {
  PlayerIndex land_owner = get_land_owner(land_idx);
  int result =
      sprintf(buffer, "%s%d %s: %s %d/%d/%d/%d Combat:%s\n", get_player_color(land_owner), land_idx,
              get_land_name(land_idx), get_player_name(land_owner), sea_state.builds_left[land_idx],
              get_factory_hp(land_idx), get_factory_max(land_idx), get_land_value(land_idx),
              is_flagged_for_combat(convert_land_to_air(land_idx)) ? "true" : "false");
  if (result >= 0) {
    strcat(printableGameStatus, buffer);
  }
}

void appendArmyStatus(LandIndex land_idx, char* buffer) {
  ArmySum all_players_armies = 0;
#pragma unroll
  for (PlayerIndex player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    all_players_armies += get_player_armies(player_idx, land_idx);
  }
  if (all_players_armies == 0) {
    strcat(printableGameStatus, "\033[0m");
    return;
  }
  strcat(printableGameStatus, "                 |Tot| 0| 1| 2| 3| 4| 5| 6|\n");
  if (get_player_armies(0, land_idx) == 0) {
    return;
  }
  strcat(printableGameStatus, get_player_color(sea_state.player_index));
  LandUnitSumArray* land_units = get_my_land_unit_types(land_idx);
  for (LandUnitType unit_type = 0; unit_type < LAND_UNIT_TYPES_COUNT; unit_type++) {
    LandUnitSum unit_sum = get_land_unit_sum(land_units, unit_type);
    if (unit_sum > 0) {
      int offset = 0;
      offset += sprintf(buffer + offset, "%s %-14s %3d", get_player_name(sea_state.player_index),
                        NAMES_UNIT_LAND[unit_type], unit_sum);
      LandUnitStateSums* landUnitStateSums =
          get_land_unit_state_sums(get_my_land_unit_states(land_idx), unit_type);
#pragma unroll 7
      for (GenericLandUnitState unit_state = 0; unit_state < STATES_MOVE_LAND[unit_type];
           unit_state++) {
        offset += sprintf(buffer + offset, "%3d",
                          get_land_unit_state_sum_at(landUnitStateSums, unit_state));
      }
      strcat(buffer, "\n");
      strcat(printableGameStatus, buffer);
    }
    strcat(printableGameStatus, "\033[0m");
    for (PlayerIndex player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      if (get_player_armies(player_idx, land_idx) == 0) {
        continue;
      }
      int offset = 0;
      offset += sprintf(buffer + offset, "%s", get_player_color(player_idx));
      LandUnitSumArray* land_units2 = get_player_land_unit_types_ref(player_idx, land_idx);
#pragma unroll
      for (LandUnitType unit_type2 = 0; unit_type2 < LAND_UNIT_TYPES_COUNT; unit_type2++) {
        LandUnitSum unit_sum2 = get_land_unit_sum(land_units2, unit_type2);
        if (unit_sum2 > 0) {
          offset += sprintf(buffer + offset, "%s %-14s %3d\n", get_player_name(player_idx),
                            NAMES_UNIT_LAND[unit_type2], unit_sum2);
        }
      }
      offset += sprintf(buffer + offset, "\033[0m");
      strcat(printableGameStatus, buffer);
    }
    strcat(printableGameStatus, "\n");
  }
}

void setPrintableStatusLands() {
#pragma unroll
  for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    appendLandStatus(land_idx, buffer);
    appendArmyStatus(land_idx, buffer);
  }
}
void processPlayerNavies(SeaIndex sea_idx, char* my_name, SeaUnitStates* sea_unit_states) {
  if (get_player_navies(0, sea_idx) != 0) {
    SeaUnitSumArray* sea_units = get_my_sea_unit_types(sea_idx);
    for (SeaUnitType unit_type = 0; unit_type < SEA_UNIT_TYPES_COUNT; unit_type++) {
      SeaUnitSum unit_sum = get_sea_unit_sum(sea_units, unit_type);
      if (unit_sum > 0) {
        (void)sprintf(buffer, "%s %-14s%3d", my_name, NAMES_UNIT_SEA[unit_type], unit_sum);
        strcat(printableGameStatus, buffer);

        SeaUnitStateSums* seaUnitStateSums = get_sea_unit_state_sums(sea_unit_states, unit_type);
#pragma unroll 5
        for (GenericSeaUnitState unit_state = 0; unit_state < STATES_MOVE_SEA[unit_type];
             unit_state++) {
          (void)sprintf(buffer, "%3d", get_sea_unit_state_sum_at(seaUnitStateSums, unit_state));
          strcat(printableGameStatus, buffer);
        }
        strcat(printableGameStatus, "\n");
      }
    }
  }
}

void processOtherPlayersNavies(SeaIndex sea_idx) {
  for (PlayerIndex player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
    if (get_player_navies(player_idx, sea_idx) != 0) {
      (void)sprintf(buffer, "%s", get_player_color(player_idx));
      strcat(printableGameStatus, buffer);

      SeaUnitSumArray* sea_units = get_player_sea_unit_types_ref(player_idx, sea_idx);
#pragma unroll
      for (SeaUnitType unit_type = 0; unit_type < SEA_UNIT_TYPES_COUNT - 1; unit_type++) {
        SeaUnitSum unit_sum = (*sea_units)[unit_type];
        if (unit_sum > 0) {
          (void)sprintf(buffer, "%s %-14s%3d\n", get_player_name(player_idx),
                        NAMES_UNIT_SEA[unit_type], unit_sum);
          strcat(printableGameStatus, buffer);
        }
      }
      strcat(printableGameStatus, "\033[0m");
    }
  }
}

void setPrintableStatusSeas() {
  PlayerIndex player_idx = sea_state.player_index;
  char* my_color = get_player_color(player_idx);
  char* my_name = get_player_name(player_idx);
  for (SeaIndex sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    SeaUnitStates* sea_unit_states = get_my_sea_unit_states(sea_idx);
    NavySum all_players_navies = 0;
#pragma unroll
    for (PlayerIndex player_idx2 = 0; player_idx2 < PLAYERS_COUNT; player_idx2++) {
      all_players_navies += get_player_navies(player_idx2, sea_idx);
    }
    if (all_players_navies == 0) {
      continue;
    }
    (void)sprintf(buffer, "%d %s Combat:%s\n                 |Tot| 0| 1| 2| 3| 4| 5| 6|\n%s",
                  convert_sea_to_air(sea_idx), get_sea_name(sea_idx),
                  is_flagged_for_combat(convert_sea_to_air(sea_idx)) ? "true" : "false", my_color);
    strcat(printableGameStatus, buffer);

    processPlayerNavies(sea_idx, my_name, sea_unit_states);
    strcat(printableGameStatus, "\033[0m");

    processOtherPlayersNavies(sea_idx);
  }
}
void setPrintableStatus() {
#ifdef DEBUG
  debug_checks();
#endif
  printableGameStatus[0] = '\0';

  strcat(printableGameStatus, "---\n");
  setPrintableStatusLands();
  setPrintableStatusSeas();
  strcat(printableGameStatus, "\n");

  PlayerIndex player_idx = sea_state.player_index;
  (void)sprintf(buffer, "%s%s\033[0m: %d IPC\n", get_player_color(player_idx),
                get_player_name(player_idx), get_money(0));
  strcat(printableGameStatus, buffer);
}

Action getUserInput() {
  while (true) {
    // 0-valid_moves_count
    // printf("Enter a number between 0 and 255: ");
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
      char* endptr = NULL;
      long temp_input = strtol(buffer, &endptr, BASE10);
      int user_input = 0;
      if (temp_input >= INT_MIN && temp_input <= INT_MAX) {
        user_input = (int)temp_input;
      } else {
        // Handle the error case where the input is out of range
        user_input = 0; // or some other default/error value
      }
      if (*endptr == '\n' || *endptr == '\0') {
#pragma unroll 4
        for (int i = 0; i < valid_actions_count; i++) {
          if (user_input == valid_actions[i]) {
            return (Action)user_input;
          }
        }
        return valid_actions[0];
      }
    }
  }
}
Action getAIInput() {
#ifdef DEBUG
  printf("selecting random action %d\n", get_random_number(random_number_index));
#endif
  answers_remaining--;
  if (answers_remaining == 0) {
#ifdef DEBUG
    bool action_found = false;
#pragma unroll 4
    for (int i = 0; i < valid_actions_count; i++) {
      if (valid_actions[i] == selected_answer) {
        action_found = true;
        break;
      }
    }
    if (!action_found) {
      printf("selected_answer %d not found in valid_actions\n", selected_answer);
    }
#endif
    return selected_answer;
  }
  return valid_actions[get_random_number(random_number_index++) % valid_actions_count];
}

// Notes on how allowed moves from history are calculated:
// When final unit for a unit_type is moved, set bool SkippedMoves[SourceTerr][DestTerr] = {0}
// Loop through territories with units to move
// 	I. Set LowestActionYet to be 256, or the lowest number set to True in bool
// SkippedMoves[SourceTerr][DestTerr] 	II. After a move is taken, loop as follows:
// 		1. if new action is < lowestaction yet
// 			a. loop each action that was now marked as skipped
// 				i. SkippedMove[source_terr][skipped_terr] = true
// 		2. get the distance to the destination
// 		3. loop through every territory with range of 4 of destination
// 			a. continue loop if iterated_territory's distance to destination is less
// than source territory's distance 			b. loop each action that was now marked as
// skipped 				i. Set SkippedMove[iterated_terr][skipped_terr] = true

bool check_valid1(AirIndex shared_dst, CheckedTerritories checked_territories, AirIndex dst_air) {
  AirIndexArray* source_territories = get_source_territories(shared_dst);
  AirIndex source_terr_count = get_source_terr_count(shared_dst);
#pragma unroll
  for (SourceTerritoryIndex source_terr_idx = 0; source_terr_idx < AIRS_COUNT; source_terr_idx++) {
    if (source_terr_idx >= source_terr_count) {
      break;
    }
    AirIndex src_air = get_source_territory(source_territories, source_terr_idx);
    if (has_checked_territory(checked_territories, src_air)) {
      continue;
    }
    check_territory(checked_territories, src_air);
    if (was_terr_skipped(src_air, dst_air)) {
      return false;
    }
  }
  return true;
}

bool check_land_connectionsX(LandIndex src_land, bool* checked_territories, AirIndex dst_air) {
  LandConnections* land_to_land_conn = get_l2l_conn(src_land);
  LandIndex land_to_land_count = get_l2l_count(src_land);
#pragma unroll
  for (LandConnIndex conn_idx = 0; conn_idx < MAX_LAND_TO_LAND_CONNECTIONS; conn_idx++) {
    if (conn_idx >= land_to_land_count) {
      break;
    }
    if (!check_valid1(convert_land_to_air(get_land_from_conn(land_to_land_conn, conn_idx)),
                      checked_territories, dst_air)) {
      return false;
    }
  }
  return true;
}

bool check_sea_connectionsX(LandIndex src_land, bool* checked_territories, AirIndex dst_air) {
  SeaConnections* land_to_sea_conn = get_l2s_conn(src_land);
  SeaIndex land_to_sea_count = get_l2s_count(src_land);
#pragma unroll
  for (SeaConnIndex conn_idx = 0; conn_idx < MAX_LAND_TO_SEA_CONNECTIONS; conn_idx++) {
    if (conn_idx >= land_to_sea_count) {
      break;
    }
    if (!check_valid1(convert_sea_to_air(get_sea_from_conn(land_to_sea_conn, conn_idx)),
                      checked_territories, dst_air)) {
      return false;
    }
  }
  return true;
}

bool check_lands_within_2_movesX(LandIndex src_land, bool* checked_territories, AirIndex dst_air) {
  LandArray* lands_within_2_moves = get_lands_within_2_moves(src_land);
  LandIndex lands_within_2_moves_count = get_lands_within_2_moves_count(src_land);
#pragma unroll
  for (LandConnIndex land_array_idx = 0; land_array_idx < LANDS_COUNT; land_array_idx++) {
    if (land_array_idx >= lands_within_2_moves_count) {
      break;
    }
    if (!check_valid1(
            convert_land_to_air(get_land_from_array(lands_within_2_moves, land_array_idx)),
            checked_territories, dst_air)) {
      return false;
    }
  }
  return true;
}

bool check_airs_within_X_moves(AirIndex src_air, bool* checked_territories, AirIndex dst_air,
                               Distance moves) {
  AirIndexArray* airs_within_x_moves = get_airs_winthin_x_moves(moves, src_air);
  AirIndex airs_within_x_moves_count = get_airs_winthin_x_moves_count(moves, src_air);
#pragma unroll
  for (AirConnIndex air_array_idx = 0; air_array_idx < AIRS_COUNT; air_array_idx++) {
    if (air_array_idx >= airs_within_x_moves_count) {
      break;
    }
    if (!check_valid1(get_air_from_array(airs_within_x_moves, air_array_idx), checked_territories,
                      dst_air)) {
      return false;
    }
  }
  return true;
}

bool check_load_within_2_movesX(LandIndex src_land, bool* checked_territories, AirIndex dst_air) {
  LandIndex load_within_2_moves_count = get_load_within_2_moves_count(src_land);
  SeaArray* load_within_2_moves = get_load_within_2_moves(src_land);
#pragma unroll
  for (SeaConnIndex conn_idx = 0; conn_idx < SEAS_COUNT; conn_idx++) {
    if (conn_idx >= load_within_2_moves_count) {
      break;
    }
    if (!check_valid1(convert_sea_to_air(get_sea_from_array(load_within_2_moves, conn_idx)),
                      checked_territories, dst_air)) {
      return false;
    }
  }
  return true;
}

bool check_seas_within_1_moveX(SeaIndex src_sea, bool* checked_territories, AirIndex dst_air) {
  SeaIndex seas_within_1_move_count = get_seas_within_1_move_count(canal_state, src_sea);
  SeaArray* seas_within_1_move = get_seas_within_1_move(canal_state, src_sea);
#pragma unroll
  for (SeaConnIndex conn_idx = 0; conn_idx < MAX_SEA_TO_SEA_CONNECTIONS; conn_idx++) {
    if (conn_idx >= seas_within_1_move_count) {
      break;
    }
    if (!check_valid1(convert_sea_to_air(get_sea_from_array(seas_within_1_move, conn_idx)),
                      checked_territories, dst_air)) {
      return false;
    }
  }
  return true;
}

bool check_seas_within_2_movesX(SeaIndex src_sea, bool* checked_territories, AirIndex dst_air) {
  SeaIndex seas_within_2_moves_count = get_seas_within_2_moves_count(canal_state, src_sea);
  SeaArray* seas_within_2_moves = get_seas_within_2_moves(canal_state, src_sea);
#pragma unroll
  for (SeaConnIndex conn_idx = 0; conn_idx < SEAS_COUNT; conn_idx++) {
    if (conn_idx >= seas_within_2_moves_count) {
      break;
    }
    if (!check_valid1(convert_sea_to_air(get_sea_from_array(seas_within_2_moves, conn_idx)),
                      checked_territories, dst_air)) {
      return false;
    }
  }
  return true;
}
void add_valid_land_move_if_history_allows_1(AirIndex dst_air, LandIndex src_land) {
  bool checked_territories[AIRS_COUNT] = {0};
  if (check_land_connectionsX(src_land, checked_territories, dst_air) &&
      check_sea_connectionsX(src_land, checked_territories, dst_air)) {
    valid_actions[valid_actions_count++] = dst_air;
  }
}

void add_valid_land_move_if_history_allows_2(AirIndex dst_air, LandIndex src_land) {
  bool checked_territories[AIRS_COUNT] = {0};
  if (check_lands_within_2_movesX(src_land, checked_territories, dst_air) &&
      check_load_within_2_movesX(src_land, checked_territories, dst_air)) {
    valid_actions[valid_actions_count++] = dst_air;
  }
}

void add_valid_sea_move_if_history_allows_1(SeaIndex dst_sea, SeaIndex src_sea) {
  bool checked_territories[AIRS_COUNT] = {0};
  AirIndex dst_air = convert_sea_to_air(dst_sea);
  if (check_seas_within_1_moveX(src_sea, checked_territories, dst_air)) {
    valid_actions[valid_actions_count++] = dst_air;
  }
}

void add_valid_sea_move_if_history_allows_2(SeaIndex dst_sea, SeaIndex src_sea) {
  bool checked_territories[AIRS_COUNT] = {0};
  AirIndex dst_air = convert_sea_to_air(dst_sea);
  if (check_seas_within_2_movesX(src_sea, checked_territories, dst_air)) {
    valid_actions[valid_actions_count++] = dst_air;
  }
}

void add_valid_air_move_if_history_allows_X(AirIndex dst_air, AirIndex src_air, Distance moves) {
  bool checked_territories[AIRS_COUNT] = {0};
  if (check_airs_within_X_moves(src_air, checked_territories, dst_air, moves)) {
    valid_actions[valid_actions_count++] = dst_air;
  }
}

void clear_move_history() {
  memset(hist_skipped_airs, 0, sizeof(hist_skipped_airs));
  memset(hist_source_territories_count, 0, sizeof(hist_source_territories_count));
}

Action get_user_purchase_input(AirIndex src_air) {
  Action user_input = 0;
  if (is_player_human(sea_state.player_index)) {
    setPrintableStatus();
    strcat(printableGameStatus, "Purchasing at ");
    if (src_air < LANDS_COUNT) {
      strcat(printableGameStatus, get_air_name(src_air));
      int offset = sprintf(buffer, " %d=Finished ", valid_actions[0]);
      strcat(printableGameStatus, buffer);
#pragma unroll 4
      for (int i = 1; i < valid_actions_count; i++) {
        int offset = sprintf(buffer, "%d=%s ", valid_actions[i], NAMES_UNIT_LAND[valid_actions[i]]);
        strcat(printableGameStatus, buffer);
      }
    } else {
      strcat(printableGameStatus, get_air_name(src_air));
      int offset = sprintf(buffer, " %d=Finished ", valid_actions[0]);
      strcat(printableGameStatus, buffer);
#pragma unroll 4
      for (int i = 1; i < valid_actions_count; i++) {
        int offset = sprintf(buffer, "%d=%s ", valid_actions[i], NAMES_UNIT_SEA[valid_actions[i]]);
        strcat(printableGameStatus, buffer);
      }
    }
    printf("%s\n", printableGameStatus);
    return getUserInput();
  }
  return getAIInput();
}

Action get_user_input_generic(const char* unit_name, char* terr_name) {
  Action user_input = 0;
  if (is_player_human(sea_state.player_index)) {
    setPrintableStatus();
    strcat(printableGameStatus, "Moving ");
    strcat(printableGameStatus, unit_name);
    strcat(printableGameStatus, " From: ");
    strcat(printableGameStatus, terr_name);
    strcat(printableGameStatus, " Valid Moves: ");
#pragma unroll 4
    for (int i = 0; i < valid_actions_count; i++) {
      int offset = sprintf(buffer, "%d ", valid_actions[i]);
      strcat(printableGameStatus, buffer);
    }
    printf("%s\n", printableGameStatus);
    return getUserInput();
  }
  return getAIInput();
}

Action get_user_input_land(LandUnitType unit_type, LandIndex land_idx) {
  return get_user_input_generic(NAMES_UNIT_LAND[unit_type], get_land_name(land_idx));
}

Action get_user_input_sea(SeaUnitType unit_type, SeaIndex sea_idx) {
  return get_user_input_generic(NAMES_UNIT_SEA[unit_type], get_sea_name(sea_idx));
}
void update_move_history(Action user_input, AirIndex src_air) {
  // hist_source_territories is a list of terrirtories that moved a unit into a specific territory
  // format is:
  //  hist_source_territories[dst_air][0] = src_air
  //  hist_source_territories[dst_air][1] = src_air2
  if (hist_source_territories[user_input][hist_source_territories_count[user_input]] == src_air) {
    return;
  }
  hist_source_territories[user_input][hist_source_territories_count[user_input]] = src_air;
  hist_source_territories_count[user_input]++;
#pragma unroll 4
  for (int move_index2 = 1; move_index2 < AIRS_COUNT; move_index2++) {
    if (move_index2 > valid_actions_count) {
      break;
    }
    Action move = valid_actions[valid_actions_count - move_index2];
    if (move == user_input) {
      break;
    }
    hist_skipped_airs[src_air][move] = true;
    valid_actions_count--;
  }
}

bool load_transport(LandUnitType unit_type, LandIndex src_land, SeaIndex dst_sea,
                    GenericLandUnitState land_unit_state) {
#ifdef DEBUG
  printf("load_transport: unit_type=%d src_land=%d dst_sea=%d land_unit_state=%d\n", unit_type,
         src_land, dst_sea, land_unit_state);
#endif
  const SeaUnitType* load_unit_type = LOAD_UNIT_TYPE[unit_type];
  SeaUnitStates* sea_unit_states = get_my_sea_unit_states(dst_sea);
  for (int trans_type_idx = 0; trans_type_idx < 4; trans_type_idx++) {
    if (UNIT_WEIGHTS[unit_type] > 2 && trans_type_idx < 2) {
      continue;
    }
    SeaUnitType trans_type = TRANSPORT_ORDER[trans_type_idx];
    SeaUnitStateSums* units_sea_ptr_dst_sea_trans_type =
        get_sea_unit_state_sums(sea_unit_states, trans_type);
    SeaUnitStateCount states_unloading = STATES_UNLOADING[trans_type];
    // SeaUnitStateCount state_highest_done_staging = STATES_MOVE_SEA[trans_type] -
    // STATES_STAGING[trans_type]; state_highest_done_staging represents the done staging status if
    // no moves occured - MIN 3, MAX 4 stages_unloading is the status of the transport_type that is
    // still allowed to unload - MIN 0, MAX 1
#pragma unroll
    for (GenericSeaUnitState trans_state1 = 0; trans_state1 < 4; trans_state1++) {
      GenericSeaUnitState trans_state = states_unloading + trans_state1;
      if (units_sea_ptr_dst_sea_trans_type[trans_state] > 0) {
        SeaUnitType new_trans_type = load_unit_type[trans_type];
        (*units_sea_ptr_dst_sea_trans_type[trans_state])--;
        GenericSeaUnitState new_trans_state = trans_state;
        if (trans_type == TRANS_EMPTY && trans_state == TRANS_EMPTY_UNLOADING_STATES) {
          new_trans_state = STATES_UNLOADING[new_trans_type]; // empty transports don't have an
                                                              // "unloading" state
        }
        SeaUnitSumArray* sea_units = get_my_sea_unit_types(dst_sea);
        replace_transport(sea_unit_states, sea_units, new_trans_type, new_trans_state, trans_type,
                          trans_state);
        remove_my_land_unit_state(src_land, unit_type, land_unit_state);
        recalc_transports_cargo_space(dst_sea, sea_units);
#ifdef DEBUG
        debug_checks();
#endif
        return true;
      }
    }
  }
  printf("Error: Failed to load transport\n");
  return false;
}

void add_valid_land_moves_within_2(LandIndex src_land, AirDistances* land_dist) {
  // check for moving from land to land (two moves away)
  LandIndex lands_within_2_moves_count = get_lands_within_2_moves_count(src_land);
  LandArray* lands_within_2_moves = get_lands_within_2_moves(src_land);
  bool* is_land_path_blocked_src_land = is_land_path_blocked[src_land];
#pragma unroll
  for (LandConnIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    if (land_idx >= lands_within_2_moves_count) {
      break;
    }
    LandIndex dst_land = get_land_from_array(lands_within_2_moves, land_idx);
    if (get_air_from_array(land_dist, dst_land) == 1) {
      // add_valid_land_move_if_history_allows_2(convert_land_to_air(dst_land), src_land);

      valid_actions[valid_actions_count++] = convert_land_to_air(dst_land);
      continue;
    }
    if (is_land_path_blocked_src_land[dst_land]) {
      continue;
    }
    valid_actions[valid_actions_count++] = convert_land_to_air(dst_land);
  }
}

void add_valid_sea_moves_within_2(LandIndex src_land, AirDistances* land_dist) {
  LandIndex load_within_2_moves_count = get_load_within_2_moves_count(src_land);
  SeaArray* load_within_2_moves = get_load_within_2_moves(src_land);
  bool* is_land_path_blocked_src_land = is_land_path_blocked[src_land];
#pragma unroll
  for (SeaConnIndex conn_idx = 0; conn_idx < SEAS_COUNT; conn_idx++) {
    if (conn_idx >= load_within_2_moves_count) {
      break;
    }
    SeaIndex dst_sea = get_sea_from_array(load_within_2_moves, conn_idx);
    if (transports_with_large_cargo_space[dst_sea] == 0) {
      continue;
    }
    AirIndex dst_air = convert_sea_to_air(dst_sea);
    if (get_air_from_array(land_dist, dst_air) == 1) {
      // add_valid_land_move_if_history_allows_2(convert_land_to_air(dst_land), src_land);
      valid_actions[valid_actions_count++] = dst_air;
      continue;
    }
    if (is_land_path_blocked_src_land[dst_air]) {
      continue;
    }
    valid_actions[valid_actions_count++] = dst_air;
  }
}

void add_valid_land_moves_within_1(LandIndex src_land, LandUnitType unit_type) {
  LandIndex lands_within_1_move_count = get_l2l_count(src_land);
  LandConnections* lands_within_1_move = get_l2l_conn(src_land);
  bool is_non_combat = is_non_combat_unit(unit_type);
#pragma unroll
  for (LandConnIndex conn_idx = 0; conn_idx < MAX_LAND_TO_LAND_CONNECTIONS; conn_idx++) {
    if (conn_idx >= lands_within_1_move_count) {
      break;
    }
    LandIndex dst_land = get_land_from_conn(lands_within_1_move, conn_idx);
    if (is_non_combat && !is_allied_0[*owner_idx[dst_land]]) {
      continue;
    }
    // 1. iterate through which territories have sent units within 2 moves of src_land
    // 2. during the iteration, build a set of territories that have been skipped
    // add_valid_land_move_if_history_allows_2(convert_land_to_air(dst_land), src_land);
    valid_actions[valid_actions_count++] = convert_land_to_air(dst_land);
  }
}

void add_valid_sea_moves_within_1(LandIndex src_land, LandUnitType unit_type) {
  if (is_non_loadable_unit(unit_type)) {
    return;
  }
  SeaIndex land_to_sea_count = get_l2s_count(src_land);
  SeaConnections* land_to_sea_conn = get_l2s_conn(src_land);
  bool is_heavy = is_heavy_unit(unit_type);
#pragma unroll
  for (int conn_idx = 0; conn_idx < MAX_LAND_TO_SEA_CONNECTIONS; conn_idx++) {
    if (conn_idx >= land_to_sea_count) {
      break;
    }
    SeaIndex dst_sea = get_sea_from_conn(land_to_sea_conn, conn_idx);
    if (transports_with_small_cargo_space[dst_sea] == 0) {
      continue;
    }
    if (is_heavy && transports_with_large_cargo_space[dst_sea] == 0) {
      continue;
    }
    valid_actions[valid_actions_count++] = convert_sea_to_air(dst_sea);
  }
}

void add_valid_land_moves(LandIndex src_land, Distance moves_remaining, LandUnitType unit_type) {
  if (moves_remaining == 2) {
    AirDistances* land_dist = get_land_to_air_dist(src_land);
    add_valid_land_moves_within_2(src_land, land_dist);
    add_valid_sea_moves_within_2(src_land, land_dist);
  } else {
    add_valid_land_moves_within_1(src_land, unit_type);
    add_valid_sea_moves_within_1(src_land, unit_type);
  }
}
void add_valid_sea_moves(SeaIndex src_sea, Distance moves_remaining) {
  if (moves_remaining == 2) {
    // check for moving from sea to sea (two moves away)
    SeaIndex seas_within_2_moves_count = get_seas_within_2_moves_count(canal_state, src_sea);
    SeaArray* seas_within_2_moves =
        get_seas_within_2_moves(canal_state, src_sea); // todo optimize canal_state
    bool* is_sea_path_blocked_src_sea = is_sea_path_blocked[src_sea];
#pragma unroll
    for (SeaConnIndex conn_idx = 0; conn_idx < SEAS_COUNT; conn_idx++) {
      if (conn_idx >= seas_within_2_moves_count) {
        break;
      }
      SeaIndex dst_sea = get_sea_from_array(seas_within_2_moves, conn_idx);
      if (is_sea_path_blocked_src_sea[dst_sea]) {
        continue;
      }
      // add_valid_sea_move_if_history_allows_2(dst_sea, src_sea);
      valid_actions[valid_actions_count++] = convert_sea_to_air(dst_sea);
    }
  } else {
    // check for moving from sea to sea (one move away)
    SeaIndex seas_within_1_move_count = get_seas_within_1_move_count(canal_state, src_sea);
    SeaArray* seas_within_1_move =
        get_seas_within_1_move(canal_state, src_sea); // todo optimize canal_state
#pragma unroll
    for (SeaConnIndex conn_idx = 0; conn_idx < MAX_SEA_TO_SEA_CONNECTIONS; conn_idx++) {
      if (conn_idx >= seas_within_1_move_count) {
        break;
      }
      SeaIndex dst_sea = get_sea_from_array(seas_within_1_move, conn_idx);
      // add_valid_sea_move_if_history_allows_1(dst_sea, src_sea);
      valid_actions[valid_actions_count++] = convert_sea_to_air(dst_sea);
    }
  }
}

void add_valid_sub_moves(SeaIndex src_sea, Distance moves_remaining) {
  if (moves_remaining == 2) {
    // check for moving from sea to sea (two moves away)
    SeaIndex seas_within_2_moves_count = get_seas_within_2_moves_count(canal_state, src_sea);
    SeaArray* seas_within_2_moves =
        get_seas_within_2_moves(canal_state, src_sea); // todo optimize canal_state
    bool* is_sub_path_blocked_src_sea = is_sub_path_blocked[src_sea];
#pragma unroll
    for (SeaConnIndex conn_idx = 0; conn_idx < SEAS_COUNT; conn_idx++) {
      if (conn_idx >= seas_within_2_moves_count) {
        break;
      }
      SeaIndex dst_sea = get_sea_from_array(seas_within_2_moves, conn_idx);
      if (is_sub_path_blocked_src_sea[dst_sea]) {
        continue;
      }
      // add_valid_sea_move_if_history_allows_2(dst_sea, src_sea);
      valid_actions[valid_actions_count++] = convert_sea_to_air(dst_sea);
    }
  } else {
    // check for moving from sea to sea (one move away)
    SeaIndex seas_within_1_move_count = get_seas_within_1_move_count(canal_state, src_sea);
    SeaArray* seas_within_1_move =
        get_seas_within_1_move(canal_state, src_sea); // todo optimize canal_state
#pragma unroll
    for (SeaConnIndex conn_idx = 0; conn_idx < MAX_SEA_TO_SEA_CONNECTIONS; conn_idx++) {
      if (conn_idx >= seas_within_1_move_count) {
        break;
      }
      SeaIndex dst_sea = get_sea_from_array(seas_within_1_move, conn_idx);
      // add_valid_sea_move_if_history_allows_1(dst_sea, src_sea);
      valid_actions[valid_actions_count++] = convert_sea_to_air(dst_sea);
    }
  }
}

bool stage_transport_units() {
  // loop through transports with "3" moves remaining (that aren't full),
  // start at sea 0 to n
  // todo: optimize with cache - only loop through regions with transports
  for (SeaUnitType unit_type = TRANS_EMPTY; unit_type <= TRANS_1T; unit_type++) {
    GenericSeaUnitState staging_state = STATES_MOVE_SEA[unit_type] - 1;
    GenericSeaUnitState done_staging = staging_state - 1;
    clear_move_history();
    for (SeaIndex src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      SeaUnitStateSum* total_ships = my_sea_unit_states[src_sea][unit_type][staging_state];
      if (*total_ships == 0) {
        continue;
      }
      AirIndex src_air = convert_sea_to_air(src_sea);
      valid_actions[0] = src_air;
      valid_actions_count = 1;
      add_valid_sea_moves(src_sea, 2);
      SeaUnitStateSums* sea_unit_states = &my_sea_unit_states[src_sea][unit_type];
#pragma unroll 4
      while (*total_ships > 0) {
        AirIndex dst_air = 0;
        if (valid_actions_count == 1) {
          dst_air = valid_actions[0];
        } else {
          if (answers_remaining == 0) {
            return true;
          }
#ifdef DEBUG
          dst_air = get_user_input_sea(unit_type, src_sea);
#else
          dst_air = getAIInput();
#endif
        }
#ifdef DEBUG
        printf("stage_transport_units: unit_type=%d src_air=%d dst_air=%d\n", unit_type, src_air,
               dst_air);
        setPrintableStatus();
        printf("%s\n", printableGameStatus);
#endif
        update_move_history(dst_air, src_sea);
        if (src_air == dst_air) {
          acc_sea_state_sums(sea_unit_states, done_staging, *total_ships);
          //(*units_sea_ptr_src_sea_unit_type)[done_staging] += *total_ships;
          *total_ships = 0;
          continue;
        }
        SeaIndex dst_sea = convert_air_to_sea(dst_air);
        Distance sea_distance =
            get_sea_dist(canal_state, src_sea, dst_air); // todo optimize canal_state
        if (enemy_blockade[dst_sea] > 0) {
          sea_state.flagged_for_combat[dst_air] = true;
          sea_distance = MAX_MOVE_SEA[unit_type];
          continue;
        }
        my_sea_unit_states[dst_sea][unit_type][done_staging - sea_distance]++;
        my_sea_unit_types[dst_sea][unit_type]++;
        player_navies[0][dst_sea]++;
        transports_with_small_cargo_space[dst_sea]++;
        (*sea_unit_states)[staging_state]--;
        my_sea_unit_types[src_sea][unit_type]--;
        player_navies[0][src_sea]--;
        transports_with_small_cargo_space[src_sea]--;
        if (unit_type <= TRANS_1I) {
          transports_with_large_cargo_space[src_sea]--;
          transports_with_large_cargo_space[dst_sea]++;
        }
      }
    }
  }
  return false;
}

inline void inc_sea_unit_state(SeaIndex sea_idx, SeaUnitType unit_type,
                               GenericSeaUnitState unit_state) {
  (*(my_sea_unit_states[sea_idx][unit_type]))[unit_state]++;
  my_sea_unit_types[sea_idx][unit_type]++;
}

void pre_move_fighter_units() {
#ifdef DEBUG
  setPrintableStatus();
  printf("%s\n", printableGameStatus);
  if (enemy_units_count[2] == MAX_UINT8_T) {
    printf("DEBUG: enemy_units_count[2] == -1\n");
  }
#endif
  clear_move_history();
  // refresh_canFighterLandHere
  for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    PlayerIndex land_owner = get_land_owner(land_idx);
    // is allied owned and not recently conquered?
    canFighterLandHere[land_idx] =
        (is_allied_0[land_owner] && !sea_state.flagged_for_combat[land_idx]);
    // check for possiblity to build carrier under fighter
    if (land_owner == sea_state.player_index && *factory_max[land_idx] > 0) {
      SeaIndex land_to_sea_count = get_l2s_count(land_idx);
      SeaConnections* land_to_sea_conn = get_l2s_conn(land_idx);
#pragma unroll
      for (int conn_idx = 0; conn_idx < MAX_LAND_TO_SEA_CONNECTIONS; conn_idx++) {
        if (conn_idx >= land_to_sea_count) {
          break;
        }
        canFighterLandHere[convert_sea_to_air(get_sea_from_conn(land_to_sea_conn, conn_idx))] =
            true;
      }
    }
  }
  for (SeaIndex sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    if (get_allied_carriers(sea_idx) > 0) {
      canFighterLandHere[sea_idx] = true;
      // if player owns these carriers, then landing area is 2 spaces away
      if (my_sea_unit_states[sea_idx][CARRIERS][CARRIER_MOVES_MAX] > 0) {
        SeaConnections* sea_to_sea_conn = get_s2s_conn(sea_idx);
        SeaIndex sea_to_sea_count = get_s2s_count(sea_idx);
#pragma unroll
        for (SeaConnIndex conn_idx = 0; conn_idx < MAX_SEA_TO_SEA_CONNECTIONS; conn_idx++) {
          if (conn_idx >= sea_to_sea_count) {
            break;
          }
          SeaIndex connected_sea1 = get_sea_from_conn(sea_to_sea_conn, conn_idx);
          canFighterLandHere[LANDS_COUNT + connected_sea1] = true;
          SeaIndex sea_to_sea_count2 = get_s2s_count(connected_sea1);
          SeaConnections* sea_to_sea_conn2 = get_s2s_conn(connected_sea1);
#pragma unroll
          for (SeaConnIndex conn2_idx = 0; conn2_idx < MAX_SEA_TO_SEA_CONNECTIONS; conn2_idx++) {
            if (conn2_idx >= sea_to_sea_count2) {
              break;
            }
            canFighterLandHere[convert_sea_to_air(get_sea_from_conn(sea_to_sea_conn2, conn2_idx))] =
                true;
          }
        }
      }
    }
  }
  // refresh_canFighterLandIn1Move
  for (int air_idx = 0; air_idx < AIRS_COUNT; air_idx++) {
    canFighterLandIn1Move[air_idx] = false;
    AirIndex air_conn_count = get_a2a_count(air_idx);
    AirConnection* air_conn = get_a2a_conn(air_idx);
#pragma unroll
    for (AirConnIndex conn_idx = 0; conn_idx < MAX_SEA_TO_SEA_CONNECTIONS; conn_idx++) {
      if (conn_idx >= air_conn_count) {
        break;
      }
      if (canFighterLandHere[get_air_from_a2a(air_conn, conn_idx)]) {
        canFighterLandIn1Move[air_idx] = true;
        break;
      }
    }
  }
}

inline PlayerIndex convert_state_player_offset(PlayerIndex player_idx) {
  return (player_idx + sea_state.player_index) % PLAYERS_COUNT;
}

void conquer_land(LandIndex dst_land) {
#ifdef DEBUG
  setPrintableStatus();
  printf("%s\n", printableGameStatus);
  printf("conquer_land: dst_land=%d\n", dst_land);
#endif
  PlayerIndex old_owner_id = *owner_idx[dst_land];
  if (get_player_capital(convert_state_player_offset(old_owner_id)) == dst_land) {
    sea_state.money[0] += sea_state.money[old_owner_id];
    sea_state.money[old_owner_id] = 0;
  }
  income_per_turn[old_owner_id] -= get_land_value(dst_land);
  PlayerIndex new_owner_id = 0;
  PlayerIndex orig_owner_id =
      (get_original_owner_index(dst_land) + PLAYERS_COUNT - sea_state.player_index) % PLAYERS_COUNT;
  if (is_allied(orig_owner_id)) {
    new_owner_id = orig_owner_id;
  }
#ifdef DEBUG
  printf("conquer_land: old_owner_id=%d new_owner_id=%d orig_owner_id=%d\n", old_owner_id,
         new_owner_id, orig_owner_id);
#endif
  *owner_idx[dst_land] = new_owner_id;
  acc_income_from_land(new_owner_id, dst_land);
  factory_locations[new_owner_id][factory_count[new_owner_id]++] = dst_land;
  factory_count[old_owner_id]--;
  for (FactoryLocIndex factory_idx = 0; factory_idx < LANDS_COUNT; factory_idx++) {
    if (factory_idx >= factory_count[orig_owner_id]) {
      break;
    }
    if (factory_locations[old_owner_id][factory_idx] == dst_land) {
#ifdef DEBUG
      printf("DEBUG: Found factory at %s\n", get_land_name(dst_land));
#endif
#pragma unroll
      for (FactoryLocIndex new_fact_idx = 0; new_fact_idx < LANDS_COUNT; new_fact_idx++) {
        if (new_fact_idx < factory_idx || new_fact_idx >= factory_count[old_owner_id]) {
          break;
        }
#ifdef DEBUG
        printf("DEBUG: Moving factory at %s\n",
               get_land_name(factory_locations[old_owner_id][new_fact_idx]));
#endif
        factory_locations[old_owner_id][new_fact_idx] =
            factory_locations[old_owner_id][new_fact_idx + 1];
      }
      break;
    }
  }
#ifdef DEBUG
  setPrintableStatus();
  printf("%s\n", printableGameStatus);
  for (PlayerIndex player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
#pragma unroll
    for (FactoryLocIndex factory_idx = 0; factory_idx < LANDS_COUNT; factory_idx++) {
      if (factory_idx >= factory_count[player_idx]) {
        break;
      }
      LandIndex factory_location = factory_locations[player_idx][factory_idx];
      if (*owner_idx[factory_location] != player_idx) {
        printf("DEBUG: Player %s has a unowned factory at %s\n",
               get_player_name(sea_state.player_index), get_land_name(factory_location));
      }
    }
  }
#endif
}

bool move_land_unit_type(LandUnitType unit_type) {
  clear_move_history();
  Distance max_moves = MAX_MOVE_LAND[unit_type];
  for (Distance moves_remaining2 = 0; moves_remaining2 < BOMBER_MOVES_MAX; moves_remaining2++) {
    if (moves_remaining2 >= max_moves) {
      break;
    }
    Distance moves_remaining = max_moves - moves_remaining2;
    for (LandIndex src_land = 0; src_land < LANDS_COUNT; src_land++) {
      LandUnitStateSum* total_units = my_land_unit_states[src_land][unit_type][moves_remaining];
      if (*total_units == 0) {
        continue;
      }
      AirDistances* land_dist = get_land_to_air_dist(src_land);
      valid_actions[0] = src_land;
      valid_actions_count = 1;
      add_valid_land_moves(src_land, moves_remaining, unit_type);
#pragma unroll
      while (*total_units > 0) {
        AirIndex dst_air = 0;
        if (valid_actions_count == 1) {
          dst_air = valid_actions[0];
        } else {
          if (answers_remaining == 0) {
            return true;
          }
#ifdef DEBUG
          dst_air = get_user_input_land(unit_type, src_land);
#else
          dst_air = getAIInput();
#endif
        }
#ifdef DEBUG
        setPrintableStatus();
        printf("%s\n", printableGameStatus);
        printf("DEBUG: player: %s moving land unit %d, src_air: %d, dst_air: %d enemy count: %d\n",
               get_player_name(sea_state.player_index), unit_type, src_land, dst_air,
               enemy_units_count[dst_air]);
#endif
        update_move_history(dst_air, src_land);
        if (src_land == dst_air) {
          *(my_land_unit_states[src_land][unit_type][0]) += *total_units;
          *total_units = 0;
          continue;
        }
        if (dst_air >= LANDS_COUNT) {
          load_transport(unit_type, src_land, dst_air - LANDS_COUNT, moves_remaining);
          // recalculate valid moves since transport cargo has changed
          valid_actions_count = 1;
          add_valid_land_moves(src_land, moves_remaining, unit_type);
          continue;
        }
        sea_state.flagged_for_combat[dst_air] = enemy_units_count[dst_air] > 0;
        // if the destination is not blitzable, then end unit turn
        Distance landDistance = get_air_distance(land_dist, dst_air);
        if (is_allied_0[*owner_idx[dst_air]] || enemy_units_count[dst_air] > 0) {
          landDistance = moves_remaining;
        }
        my_land_unit_states[dst_air][unit_type][moves_remaining - landDistance]++;
        my_land_unit_types[dst_air][unit_type]++;
        player_armies[0][dst_air]++;
        my_land_unit_types[src_land][unit_type]--;
        player_armies[0][src_land]--;
        *total_units -= 1;
        if (!is_allied_0[*owner_idx[dst_air]] && enemy_units_count[dst_air] == 0) {
#ifdef DEBUG
          printf("Conquering land");
#endif
          conquer_land(dst_air);
          sea_state.flagged_for_combat[dst_air] = true;
        }
#ifdef DEBUG
        debug_checks();
#endif
      }
    }
  }
  return false;
}

void skip_empty_transports() {
  for (SeaIndex src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
#pragma unroll
    for (TransEmptyState cur_state = TRANS_EMPTY_STATES - TRANS_EMPTY_STAGING_STATES;
         cur_state >= TRANS_EMPTY_UNLOADING_STATES + 1; cur_state--) {
      SeaUnitStateSum* total_ships = my_sea_unit_states[src_sea][TRANS_EMPTY][cur_state];
      if (*total_ships == 0) {
        continue;
      }
      my_sea_unit_states[src_sea][TRANS_EMPTY][0] += *total_ships;
      *total_ships = 0;
    }
  }
}

bool move_transport_units() {
  // loop through remaining Empty transports and set their moves remaining to 0. They had their
  // chance.
  skip_empty_transports();
  for (SeaUnitType unit_type = TRANS_1I; unit_type <= TRANS_1I_1T;
       unit_type++) { // there should be no TRANS_EMPTY
    SeaUnitStateCount max_state = STATES_MOVE_SEA[unit_type] - STATES_STAGING[unit_type];
    SeaUnitStateCount done_moving = STATES_UNLOADING[unit_type];
    SeaUnitStateCount min_state = STATES_UNLOADING[unit_type] + 1;
    clear_move_history();
    for (SeaIndex src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      for (GenericSeaUnitState cur_state = max_state; cur_state >= min_state; cur_state--) {
        SeaUnitStateSum* total_ships = my_sea_unit_states[src_sea][unit_type][cur_state];
        if (*total_ships == 0) {
          continue;
        }
        Distance moves_remaining = cur_state - STATES_UNLOADING[unit_type];
        AirIndex src_air = convert_sea_to_air(src_sea);
        valid_actions[0] = src_air;
        valid_actions_count = 1;
        add_valid_sea_moves(src_sea, moves_remaining);
#pragma unroll
        while (*total_ships > 0) {
          AirIndex dst_air = 0;
          if (valid_actions_count == 1) {
            dst_air = valid_actions[0];
          } else {
            if (answers_remaining == 0) {
              return true;
            }
#ifdef DEBUG
            dst_air = get_user_input_sea(unit_type, src_sea);
#else
            dst_air = getAIInput();
#endif
          }
#ifdef DEBUG
          printf("DEBUG: moving transport units unit_type: %d, src_air: %d, dst_air: %d\n",
                 unit_type, src_air, dst_air);
#endif
          update_move_history(dst_air, src_air);
          SeaIndex dst_sea = convert_air_to_sea(dst_air);
          if (enemy_blockade[dst_sea] > 0) {
#ifdef DEBUG
            printf("Enemy units detected, flagging for combat");
#endif
            flag_for_combat(dst_air);
          }
          if (src_air == dst_air) {
            my_sea_unit_states[src_sea][unit_type][done_moving] += *total_ships;
            *total_ships = 0;
            continue;
          }
          my_sea_unit_states[dst_sea][unit_type][done_moving]++;
          my_sea_unit_types[dst_sea][unit_type]++;
          player_navies[0][dst_sea]++;
          *total_ships -= 1;
          my_sea_unit_types[src_sea][unit_type]--;
          player_navies[0][src_sea]--;
          if (unit_type <= TRANS_1T) {
            transports_with_small_cargo_space[dst_sea]++;
            transports_with_small_cargo_space[src_sea]--;
            if (unit_type <= TRANS_1I) {
              transports_with_large_cargo_space[dst_sea]++;
              transports_with_large_cargo_space[src_sea]--;
            }
          }
        }
      }
    }
  }
  return false;
}
bool move_subs() {
  clear_move_history();
  for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    SeaUnitStateSum* total_subs = my_sea_unit_states[src_sea][SUBMARINES][SUB_UNMOVED];
    if (*total_subs == 0) {
      continue;
    }
    AirIndex src_air = convert_sea_to_air(src_sea);
    valid_actions[0] = src_air;
    valid_actions_count = 1;
    add_valid_sub_moves(src_sea, SUB_MOVES_MAX);
#pragma unroll
    while (*total_subs > 0) {
      AirIndex dst_air = 0;
      if (valid_actions_count == 1) {
        dst_air = valid_actions[0];
      } else {
        if (answers_remaining == 0) {
          return true;
        }
#ifdef DEBUG
        dst_air = get_user_input_sea(SUBMARINES, src_sea);
#else
        dst_air = getAIInput();
#endif
      }
#ifdef DEBUG
      setPrintableStatus();
      printf("%s\n", printableGameStatus);
      printf("DEBUG: moving sub units unit_type: %d, src_air: %d, dst_air: %d\n", SUBMARINES,
             src_air, dst_air);
#endif
      update_move_history(dst_air, src_air);
      SeaIndex dst_sea = convert_air_to_sea(dst_air);
      if (enemy_units_count[dst_sea] > 0) {
#ifdef DEBUG
        printf("Submarine moving to where enemy units are present, flagging for combat");
#endif
        flag_for_combat(convert_sea_to_air(dst_sea));
        // break;
      }
      if (src_air == dst_air) {
        my_sea_unit_states[src_sea][SUBMARINES][SUB_DONE_MOVING] += *total_subs;
        *total_subs = 0;
        continue;
      }
      my_sea_unit_states[dst_sea][SUBMARINES][SUB_DONE_MOVING]++;
      my_sea_unit_types[dst_sea][SUBMARINES]++;
      player_navies[0][dst_sea]++;
      *total_subs -= 1;
      my_sea_unit_types[src_sea][SUBMARINES]--;
      player_navies[0][src_sea]--;
    }
  }
  return false;
}

void carry_allied_fighters(SeaIndex src_sea, SeaIndex dst_sea) {
#ifdef DEBUG
  printf("DEBUG: carry_allied_fighters: src_sea: %d, dst_sea: %d\n", src_sea, dst_sea);
  setPrintableStatus();
  printf("%s\n", printableGameStatus);
#endif
  int allied_fighters_moved = 0;
  for (PlayerIndex other_player_idx = 1; other_player_idx < PLAYERS_COUNT; other_player_idx++) {
#pragma unroll
    while (player_sea_unit_types[other_player_idx][src_sea][FIGHTERS_SEA] > 0) {
      (*player_sea_unit_types[other_player_idx])[src_sea][FIGHTERS_SEA]--;
      player_navies[other_player_idx][src_sea]--;
      (*player_sea_unit_types[other_player_idx])[dst_sea][FIGHTERS_SEA]++;
      player_navies[other_player_idx][dst_sea]++;
      if (allied_fighters_moved == 1) {
        return;
      }
      allied_fighters_moved++;
    }
  }
}

bool move_destroyers_battleships() {
  for (int unit_type = DESTROYERS; unit_type <= BS_DAMAGED; unit_type++) {
    SeaUnitState unmoved = UNMOVED_SEA[unit_type];
    SeaUnitState done_moving = DONE_MOVING_SEA[unit_type];
    Distance moves_remaining = MAX_MOVE_SEA[unit_type];
    clear_move_history();
    for (SeaIndex src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      SeaUnitStateSum* total_ships = my_sea_unit_states[src_sea][unit_type][unmoved];
      if (*total_ships == 0) {
        continue;
      }
      AirIndex src_air = convert_sea_to_air(src_sea);
      valid_actions[0] = src_air;
      valid_actions_count = 1;
      add_valid_sea_moves(src_sea, moves_remaining);
#pragma unroll
      while (*total_ships > 0) {
        AirIndex dst_air = 0;
        if (valid_actions_count == 1) {
          dst_air = valid_actions[0];
        } else {
          if (answers_remaining == 0) {
            return true;
          }
#ifdef DEBUG
          dst_air = get_user_input_sea(unit_type, src_sea);
#else
          dst_air = getAIInput();
#endif
        }
#ifdef DEBUG
        setPrintableStatus();
        printf("%s\n", printableGameStatus);
        printf("DEBUG: moving ships units unit_type: %d, src_air: %d, dst_air: %d\n", unit_type,
               src_air, dst_air);
#endif
        update_move_history(dst_air, src_air);
        if (enemy_units_count[dst_air] > 0) {
#ifdef DEBUG
          printf("Moving large ships. Enemy units detected, flagging for combat");
#endif
          sea_state.flagged_for_combat[dst_air] = true;
          // break;
        }
        if (src_air == dst_air) {
          my_sea_unit_states[src_sea][unit_type][done_moving] += *total_ships;
          *total_ships = 0;
          continue;
        }
        SeaIndex dst_sea = convert_air_to_sea(dst_air);
        my_sea_unit_states[dst_sea][unit_type][done_moving]++;
        my_sea_unit_types[dst_sea][unit_type]++;
        player_navies[0][dst_sea]++;
        *total_ships -= 1;
        my_sea_unit_types[src_sea][unit_type]--;
        player_navies[0][src_sea]--;
        if (unit_type == CARRIERS) {
          carry_allied_fighters(src_sea, dst_sea);
        }
#ifdef DEBUG
        debug_checks();
#endif
      }
    }
  }
  return false;
}

void sea_retreat(SeaIndex src_sea, SeaIndex dst_sea) {
#ifdef DEBUG
  debug_checks();
  printf("DEBUG: retreating to sea: %d\n", dst_sea);
#endif
#pragma unroll
  for (SeaUnitType unit_type = TRANS_EMPTY; unit_type <= BS_DAMAGED; unit_type++) {
    *my_sea_unit_states[dst_sea][unit_type][0] += my_sea_unit_types[src_sea][unit_type];
    my_sea_unit_types[dst_sea][unit_type] += my_sea_unit_types[src_sea][unit_type];
    player_navies[0][dst_sea] += my_sea_unit_types[src_sea][unit_type];
    player_navies[0][src_sea] -= my_sea_unit_types[src_sea][unit_type];
    *my_sea_unit_states[src_sea][unit_type][STATES_UNLOADING[unit_type]] = 0;
    my_sea_unit_types[src_sea][unit_type] = 0;
  }
  unflag_for_combat(convert_sea_to_air(src_sea));
#ifdef DEBUG
  debug_checks();
#endif
}

Action ask_to_retreat() {
  if (is_player_human(sea_state.player_index)) {
    setPrintableStatus();
    strcat(printableGameStatus, "To where do you want to retreat (255 for no)? ");
    printf("%s\n", printableGameStatus);
    return getUserInput();
  }
  return getAIInput();
}
void remove_land_defenders(LandIndex src_land, HitPoints hits) {
  for (UnitTypeIndex unit_idx = 0; unit_idx < DEFENDER_LAND_UNIT_TYPES_COUNT; unit_idx++) {
#pragma unroll
    for (EnemyIndex enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
      PlayerIndex enemy_player_idx = enemies_0[enemy_idx];
      LandUnitSum* total_units =
          player_land_unit_types[enemy_player_idx][src_land][get_order_of_land_defenders(unit_idx)];
      if (*total_units > 0) {
#ifdef DEBUG
        printf(
            "DEBUG: remove_land_defenders: src_land: %d, hits: %d, unit_idx: %d, enemy_idx: %d\n",
            src_land, hits, get_order_of_land_defenders(unit_idx), enemy_idx);
#endif
        if (*total_units < hits) {
          hits -= *total_units;
          player_armies[enemy_player_idx][src_land] -= *total_units;
          enemy_units_count[src_land] -= *total_units;
          *total_units = 0;
        } else {
          *total_units -= hits;
          player_armies[enemy_player_idx][src_land] -= hits;
          enemy_units_count[src_land] -= hits;
          return;
        }
      }
    }
  }
}
void remove_land_attackers(LandIndex src_land, HitPoints hits) {
  LandUnitSum* total_units = 0;
#pragma unroll
  for (UnitTypeIndex unit_idx = 0; unit_idx < ATTACKER_LAND_UNIT_TYPES_COUNT_1; unit_idx++) {
    LandUnitType unit_type = get_order_of_land_attackers_1(unit_idx);
    // todo fix - why are AA guns with 1 move remaining here?
    total_units = my_land_unit_states[src_land][unit_type][0];
    if (*total_units > 0) {
#ifdef DEBUG
      printf("DEBUG: remove_land_attackers: unit_type: %d src_land: %d, hits: %d\n", unit_type,
             src_land, hits);
#endif
      if (*total_units < hits) {
        hits -= *total_units;
        player_armies[0][src_land] -= *total_units;
        *total_units = 0;
        my_land_unit_types[src_land][unit_type] = 0;
      } else {
        *total_units -= hits;
        my_land_unit_types[src_land][unit_type] -= hits;
        player_armies[0][src_land] -= hits;
        hits = 0;
        return;
      }
    }
  }
  for (UnitTypeIndex unit_idx = 0; unit_idx < ATTACKER_LAND_UNIT_TYPES_COUNT_2; unit_idx++) {
    LandUnitType unit_type = get_order_of_land_attackers_2(unit_idx);
    if (player_land_unit_types[0][src_land][unit_type] == 0) {
      continue;
    }
#pragma unroll
    for (GenericLandUnitState cur_state = 1; cur_state < STATES_MOVE_LAND[unit_type] - 1;
         cur_state++) {
      total_units = my_land_unit_states[src_land][unit_type][cur_state];
      if (*total_units > 0) {
#ifdef DEBUG
        printf("DEBUG: remove_land_attackers: unit_type: %d src_land: %d, hits: %d\n", unit_type,
               src_land, hits);
#endif

        if (*total_units < hits) {
          hits -= *total_units;
          my_land_unit_types[src_land][unit_type] -= *total_units;
          player_armies[0][src_land] -= *total_units;
          *total_units = 0;
        } else {
          *total_units -= hits;
          my_land_unit_types[src_land][unit_type] -= hits;
          player_armies[0][src_land] -= hits;
          hits = 0;
          return;
        }
      }
    }
  }
}
void remove_sea_defenders(SeaIndex src_sea, HitPoints hits, bool defender_submerged) {
#ifdef DEBUG
  printf("DEBUG: remove_sea_defenders: src_sea: %d, hits: %d\n", src_sea, hits);
#endif
  EnemyIndex enemy_idx = 0;
  AirIndex src_air = convert_sea_to_air(src_sea);
  PlayerIndex enemy_player_idx = 0;
#pragma unroll
  for (EnemyIndex enemy_idx = 0; enemy_idx < 3; enemy_idx++) {
    if (enemy_idx >= enemies_count_0) {
      break;
    }
    enemy_player_idx = enemies_0[enemy_idx];
    SeaUnitSum* total_battleships =
        &(*player_sea_unit_types[enemy_player_idx])[src_sea][BATTLESHIPS];
    SeaUnitSum* total_bs_damaged = &(*player_sea_unit_types[enemy_player_idx])[src_sea][BS_DAMAGED];
    if (*total_battleships > 0) {
#ifdef DEBUG
      printf("DEBUG: remove_sea_defenders: src_sea: %d, hits: %d, total_battleships: %d\n", src_sea,
             hits, *total_battleships);
#endif
      if (*total_battleships < hits) {
        hits -= *total_battleships;
        *total_bs_damaged += *total_battleships;
        *total_battleships = 0;
#ifdef DEBUG
        debug_checks();
#endif
      } else {
        *total_bs_damaged += hits;
        *total_battleships -= hits;
        hits = 0;
#ifdef DEBUG
        debug_checks();
#endif
        return;
      }
    }
  }
  if (!defender_submerged) {
#pragma unroll 3
    for (enemy_idx = 0; enemy_idx < 3; enemy_idx++) {
      if (enemy_idx >= enemies_count_0) {
        break;
      }
      enemy_player_idx = enemies_0[enemy_idx];
      SeaUnitSum* total_units = &(*player_sea_unit_types[enemy_player_idx])[src_sea][SUBMARINES];
      if (*total_units > 0) {
#ifdef DEBUG
        printf("DEBUG: remove_sea_defenders subs: src_sea: %d, hits: %d, total_units: %d\n",
               src_sea, hits, *total_units);
#endif
        if (*total_units < hits) {
          hits -= *total_units;
          player_navies[enemy_player_idx][src_sea] -= *total_units;
          enemy_units_count[src_air] -= *total_units;
          *total_units = 0;
#ifdef DEBUG
          debug_checks();
#endif
        } else {
          *total_units -= hits;
          player_navies[enemy_player_idx][src_sea] -= hits;
          enemy_units_count[src_air] -= hits;
          hits = 0;
#ifdef DEBUG
          debug_checks();
#endif
          return;
        }
      }
    }
  }

  // skipping submarines
  for (UnitTypeIndex unit_idx = 1; unit_idx < DEFENDER_SEA_UNIT_TYPES_COUNT; unit_idx++) {
#pragma unroll 3
    for (EnemyIndex enemy_idx = 0; enemy_idx < 3; enemy_idx++) {
      if (enemy_idx >= enemies_count_0) {
        break;
      }
      enemy_player_idx = enemies_0[enemy_idx];
      SeaUnitSum* total_units =
          player_sea_unit_types[enemy_player_idx][src_sea][get_order_of_sea_defenders(unit_idx)];
      if (*total_units > 0) {
#ifdef DEBUG
        printf(
            "DEBUG: remove_sea_defenders: unit_type: %d src_sea: %d, hits: %d, total_units: %d\n",
            get_order_of_sea_defenders(unit_idx), src_sea, hits, *total_units);
#endif
        if (*total_units < hits) {
          hits -= *total_units;
          player_navies[enemy_player_idx][src_sea] -= *total_units;
          enemy_units_count[src_air] -= *total_units;
          if (unit_idx >= DESTROYERS && unit_idx <= BS_DAMAGED) {
            enemy_blockade[src_sea] -= *total_units;
          }
          *total_units = 0;
#ifdef DEBUG
          debug_checks();
#endif
        } else {
          *total_units -= hits;
          player_navies[enemy_player_idx][src_sea] -= hits;
          enemy_units_count[src_air] -= hits;
          if (unit_idx >= DESTROYERS && unit_idx <= BS_DAMAGED) {
            enemy_blockade[src_sea] -= hits;
          }
          hits = 0;
#ifdef DEBUG
          debug_checks();
#endif
          return;
        }
      }
    }
  }
#ifdef DEBUG
  setPrintableStatus();
  printf("%s\n", printableGameStatus);
#endif
}

void remove_sea_attackers(SeaIndex src_sea, HitPoints hits) {
#ifdef DEBUG
  printf("DEBUG: remove_sea_attackers: src_sea: %d, hits: %d\n", src_sea, hits);
#endif
  SeaUnitSum* total_battleships = my_sea_unit_states[src_sea][BATTLESHIPS][0];
  SeaUnitSum* total_bs_damaged = my_sea_unit_states[src_sea][BS_DAMAGED][0];
  if (*total_battleships > 0) {
    if (*total_battleships < hits) {
#ifdef DEBUG
      printf("DEBUG: remove_sea_attackers: src_sea: %d, hits: %d, total_battleships: %d\n", src_sea,
             hits, *total_battleships);
#endif
      hits -= *total_battleships;
      *total_bs_damaged += *total_battleships;
      my_sea_unit_types[src_sea][BS_DAMAGED] += *total_battleships;
      *total_battleships = 0;
      my_sea_unit_types[src_sea][BATTLESHIPS] = 0;
    } else {
      *total_bs_damaged += hits;
      my_sea_unit_types[src_sea][BS_DAMAGED] += hits;
      *total_battleships -= hits;
      my_sea_unit_types[src_sea][BATTLESHIPS] -= hits;
      hits = 0;
      return;
    }
  }
  SeaUnitSum* total_units = 0;
#pragma unroll
  for (UnitTypeIndex unit_idx = 0; unit_idx < ATTACKER_SEA_UNIT_TYPES_COUNT_1; unit_idx++) {
    SeaUnitType unit_type = get_order_of_sea_attackers_1(unit_idx);
    SeaUnitSum* total_units = my_sea_unit_states[src_sea][unit_type][0];
    if (*total_units > 0) {
#ifdef DEBUG
      printf("DEBUG: remove_sea_attackers: unit_type: %d src_sea: %d, hits: %d\n", unit_type,
             src_sea, hits);
#endif
      if (*total_units < hits) {
        hits -= *total_units;
        player_navies[0][src_sea] -= *total_units;
        *total_units = 0;
        my_sea_unit_types[src_sea][unit_type] = 0;
      } else {
        *total_units -= hits;
        player_navies[0][src_sea] -= hits;
        my_sea_unit_types[src_sea][unit_type] -= hits;
        hits = 0;
        return;
      }
    }
  }
  total_units = my_sea_unit_states[src_sea][CARRIERS][0];
  if (*total_units > 0) {
    if (*total_units < hits) {
#ifdef DEBUG
      printf("DEBUG: remove_sea_attackers carriers: unit_type: %d src_sea: %d, hits: %d\n",
             CARRIERS, src_sea, hits);
#endif
      hits -= *total_units;
      player_navies[0][src_sea] -= *total_units;
      *total_units = 0;
      my_sea_unit_types[src_sea][CARRIERS] = 0;
      set_allied_carriers(src_sea, 0);
    } else {
      *total_units -= hits;
      my_sea_unit_types[src_sea][CARRIERS] -= hits;
      acc_allied_carriers(src_sea, -hits);
      player_navies[0][src_sea] -= hits;
      hits = 0;
      return; // todo: do allied fighters have a chance to land?
    }
  }
  total_units = my_sea_unit_states[src_sea][CRUISERS][0];
  if (*total_units > 0) {
    if (*total_units < hits) {
#ifdef DEBUG
      printf("DEBUG: remove_sea_attackers cruisers: unit_type: %d src_sea: %d, hits: %d\n",
             CRUISERS, src_sea, hits);
#endif
      hits -= *total_units;
      player_navies[0][src_sea] -= *total_units;
      *total_units = 0;
      my_sea_unit_types[src_sea][CRUISERS] = 0;
    } else {
      *total_units -= hits;
      my_sea_unit_types[src_sea][CRUISERS] -= hits;
      player_navies[0][src_sea] -= hits;
      hits = 0;
      return;
    }
  }
  for (UnitTypeIndex unit_idx = 0; unit_idx < ATTACKER_SEA_UNIT_TYPES_COUNT_2; unit_idx++) {
    SeaUnitType unit_type = get_order_of_sea_attackers_2(unit_idx);
    if (my_land_unit_types[src_sea][unit_type] == 0) {
      continue;
    }
    const SeaUnitStateCount states_move_sea = STATES_MOVE_SEA[unit_type];
#pragma unroll
    for (GenericSeaUnitState cur_state = 0; cur_state < BOMBER_SEA_STATES; cur_state++) {
      if (cur_state >= states_move_sea) {
        break;
      }
      total_units = my_sea_unit_states[src_sea][unit_type][cur_state];
      if (*total_units > 0) {
#ifdef DEBUG
        printf("DEBUG: remove_sea_attackers: unit_type: %d src_sea: %d, hits: %d\n", unit_type,
               src_sea, hits);
#endif
        if (*total_units < hits) {
          hits -= *total_units;
          player_navies[0][src_sea] -= *total_units;
          *total_units = 0;
          my_sea_unit_types[src_sea][unit_type] = 0;
        } else {
          *total_units -= hits;
          my_sea_unit_types[src_sea][unit_type] -= hits;
          player_navies[0][src_sea] -= hits;
          hits = 0;
          return;
        }
      }
    }
  }
#pragma unroll
  for (UnitTypeIndex unit_idx = 0; unit_idx < ATTACKER_SEA_UNIT_TYPES_COUNT_3; unit_idx++) {
    SeaUnitType unit_type = get_order_of_sea_attackers_3(unit_idx);
    SeaUnitSum* total_units = my_sea_unit_states[src_sea][unit_type][0];
    if (*total_units > 0) {
#ifdef DEBUG
      printf("DEBUG: remove_sea_attackers: unit_type: %d src_sea: %d, hits: %d\n", unit_type,
             src_sea, hits);
#endif
      if (*total_units < hits) {
        hits -= *total_units;
        player_navies[0][src_sea] -= *total_units;
        if (unit_type <= TRANS_1T) {
          transports_with_small_cargo_space[src_sea] -= *total_units;
          if (unit_type <= TRANS_1I) {
            transports_with_large_cargo_space[src_sea] -= *total_units;
          }
        }
        *total_units = 0;
        my_sea_unit_types[src_sea][unit_type] = 0;
      } else {
        *total_units -= hits;
        my_sea_unit_types[src_sea][unit_type] -= hits;
        player_navies[0][src_sea] -= hits;
        if (unit_type <= TRANS_1T) {
          transports_with_small_cargo_space[src_sea] -= hits;
          if (unit_type <= TRANS_1I) {
            transports_with_large_cargo_space[src_sea] -= hits;
          }
        }
        hits = 0;
        return;
      }
    }
  }
}

bool resolve_sea_battles() {
  for (SeaIndex src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    // check if battle is over (e.g. untargetable subs/air/trans or zero units)
    // target options:
    // 1. attacker has air, defender has std ships or air
    // 2. attacker has any ships, defender has any non-transports
    // 3. defender has air, attacker has std ships or air
    // 4. defender has any ships, attacker has any non-transports
    AirIndex src_air = convert_sea_to_air(src_sea);
    // if not flagged for combat, continue
    if (!sea_state.flagged_for_combat[src_air]) {
      continue;
    }
    if (player_navies[0][src_sea] == 0) {
      continue;
    }
#ifdef DEBUG
    setPrintableStatus();
    printf("%s\n", printableGameStatus);
    printf("DEBUG: resolve_sea_battles: src_sea: %d\n", src_sea);
#endif
    // does enemy only have submerged subs?
    bool defender_submerged = my_sea_unit_types[src_sea][DESTROYERS] == 0;
    if (defender_submerged) {
      SeaUnitSum total_enemy_subs = 0;
#pragma unroll
      for (EnemyIndex enemy_idx = 0; enemy_idx < 3; enemy_idx++) {
        if (enemy_idx >= enemies_count_0) {
          break;
        }
        total_enemy_subs += (*player_sea_unit_types[enemies_0[enemy_idx]])[src_sea][SUBMARINES];
      }
      if (total_enemy_subs == enemy_units_count[src_sea]) {
        continue;
      }
    }
    // combat is proceeding, so disable bombardment capabilities of ships
#pragma unroll
    for (SeaUnitType unit_type = CRUISERS; unit_type <= BS_DAMAGED; unit_type++) {
      *my_sea_unit_states[src_sea][unit_type][0] += *my_sea_unit_states[src_sea][unit_type][1];
      *my_sea_unit_states[src_sea][unit_type][1] = 0;
    }

    while (true) {
#ifdef DEBUG
      setPrintableStatus();
      printf("%s\n", printableGameStatus);
      if (my_land_unit_types[2][FIGHTERS_LAND] == MAX_UINT8_T) {
        printf("DEBUG: units_land_ptr[0][FIGHTERS][0] == 255\n");
      }
      if (enemy_units_count[2] == MAX_UINT8_T) {
        printf("DEBUG: enemy_units_count[2] == -1\n");
      }
#endif
      SeaUnitState* units11 = my_sea_unit_states[src_sea][DESTROYERS][0];
      bool targets_exist = false;
      EnemyIndex enemy_idx = 0;
      PlayerIndex enemy_player_idx = 0;
      if (my_sea_unit_types[src_sea][DESTROYERS]) {
        if (enemy_units_count[src_air] > 0) {
          targets_exist = true;
        }
      } else if (my_sea_unit_types[src_sea][CARRIERS] + my_sea_unit_types[src_sea][CRUISERS] +
                     my_sea_unit_types[src_sea][BATTLESHIPS] +
                     my_sea_unit_types[src_sea][BS_DAMAGED] >
                 0) {
        if (enemy_units_count[src_air] > 0) {
#pragma unroll 3
          for (enemy_idx = 0; enemy_idx < 3; enemy_idx++) {
            if (enemy_idx >= enemies_count_0) {
              break;
            }
            enemy_player_idx = enemies_0[enemy_idx];
            if (player_navies[enemy_player_idx][src_sea] -
                    (*player_sea_unit_types[enemy_player_idx])[src_sea][SUBMARINES] >
                0) {
              targets_exist = true;
              break;
            }
          }
        }
      } else if (my_sea_unit_types[src_sea][SUBMARINES] > 0) { // no blockade ships, only subs
#pragma unroll 3
        for (enemy_idx = 0; enemy_idx < 3; enemy_idx++) {
          if (enemy_idx >= enemies_count_0) {
            break;
          }
          enemy_player_idx = enemies_0[enemy_idx];
          if (player_navies[enemy_player_idx][src_sea] -
                  ((*player_sea_unit_types[enemy_player_idx])[src_sea][FIGHTERS_SEA] +
                   (*player_sea_unit_types[enemy_player_idx])[src_sea][SUBMARINES]) >
              0) {
            targets_exist = true;
            break;
          }
        }
      } else if (my_sea_unit_types[src_sea][FIGHTERS_SEA] +
                     my_sea_unit_types[src_sea][BOMBERS_SEA] >
                 0) { // no ships, only air
#pragma unroll 3
        for (enemy_idx = 0; enemy_idx < 3; enemy_idx++) {
          if (enemy_idx >= enemies_count_0) {
            break;
          }
          enemy_player_idx = enemies_0[enemy_idx];
          if (player_navies[enemy_player_idx][src_sea] -
                  (*player_sea_unit_types[enemy_player_idx])[src_sea][SUBMARINES] >
              0) {
            targets_exist = true;
            break;
          }
        }
      }
      // todo only allow hits to hit valid targets
      // untargetable battle
      if (!targets_exist) {
        // allied_carriers[src_sea] =
        //     data.units_sea[src_sea].carriers[0] + data.units_sea[src_sea].carriers[1];
        // for (int player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
        //   allied_carriers[src_sea] +=
        //       other_sea_units_ptr[player_idx][src_sea][CARRIERS] * is_allied_0[player_idx];
        // }
        unflag_for_combat(src_air);
        break;
      }
      // fire subs (defender always submerges if possible)
      Power attacker_damage = *my_sea_unit_states[src_sea][SUBMARINES][0] * SUB_ATTACK; // todo FIX
      // int attacker_damage = current_player_sea_unit_types[src_sea][SUBMARINES] * SUB_ATTACK;
      // //TODO FIX
      HitPoints attacker_hits =
          (attacker_damage / DICE_SIDES) +
          (get_random_number(random_number_index++) % DICE_SIDES < attacker_damage % DICE_SIDES
               ? 1
               : 0);

      Power defender_damage = 0;
      HitPoints defender_hits = 0;
      if (!defender_submerged) {
        defender_damage = 0; // todo skip if no subs
#pragma unroll
        for (enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
          defender_damage += (*player_sea_unit_types[enemies_0[enemy_idx]])[src_sea][SUBMARINES];
        }
        defender_hits =
            (defender_damage / DICE_SIDES) +
            (get_random_number(random_number_index++) % DICE_SIDES < defender_damage % DICE_SIDES
                 ? 1
                 : 0);
        if (defender_hits > 0) {
          remove_sea_attackers(src_sea, defender_hits);
        }
      }
      // remove casualties
      if (attacker_hits > 0) {
        remove_sea_defenders(src_sea, attacker_hits, defender_submerged);
      }
      // fire all ships and air for both sides
      attacker_damage = 0;
#pragma unroll
      for (UnitTypeIndex unit_type_idx = 0; unit_type_idx < BLOCKADE_UNIT_TYPES_COUNT;
           unit_type_idx++) {
        SeaUnitType unit_type = BLOCKADE_UNIT_TYPES[unit_type_idx];
        attacker_damage += my_sea_unit_types[src_sea][unit_type] * ATTACK_UNIT_SEA[unit_type];
      }
      attacker_damage += my_sea_unit_types[src_sea][FIGHTERS_SEA] * ATTACK_UNIT_SEA[FIGHTERS_SEA];
      attacker_damage += my_sea_unit_types[src_sea][BOMBERS_SEA] * ATTACK_UNIT_SEA[BOMBERS_SEA];
      attacker_hits =
          (attacker_damage / DICE_SIDES) +
          (get_random_number(random_number_index++) % DICE_SIDES < attacker_damage % DICE_SIDES
               ? 1
               : 0);

      defender_damage = 0;
      for (enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
        SeaUnitSumArray* enemy_units = &(*player_sea_unit_types[enemies_0[enemy_idx]])[src_sea];
#pragma unroll
        for (SeaUnitType unit_type = 0; unit_type < BLOCKADE_UNIT_TYPES_COUNT; unit_type++) {
          defender_damage += *enemy_units[unit_type] * DEFENSE_UNIT_SEA[unit_type];
        }
        defender_damage += *enemy_units[FIGHTERS_SEA] * DEFENSE_UNIT_SEA[FIGHTERS_SEA];
      }
      defender_hits =
          (defender_damage / DICE_SIDES) +
          (get_random_number(random_number_index++) % DICE_SIDES < defender_damage % DICE_SIDES
               ? 1
               : 0);
      // remove casualties
      if (defender_hits > 0) {
        remove_sea_attackers(src_sea, defender_hits);
      }
      if (attacker_hits > 0) {
        remove_sea_defenders(src_sea, attacker_hits, defender_submerged);
      }

      if (enemy_units_count[src_air] == 0 || player_navies[0][src_sea] == 0) {
        unflag_for_combat(src_air);
        break;
      }

      if (my_sea_unit_types[src_sea][FIGHTERS_SEA] + my_sea_unit_types[src_sea][BOMBERS_SEA] +
                  my_sea_unit_types[src_sea][SUBMARINES] + my_sea_unit_types[src_sea][DESTROYERS] +
                  my_sea_unit_types[src_sea][CARRIERS] + my_sea_unit_types[src_sea][CRUISERS] +
                  my_sea_unit_types[src_sea][BATTLESHIPS] + my_sea_unit_types[src_sea][BS_DAMAGED] >
              0 ||
          enemy_blockade[src_sea] == 0) {
        valid_actions[0] = src_air;
        valid_actions_count = 1;
      } else {
        valid_actions_count = 0;
      }
      SeaConnections* sea_to_sea_conn = get_s2s_conn(src_sea);
      SeaConnIndex s2s_count = get_s2s_count(src_sea);
#pragma unroll
      for (SeaConnIndex sea_conn_idx = 0; sea_conn_idx < MAX_SEA_TO_SEA_CONNECTIONS;
           sea_conn_idx++) {
        if (sea_conn_idx >= s2s_count) {
          continue;
        }
        SeaIndex sea_dst = get_sea_from_conn(sea_to_sea_conn, sea_conn_idx);
        if (enemy_blockade[sea_dst] == 0) {
          valid_actions[valid_actions_count++] = convert_sea_to_air(sea_dst);
        }
      }
      if (valid_actions_count == 0) {
        continue;
      }
      AirIndex dst_air = 0;
      if (valid_actions_count == 1) {
        dst_air = valid_actions[0];
      } else {
        if (answers_remaining == 0) {
          return true;
        }
#ifdef DEBUG
        dst_air = ask_to_retreat();
#else
        dst_air = getAIInput();
#endif
      }
      // if retreat, move units to retreat zone immediately and end battle
      SeaIndex dst_sea = convert_air_to_sea(dst_air);
      if (get_sea_dist(canal_state, src_sea, dst_sea) == 1 && !is_flagged_for_combat(dst_air)) {
        sea_retreat(src_sea, dst_sea);
        break;
      }
      // loop
    }
  }
  return false;
}

void add_valid_unload_moves(SeaIndex src_sea) {
  LandConnections* near_land = get_s2l_conn(src_sea);
  LandIndex near_land_count = get_s2l_count(src_sea);
#pragma unroll
  for (LandConnIndex conn_idx = 0; conn_idx < MAX_SEA_TO_LAND_CONNECTIONS; conn_idx++) {
    if (conn_idx >= near_land_count) {
      continue;
    }
    LandIndex dst_land = get_land_from_conn(near_land, conn_idx);
    add_valid_air_move_if_history_allows_X(dst_land, convert_sea_to_air(src_sea), 1);
  }
}

void add_valid_fighter_moves(AirIndex src_air, Distance remaining_moves) {
  AirIndexArray* near_air = get_airs_within_x_moves(remaining_moves - 1, src_air);
  AirIndex near_air_count = get_airs_within_x_moves_count(remaining_moves - 1, src_air);
#pragma unroll
  for (AirConnIndex conn_idx = 0; conn_idx < AIRS_COUNT; conn_idx++) {
    if (conn_idx >= near_air_count) {
      continue;
    }
    AirIndex dst_air = get_air_from_array(near_air, conn_idx);
    Distance air_dist = get_air_dist(src_air, dst_air);
    if (air_dist <= 2 || canFighterLandHere[dst_air] ||
        (air_dist == 3 && canFighterLandIn1Move[dst_air])) {
      if (!canFighterLandHere[dst_air] && enemy_units_count[dst_air] == 0) // waste of a move
      {
        continue;
      }
      add_valid_air_move_if_history_allows_X(dst_air, src_air, air_dist);
    }
  }
}

void add_valid_fighter_landing(AirIndex src_air, Distance remaining_moves) {
  AirIndexArray* near_air = get_airs_within_x_moves(remaining_moves - 1, src_air);
  AirIndex near_air_count = get_airs_within_x_moves_count(remaining_moves - 1, src_air);
#pragma unroll
  for (AirConnIndex conn_idx = 0; conn_idx < AIRS_COUNT; conn_idx++) {
    if (conn_idx >= near_air_count) {
      continue;
    }
    AirIndex dst_air = get_air_from_array(near_air, conn_idx);
    if (canFighterLandHere[dst_air]) {
      add_valid_air_move_if_history_allows_X(dst_air, src_air, remaining_moves);
    }
  }
}

void add_valid_bomber_moves(AirIndex src_air, Distance remaining_moves) {
  AirIndexArray* near_air = get_airs_within_x_moves(remaining_moves - 1, src_air);
  AirIndex near_air_count = get_airs_within_x_moves_count(remaining_moves - 1, src_air);
#pragma unroll
  for (AirConnIndex conn_idx = 0; conn_idx < near_air_count; conn_idx++) {
    AirIndex dst_air = get_air_from_array(near_air, conn_idx);
    Distance air_dist = get_air_dist(src_air, dst_air);
    if (air_dist <= 3 || canBomberLandHere[dst_air] ||
        (air_dist == 4 && canBomberLandIn2Moves[dst_air]) ||
        (air_dist == 5 && canBomberLandIn1Move[dst_air])) {
      if (!canBomberLandHere[dst_air] && enemy_units_count[dst_air] == 0) {
        if (dst_air >= LANDS_COUNT || *factory_max[dst_air] == 0 ||
            *factory_hp[dst_air] == -*factory_max[dst_air]) // waste of a move
          continue;
      }
      add_valid_air_move_if_history_allows_X(dst_air, src_air, air_dist);
    }
  }
}
bool unload_transports() {
  for (SeaUnitType unit_type = TRANS_1I; unit_type <= TRANS_1I_1T; unit_type++) {
    SeaUnitStateCount unloading_state = STATES_UNLOADING[unit_type];
    LandUnitType unload_cargo1 = UNLOAD_CARGO1[unit_type];
    LandUnitType unload_cargo2 = UNLOAD_CARGO2[unit_type];
    clear_move_history();
    for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      SeaUnitSum* total_units = my_sea_unit_states[src_sea][unit_type][unloading_state];
      if (*total_units == 0) {
        continue;
      }
      AirIndex src_air = convert_sea_to_air(src_sea);
      valid_actions[0] = src_air;
      valid_actions_count = 1;
      add_valid_unload_moves(src_sea);
      while (*total_units > 0) {
        AirIndex dst_air = 0;
        if (valid_actions_count == 1) {
          dst_air = valid_actions[0];
        } else {
          if (answers_remaining == 0) {
            return true;
          }
#ifdef DEBUG
          dst_air = get_user_input_sea(unit_type, src_sea);
#else
          dst_air = getAIInput();
#endif
        }
#ifdef DEBUG
        printf("DEBUG: unload_transports unit_type: %d, src_sea: %d, dst_air: %d, unload_cargo: "
               "%d, %d\n",
               unit_type, src_sea, dst_air, unload_cargo1, unload_cargo2);
        setPrintableStatus();
        printf("%s\n", printableGameStatus);
#endif
        update_move_history(dst_air, src_sea);
        if (src_air == dst_air) {
          my_sea_unit_states[src_sea][unit_type][0] += *total_units;
          *total_units = 0;
          continue;
        }
        bombard_max[dst_air]++;
        my_land_unit_states[dst_air][unload_cargo1][0]++;
        my_land_unit_types[dst_air][unload_cargo1]++;
        player_armies[0][dst_air]++;
        my_sea_unit_states[src_sea][TRANS_EMPTY][0]++;
        my_sea_unit_types[src_sea][TRANS_EMPTY]++;
        my_sea_unit_types[src_sea][unit_type]--;
        *total_units -= 1;
        if (unit_type > TRANS_1T) {
          bombard_max[dst_air]++;
          my_land_unit_states[dst_air][unload_cargo2][0]++;
          my_land_unit_types[dst_air][unload_cargo2]++;
          player_armies[0][dst_air]++;
        }
        if (!is_allied_0[*owner_idx[dst_air]]) {
          sea_state.flagged_for_combat[dst_air] = true;
          if (enemy_units_count[dst_air] == 0) {
            conquer_land(dst_air);
          }
        }
#ifdef DEBUG
        debug_checks();
#endif
      }
    }
  }
  return false;
}
bool resolve_land_battles() {
  for (LandIndex src_land = 0; src_land < LANDS_COUNT; src_land++) {
    // check if battle is over
    if (is_flagged_for_combat(convert_land_to_air(src_land))) {
      continue;
    }
#ifdef DEBUG
    // debug print the current src_land and its name
    printf("Resolve land combat in: %d, Name: %s\n", src_land, get_land_name(src_land));
    setPrintableStatus();
    printf("%s\n", printableGameStatus);
#endif
    ArmySum* units_land_player_total_0_src_land = &player_armies[0][src_land];
    // check if no friendlies remain
    if (*units_land_player_total_0_src_land == 0) {
#ifdef DEBUG
      printf("No friendlies remain");
#endif
      continue;
    }
    Power attacker_damage = 0;
    HitPoints attacker_hits = 0;
    // only bombers exist
    LandUnitSum* other_land_units_ptr_0_src_land = my_land_unit_types[src_land];
    LandUnitSum* bombers_count = &other_land_units_ptr_0_src_land[BOMBERS_LAND];
    if (*bombers_count > 0 && player_armies[0][src_land] == *bombers_count) {
      if (*factory_hp[src_land] > -*factory_max[src_land]) {
#ifdef DEBUG
        printf("Strategic Bombing");
#endif
        // fire_strat_aa_guns();
        Power defender_damage = *bombers_count;
        HitPoints defender_hits =
            (defender_damage / DICE_SIDES) +
            (get_random_number(random_number_index++) % DICE_SIDES < defender_damage % DICE_SIDES
                 ? 1
                 : 0);
        if (defender_hits > 0) {
          LandUnitStateSums* units_land_ptr_src_land_bombers =
              &my_land_unit_states[src_land][BOMBERS_LAND];
#pragma unroll
          for (BomberLandState cur_state = 1; cur_state < BOMBER_LAND_STATES - 1; cur_state++) {
            LandUnitSum* total_units = *units_land_ptr_src_land_bombers[cur_state];
            if (*total_units < defender_hits) {
              defender_hits -= *total_units;
              *bombers_count -= *total_units;
              *units_land_player_total_0_src_land -= *total_units;
              *total_units = 0;
            } else {
              *total_units -= defender_hits;
              *bombers_count -= defender_hits;
              *units_land_player_total_0_src_land -= defender_hits;
              defender_hits = 0;
              continue;
            }
          }
        }
        attacker_damage = *bombers_count * ((DICE_SIDES + 1) * DICE_SIDES / 2);
        attacker_hits =
            (attacker_damage / DICE_SIDES) +
            (get_random_number(random_number_index++) % DICE_SIDES < attacker_damage % DICE_SIDES
                 ? 1
                 : 0);
        HitPoints remaining_hp = (*factory_hp[src_land] - attacker_hits);
        HitPoints min_hp = (-(*factory_max[src_land]));
        *factory_hp[src_land] = (remaining_hp > min_hp) ? remaining_hp : min_hp;
        continue;
      }
    }
#ifdef DEBUG
    printf("Normal Land Combat");
#endif
    LandUnitSum* other_land_units_0_src_land = my_land_unit_types[src_land];
    // bombard_shores
    if (bombard_max[src_land] > 0) {
      attacker_damage = 0;
#ifdef DEBUG
      printf("Sea Bombardment");
#endif
      for (SeaUnitType unit_type = BS_DAMAGED; unit_type >= CRUISERS; unit_type--) {
        SeaConnections* land_to_sea_conn = get_l2s_conn(src_land);
        for (SeaConnIndex conn_idx = 0; conn_idx < get_l2s_count(src_land); conn_idx++) {
          SeaIndex src_sea = get_sea_from_conn(land_to_sea_conn, conn_idx);
          SeaUnitStateSums* total_bombard_ships = &my_sea_unit_states[src_sea][unit_type];
#pragma unroll
          while (total_bombard_ships[1] > 0 && bombard_max[src_land] > 0) {
            attacker_damage += ATTACK_UNIT_SEA[unit_type];
            (*total_bombard_ships)[0]++;
            (*total_bombard_ships)[1]--;
            bombard_max[src_land]--;
          }
        }
      }
      bombard_max[src_land] = 0;
      attacker_hits =
          (attacker_damage / DICE_SIDES) +
          (get_random_number(random_number_index++) % DICE_SIDES < attacker_damage % DICE_SIDES
               ? 1
               : 0);
      if (attacker_hits > 0) {
        remove_land_defenders(src_land, attacker_hits);
      }
    }
    // check if can fire tactical aa_guns
    ArmySum total_air_units = other_land_units_ptr_0_src_land[FIGHTERS_LAND] +
                              other_land_units_ptr_0_src_land[BOMBERS_LAND];
    Power defender_damage = 0;
    HitPoints defender_hits = 0;
    EnemyIndex enemy_idx = 0;
    if (total_air_units > 0) {
      LandUnitSum total_aa_guns = 0;
#pragma unroll 3
      for (enemy_idx = 0; enemy_idx < 3; enemy_idx++) {
        if (enemy_idx >= enemies_count_0) {
          break;
        }
        total_aa_guns += (*player_land_unit_types[enemies_0[enemy_idx]])[src_land][AA_GUNS];
      }
      if (total_aa_guns > 0) {
        // fire_tact_aa_guns();
        defender_damage = total_air_units * 3;
        defender_hits =
            (defender_damage / DICE_SIDES) +
            (get_random_number(random_number_index++) % DICE_SIDES < defender_damage % DICE_SIDES
                 ? 1
                 : 0);
        if (defender_hits > 0) {
#pragma unroll
          for (FighterState cur_state = 0; cur_state < FIGHTER_STATES; cur_state++) {
            LandUnitStateSum* total_units = my_land_unit_states[src_land][FIGHTERS_LAND][cur_state];
            if (*total_units < defender_hits) {
              defender_hits -= *total_units;
              other_land_units_ptr_0_src_land[FIGHTERS_LAND] -= *total_units;
              *units_land_player_total_0_src_land -= *total_units;
              *total_units = 0;
            } else {
              *total_units -= defender_hits;
              other_land_units_ptr_0_src_land[FIGHTERS_LAND] -= defender_hits;
              *units_land_player_total_0_src_land -= defender_hits;
              defender_hits = 0;
              break;
            }
          }
        }
        if (defender_hits > 0) {
#pragma unroll
          for (BomberLandState cur_state = 0; cur_state < BOMBER_LAND_STATES; cur_state++) {
            LandUnitStateSum* total_units = my_land_unit_states[src_land][BOMBERS_LAND][cur_state];
            if (*total_units < defender_hits) {
              defender_hits -= *total_units;
              other_land_units_ptr_0_src_land[BOMBERS_LAND] -= *total_units;
              *units_land_player_total_0_src_land -= *total_units;
              *total_units = 0;
            } else {
              *total_units -= defender_hits;
              other_land_units_ptr_0_src_land[BOMBERS_LAND] -= defender_hits;
              *units_land_player_total_0_src_land -= defender_hits;
              defender_hits = 0;
              break;
            }
          }
        }
#ifdef DEBUG
        debug_checks();
#endif
      }
    }
    if (*units_land_player_total_0_src_land == 0) {
#ifdef DEBUG
      printf("No friendlies remain");
#endif
      continue;
    }
    if (enemy_units_count[src_land] == 0) {
      // if infantry, artillery, tanks exist then capture
      if (other_land_units_0_src_land[INFANTRY] + other_land_units_0_src_land[ARTILLERY] +
              other_land_units_0_src_land[TANKS] >
          0) {
        conquer_land(src_land);
      }
      continue;
    }
    while (true) {
      // print land location name
#ifdef DEBUG
      printf("Current land battle start src_land: %d, Name: %s", src_land, get_land_name(src_land));
      setPrintableStatus();
      printf("%s\n", printableGameStatus);
#endif
      if (*units_land_player_total_0_src_land == 0) {
#ifdef DEBUG
        printf("No friendlies remain");
#endif
        break;
      }
      // land_battle
      LandUnitSum infantry_count = my_land_unit_types[src_land][INFANTRY];
      LandUnitSum artillery_count = my_land_unit_types[src_land][ARTILLERY];
      attacker_damage = (my_land_unit_types[src_land][FIGHTERS_LAND] * FIGHTER_ATTACK) +
                        (my_land_unit_types[src_land][BOMBERS_LAND] * BOMBER_ATTACK) +
                        (infantry_count * INFANTRY_ATTACK) + (artillery_count * ARTILLERY_ATTACK) +
                        (my_land_unit_types[src_land][TANKS] * TANK_ATTACK);
      // add damage for the minimum of count of infantry/artillery
      attacker_damage += infantry_count < artillery_count ? infantry_count : artillery_count;
      attacker_hits =
          (attacker_damage / DICE_SIDES) +
          (get_random_number(random_number_index++) % DICE_SIDES < attacker_damage % DICE_SIDES
               ? 1
               : 0);
      defender_damage = 0;
#ifdef DEBUG
      printf("Enemy Count: %d\n", enemy_units_count[src_land]);
#endif
#pragma unroll 3
      for (enemy_idx = 0; enemy_idx < 3; enemy_idx++) {
        if (enemy_idx >= enemies_count_0) {
          break;
        }
        LandUnitSumArray* land_units = player_land_unit_types[enemies_0[enemy_idx]][src_land];
        defender_damage += ((*land_units)[INFANTRY] * INFANTRY_DEFENSE) +
                           ((*land_units)[ARTILLERY] * ARTILLERY_DEFENSE) +
                           ((*land_units)[TANKS] * TANK_DEFENSE) +
                           ((*land_units)[FIGHTERS_LAND] * FIGHTER_DEFENSE) +
                           ((*land_units)[BOMBERS_LAND] * BOMBER_DEFENSE);
        defender_hits =
            (defender_damage / DICE_SIDES) +
            (get_random_number(random_number_index++) % DICE_SIDES < defender_damage % DICE_SIDES
                 ? 1
                 : 0);
      }
      if (defender_hits > 0) {
#ifdef DEBUG
        printf("Defender Hits: %d", defender_hits);
#endif
        remove_land_attackers(src_land, defender_hits);
#ifdef DEBUG
        debug_checks();
#endif
      }
      if (attacker_hits > 0) {
#ifdef DEBUG
        printf("Attacker Hits: %d", attacker_hits);
#endif
        remove_land_defenders(src_land, attacker_hits);
#ifdef DEBUG
        debug_checks();
#endif
      }

      if (*units_land_player_total_0_src_land == 0) {
#ifdef DEBUG
        printf("No friendlies remain");
#endif
        break;
      }
      if (enemy_units_count[src_land] == 0) {
        // if infantry, artillery, tanks exist then capture
        if (other_land_units_0_src_land[INFANTRY] + other_land_units_0_src_land[ARTILLERY] +
                other_land_units_0_src_land[TANKS] >
            0) {
          conquer_land(src_land);
        }
        break;
      }

      // ask to retreat (0-255, any non valid retreat zone is considered a no)
      valid_actions[0] = src_land;
      valid_actions_count = 1;
      LandConnections* land_to_land_conn = get_l2l_conn(src_land);
      LandIndex land_to_land_count = get_l2l_count(src_land);
#pragma unroll
      for (LandConnIndex conn_idx = 0; conn_idx < MAX_LAND_TO_LAND_CONNECTIONS; conn_idx++) {
        if (conn_idx >= land_to_land_count) {
          break;
        }
        LandIndex land_dst = get_land_from_conn(land_to_land_conn, conn_idx);
        if (enemy_units_count[land_dst] == 0 && !sea_state.flagged_for_combat[land_dst] &&
            is_allied(get_land_owner(land_dst))) {
          valid_actions[valid_actions_count++] = land_dst;
        }
      }
      AirIndex dst_air = 0;
      if (valid_actions_count == 1) {
        dst_air = valid_actions[0];
      } else {
        if (answers_remaining == 0) {
          return true;
        }
#ifdef DEBUG
        dst_air = ask_to_retreat();
#else
        dst_air = getAIInput();
#endif
      }
      // if retreat, move units to retreat zone immediately and end battle
      if (src_land != dst_air) {
#ifdef DEBUG
        printf("Retreating land_battle from: %d to: %d\n", src_land, dst_air);
#endif
#pragma unroll
        for (LandUnitType unit_type = INFANTRY; unit_type <= TANKS; unit_type++) {
          LandUnitStateSum total_units = *my_land_unit_states[src_land][unit_type][0];
          my_land_unit_states[dst_air][unit_type][0] += total_units;
          my_land_unit_types[dst_air][unit_type] += total_units;
          player_armies[0][dst_air] += total_units;
          my_land_unit_types[src_land][unit_type] -= total_units;
          *units_land_player_total_0_src_land -= total_units;
          my_land_unit_states[src_land][unit_type][0] = 0;
        }
        unflag_for_combat(convert_land_to_air(src_land));
#ifdef DEBUG
        debug_checks();
#endif
        break;
      }
    }
  }
  return false;
}

bool move_fighter_units() {
  pre_move_fighter_units();
  for (int src_air = 0; src_air < AIRS_COUNT; src_air++) {
    LandUnitStateSum* total_fighters = 0;
    if (src_air < LANDS_COUNT) { // todo fix optimize
      total_fighters = my_land_unit_states[src_air][FIGHTERS_LAND][FIGHTER_MOVES_MAX];
    } else {
      total_fighters = my_sea_unit_states[src_air - LANDS_COUNT][FIGHTERS_SEA][FIGHTER_MOVES_MAX];
    }
    if (*total_fighters == 0) {
      continue;
    }
    valid_actions[0] = src_air;
    valid_actions_count = 1;
    add_valid_fighter_moves(src_air, FIGHTER_MOVES_MAX);
#pragma unroll
    while (*total_fighters > 0) {
      AirIndex dst_air = 0;
      if (valid_actions_count == 1) {
        dst_air = valid_actions[0];
      } else {
        if (answers_remaining == 0) {
          return true;
        }
#ifdef DEBUG
        if (src_air < LANDS_COUNT) {
          dst_air = get_user_input_land(FIGHTERS_LAND, convert_air_to_land(src_air));
        } else {
          dst_air = get_user_input_sea(FIGHTERS_SEA, convert_air_to_sea(src_air));
        }
#else
        dst_air = getAIInput();
#endif
      }
#ifdef DEBUG
      setPrintableStatus();
      printf("%s\n", printableGameStatus);
      printf("DEBUG: player: %s moving fighters %d, src_air: %d, dst_air: %d\n",
             get_player_name(sea_state.player_index), FIGHTERS_AIR, src_air, dst_air);
#endif
      update_move_history(dst_air, src_air);
      if (src_air == dst_air) {
        if (src_air < LANDS_COUNT) {
          my_land_unit_states[src_air][FIGHTERS_LAND][0] += *total_fighters;
        } else {
          my_sea_unit_states[src_air - LANDS_COUNT][FIGHTERS_SEA][0] += *total_fighters;
        }
        *total_fighters = 0;
        continue;
      }
      Distance airDistance = get_air_dist(src_air, dst_air);
      if (dst_air < LANDS_COUNT) {
        if (!is_allied_0[*owner_idx[dst_air]]) {
#ifdef DEBUG
          printf("Fighter moving to enemy territory. Automatically flagging for combat");
#endif
          flag_for_combat(dst_air); // assuming enemy units are present based on valid moves
        } else {
          airDistance = 4; // use up all moves if this is a friendly rebase
        }
      } else {
#ifdef DEBUG
        printf("Fighter moving to sea. Possibly flagging for combat");
#endif
        sea_state.flagged_for_combat[dst_air] = enemy_units_count[dst_air] > 0;
      }
      if (dst_air < LANDS_COUNT) {
        my_land_unit_states[dst_air][FIGHTERS_LAND][FIGHTER_MOVES_MAX - airDistance]++;
        my_land_unit_types[dst_air][FIGHTERS_LAND]++;
        player_armies[0][dst_air]++;
      } else {
        SeaIndex dst_sea = convert_air_to_sea(dst_air);
        my_sea_unit_states[dst_sea][FIGHTERS_SEA][FIGHTER_MOVES_MAX - airDistance]++;
        my_sea_unit_types[dst_sea][FIGHTERS_SEA]++;
        player_navies[0][dst_sea]++;
      }
      if (src_air < LANDS_COUNT) {
        my_land_unit_types[src_air][FIGHTERS_LAND]--;
        player_armies[0][src_air]--;
      } else {
        SeaIndex src_sea = convert_air_to_sea(src_air);
        my_sea_unit_types[src_sea][FIGHTERS_LAND]--;
        player_navies[0][src_sea]--;
      }
      *total_fighters -= 1;
    }
  }
  return false;
}

bool move_bomber_units() {
  // check if any bombers have full moves remaining
  clear_move_history();
#pragma unroll
  for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    // is allied owned and not recently conquered?
    canBomberLandHere[land_idx] =
        (is_allied_0[*owner_idx[land_idx]] && !sea_state.flagged_for_combat[land_idx]);
  }
  //  refresh_canBomberLandIn1Move
  for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    canBomberLandIn1Move[land_idx] = false;
    LandIndex land_conn_count = get_l2l_count(land_idx);
    LandConnections* land_to_land_conn = get_l2l_conn(land_idx);
#pragma unroll
    for (LandConnIndex conn_idx = 0; conn_idx < MAX_LAND_TO_LAND_CONNECTIONS; conn_idx++) {
      if (conn_idx >= land_conn_count) {
        break;
      }
      if (canBomberLandHere[get_land_from_conn(land_to_land_conn, conn_idx)]) {
        canBomberLandIn1Move[land_idx] = true;
        break;
      }
    }
  }
  for (SeaIndex sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    canBomberLandIn1Move[LANDS_COUNT + sea_idx] = false;
    LandIndex land_conn_count = get_s2l_count(sea_idx);
    LandConnections* sea_to_land_conn = get_s2l_conn(sea_idx);
#pragma unroll
    for (LandConnIndex conn_idx = 0; conn_idx < MAX_SEA_TO_LAND_CONNECTIONS; conn_idx++) {
      if (conn_idx >= land_conn_count) {
        break;
      }
      if (canBomberLandHere[get_land_from_conn(sea_to_land_conn, conn_idx)]) {
        canBomberLandIn1Move[LANDS_COUNT + sea_idx] = true;
        break;
      }
    }
  }
  // refresh_canBomberLandIn2Moves
  for (AirIndex air_idx = 0; air_idx < AIRS_COUNT; air_idx++) {
    canBomberLandIn2Moves[air_idx] = false;
    AirIndex air_conn_count = get_a2a_count(air_idx);
    AirConnection* air_conn = get_a2a_conn(air_idx);
#pragma unroll
    for (int conn_idx = 0; conn_idx < MAX_AIR_TO_AIR_CONNECTIONS; conn_idx++) {
      if (conn_idx >= air_conn_count) {
        break;
      }
      if (canBomberLandIn1Move[(*air_conn)[conn_idx]]) {
        canBomberLandIn2Moves[air_idx] = true;
        break;
      }
    }
  }
  for (LandIndex src_land = 0; src_land < LANDS_COUNT; src_land++) {
    LandUnitStateSum* total_bombers = my_land_unit_states[src_land][BOMBERS_LAND][BOMBER_MOVES_MAX];
    if (*total_bombers == 0) {
      continue;
    }
    valid_actions[0] = src_land;
    valid_actions_count = 1;
    add_valid_bomber_moves(src_land, BOMBER_MOVES_MAX);
#pragma unroll
    while (*total_bombers > 0) {
      AirIndex dst_air = 0;
      if (valid_actions_count == 1) {
        dst_air = valid_actions[0];
      } else {
        if (answers_remaining == 0) {
          return true;
        }
#ifdef DEBUG
        dst_air = get_user_input_land(BOMBERS_LAND, src_land);
#else
        dst_air = getAIInput();
#endif
      }
#ifdef DEBUG
      printf("DEBUG: player: %s bombers fighters %d, src_air: %d, dst_air: %d\n",
             get_player_name(sea_state.player_index), BOMBERS_LAND, src_land, dst_air);
#endif
      update_move_history(dst_air, src_land);
      if (src_land == dst_air) {
        *(my_land_unit_states[src_land][BOMBERS_LAND][0]) += *total_bombers;
        *total_bombers = 0;
        continue;
      }
      if (dst_air < LANDS_COUNT) {
        if (!is_allied_0[*owner_idx[dst_air]]) {
#ifdef DEBUG
          printf("Bomber moving to enemy territory. Automatically flagging for combat");
#endif
          sea_state.flagged_for_combat[dst_air] =
              true; // assuming enemy units are present based on valid moves
        }
      } else {
#ifdef DEBUG
        printf("Bomber moving to sea. Possibly flagging for combat");
#endif
        SeaIndex dst_sea = convert_air_to_sea(dst_air);
        sea_state.flagged_for_combat[dst_air] = enemy_units_count[dst_air] > 0;
      }
      Distance airDistance = get_air_dist(src_land, dst_air);
      if (dst_air < LANDS_COUNT) {
        my_land_unit_states[dst_air][BOMBERS_LAND][BOMBER_MOVES_MAX - airDistance]++;
        (*(player_land_unit_types[0][dst_air]))[BOMBERS_LAND]++;
        player_armies[0][dst_air]++;
      } else {
        SeaIndex dst_sea = convert_air_to_sea(dst_air);
        my_sea_unit_states[dst_sea][BOMBERS_SEA][BOMBER_MOVES_MAX - 1 - airDistance]++;
        my_sea_unit_types[dst_sea][BOMBERS_SEA]++;
        player_navies[0][dst_sea]++;
      }
      my_land_unit_types[src_land][BOMBERS_LAND]--;
      player_armies[0][src_land]--;
      *total_bombers -= 1;
    }
  }
  return false;
}

bool land_fighter_units() {
  //  refresh_canFighterLandHere_final
#pragma unroll
  for (SeaIndex sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    // if (allied_carriers[sea_idx] > 0) {
    canFighterLandHere[convert_sea_to_air(sea_idx)] = allied_carriers[sea_idx] > 0;
    //}
  }
  for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    PlayerIndex land_owner = get_land_owner(land_idx);
    // is allied owned and not recently conquered?
    canFighterLandHere[land_idx] =
        is_allied_0[land_owner] && !sea_state.flagged_for_combat[land_idx];
    // check for possiblity to build carrier under fighter
    if (*factory_max[land_idx] > 0 && land_owner == sea_state.player_index) {
      SeaConnections* land_to_sea_conn = get_l2s_conn(land_idx);
      SeaIndex land_to_sea_count = get_l2l_count(land_idx);
#pragma unroll
      for (SeaConnIndex conn_idx = 0; conn_idx < MAX_LAND_TO_SEA_CONNECTIONS; conn_idx++) {
        if (conn_idx >= land_to_sea_count) {
          break;
        }
        canFighterLandHere[convert_sea_to_air(get_sea_from_conn(land_to_sea_conn, conn_idx))] =
            true;
      }
    }
  }
  //  check if any fighters have moves remaining
  for (FighterState cur_state = 1; cur_state < FIGHTER_STATES - 1;
       cur_state++) { // todo optimize to find next fighter faster
    clear_move_history();
    for (AirIndex src_air = 0; src_air < AIRS_COUNT; src_air++) {
      AirUnitStateSums* fighter_state_sums = get_air_unit_state_sums(src_air, FIGHTERS_AIR);
      AirUnitStateSum* total_fighter_sum = get_air_unit_state_sum_at(fighter_state_sums, cur_state);
      if (*total_fighter_sum == 0) {
        continue;
      }

      // valid_moves[0] = src_air;
      valid_actions_count = 0;
      add_valid_fighter_landing(src_air, cur_state);
      if (valid_actions_count == 0 || canFighterLandHere[src_air]) {
        valid_actions[valid_actions_count++] = src_air;
      }
#pragma unroll
      while (*total_fighter_sum > 0) {
        AirIndex dst_air = 0;
        if (valid_actions_count == 1) {
          dst_air = valid_actions[0];
        } else {
          if (answers_remaining == 0) {
            return true;
          }
#ifdef DEBUG
          if (src_air < LANDS_COUNT) {
            dst_air = get_user_input_land(FIGHTERS_LAND, convert_air_to_land(src_air));
          } else {
            dst_air = get_user_input_sea(FIGHTERS_SEA, convert_air_to_sea(src_air));
          }
#else
          dst_air = getAIInput();
#endif
        }
#ifdef DEBUG
        setPrintableStatus();
        printf("%s\n", printableGameStatus);
        printf("DEBUG: player: %s landing fighters %d, src_air: %d, dst_air: %d\n",
               get_player_name(sea_state.player_index), FIGHTERS_AIR, src_air, dst_air);
#endif
        update_move_history(dst_air, src_air);
        if (src_air == dst_air) {
          (*fighter_state_sums)[0]++;
          *total_fighter_sum -= 1;
          continue;
        }
        (*fighter_state_sums)[0]++;
        if (dst_air < LANDS_COUNT) {
          player_armies[0][dst_air]++;
          my_land_unit_types[dst_air][FIGHTERS_LAND]++;
        } else {
          SeaIndex dst_sea = convert_air_to_sea(dst_air);
          player_navies[0][dst_sea]++;
          my_sea_unit_types[dst_sea][FIGHTERS_SEA]++;
        }
        if (src_air < LANDS_COUNT) {
          player_armies[0][src_air]--;
          my_land_unit_types[src_air][FIGHTERS_LAND]--;
        } else {
          SeaIndex src_sea = convert_air_to_sea(src_air);
          player_navies[0][src_sea]--;
          my_sea_unit_types[src_sea][FIGHTERS_SEA]--;
        }
        *total_fighter_sum -= 1;
#ifdef DEBUG
        debug_checks();
#endif
      }
    }
  }
  return false;
}

void add_valid_bomber_landing(AirIndex src_air, Distance movement_remaining) {
#ifdef DEBUG
  printf("movement_remaining: %d\n", movement_remaining);
#endif
  LandArray* near_land = get_air_to_land_within_x_moves(movement_remaining - 1, src_air);
#pragma unroll
  for (LandIndex conn_idx = 0;
       conn_idx < get_air_to_land_within_x_moves_count(movement_remaining - 1, src_air);
       conn_idx++) {
    AirIndex dst_air = get_land_from_array(near_land, conn_idx);
    if (canBomberLandHere[dst_air]) {
#ifdef DEBUG
      printf("Adding valid move: %d\n", dst_air);
#endif
      valid_actions[valid_actions_count++] = dst_air;
    }
  }
}

bool land_bomber_units() {
#ifdef DEBUG
  printf("Landing Bombers");
#endif
#pragma unroll
  for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    // is allied owned and not recently conquered?
    canBomberLandHere[land_idx] = is_allied(get_land_owner(land_idx)) &&
                                  !is_flagged_for_combat(convert_land_to_air(land_idx));
  }
  // check if any bombers have moves remaining
  for (BomberLandState cur_state1 = 0; cur_state1 < BOMBER_LAND_STATES - 2;
       cur_state1++) { // todo optimize to find next bomber faster
    clear_move_history();
    for (AirIndex src_air = 0; src_air < AIRS_COUNT; src_air++) {
      BomberLandState cur_state = (src_air < LANDS_COUNT ? cur_state1 + 1 : cur_state1);
      AirUnitStateSums* bomber_state_sums = get_air_unit_state_sums(src_air, BOMBERS_AIR);
      AirUnitStateSum* total_bomber_sum = get_bomber_state_sum_at(bomber_state_sums, cur_state);
      if (*total_bomber_sum == 0) {
        continue;
      }
#ifdef DEBUG
      printf("Bomber Count: %d with state %d, in location %d\n", *total_bomber_sum, cur_state,
             src_air);
#endif
      // valid_moves[0] = src_air;
      valid_actions_count = 0;
      Distance movement_remaining = cur_state + (src_air < LANDS_COUNT ? 0 : 1);
      add_valid_bomber_landing(src_air, movement_remaining);
      while (*total_bomber_sum > 0) {
        if (valid_actions_count == 0) {
          valid_actions[valid_actions_count++] = src_air;
        }
        AirIndex dst_air = 0;
        if (valid_actions_count == 1) {
          dst_air = valid_actions[0];
        } else {
          if (answers_remaining == 0) {
            return true;
          }
#ifdef DEBUG
          if (src_air < LANDS_COUNT) {
            dst_air = get_user_input_land(BOMBERS_LAND, convert_air_to_land(src_air));
          } else {
            dst_air = get_user_input_sea(BOMBERS_SEA, convert_air_to_sea(src_air));
          }
#else
          dst_air = getAIInput();
#endif
        }
#ifdef DEBUG
        setPrintableStatus();
        printf("%s\n", printableGameStatus);
        printf("valid_moves_count: %d\n", valid_actions_count);
        printf("valid_moves: ");
#pragma unroll
        for (int i = 0; i < AIRS_COUNT; i++) {
          if (i >= valid_actions_count) {
            break;
          }
          printf("%d ", valid_actions[i]);
        }
        printf("\n");
        printf("DEBUG: player: %s landing bombers %d, src_air: %d, dst_air: %d\n",
               get_player_name(sea_state.player_index), BOMBERS_AIR, src_air, dst_air);
#endif
        update_move_history(dst_air, src_air);
        if (src_air == dst_air) {
          (*bomber_state_sums)[0]++;
          *total_bomber_sum -= 1;
#ifdef DEBUG
          debug_checks();
#endif
          continue;
        }
        (*bomber_state_sums)[0]++;
        player_armies[0][dst_air]++;
        my_land_unit_types[dst_air][BOMBERS_LAND]++;
        if (src_air < LANDS_COUNT) {
          player_armies[0][src_air]--;
          my_land_unit_types[src_air][BOMBERS_LAND]--;
        } else {
          SeaIndex src_sea = convert_air_to_sea(src_air);
          player_navies[0][src_sea]--;
          my_land_unit_types[src_sea][BOMBERS_LAND]--;
        }
        *total_bomber_sum -= 1;
#ifdef DEBUG
        debug_checks();
#endif
      }
#ifdef DEBUG
      debug_checks();
#endif
    }
  }
  return false;
}

bool buy_units() {
#ifdef DEBUG
  setPrintableStatus();
  printf("%s\nBuying Units\n", printableGameStatus);
#endif
  for (FactoryLocIndex factory_idx = 0; factory_idx < factory_count[0]; factory_idx++) {
    LandIndex dst_land = factory_locations[0][factory_idx];
#ifdef DEBUG
    if (get_land_owner(dst_land) != 0) {
      printf("DEBUG: player: %s cannot buy units at %s\n", get_player_name(sea_state.player_index),
             get_land_name(dst_land));
    }
#endif
    if (sea_state.builds_left[dst_land] == 0) {
      continue;
    }
    Dollars repair_cost = 0;
    // buy sea units
    SeaConnections* near_sea = get_l2s_conn(dst_land);
    SeaConnIndex l2s_count = get_l2s_count(dst_land);
    for (SeaConnIndex conn_idx = 0; conn_idx < MAX_LAND_TO_SEA_CONNECTIONS; conn_idx++) {
      if (conn_idx >= l2s_count) {
        break;
      }
      SeaIndex dst_sea = get_sea_from_conn(near_sea, conn_idx);
      AirIndex dst_air = convert_sea_to_air(dst_sea);
      valid_actions[0] = SEA_UNIT_TYPES_COUNT; // pass all units
      Action last_purchased = 0;
      while (sea_state.builds_left[dst_air] > 0) {
        if (get_money(0) < TRANSPORT_COST) {
          set_builds_left(dst_air, 0);
          sea_state.builds_left[dst_air] = 0;
          break;
        }
        ArmySum units_built = *factory_max[dst_land] - sea_state.builds_left[dst_land];
        if (*factory_hp[dst_land] <= units_built) {
          repair_cost = 1 + units_built - *factory_hp[dst_land];
        } // subtracting a negative
        // add all units that can be bought
        valid_actions_count = 1;
        for (UnitTypeIndex unit_type_idx = COST_UNIT_SEA_COUNT - 1; unit_type_idx >= 0;
             unit_type_idx--) {
          SeaUnitType unit_type = BUY_UNIT_SEA[unit_type_idx];
          if (unit_type < last_purchased) {
            break;
          }
          if (sea_state.money[0] < COST_UNIT_SEA[unit_type] + repair_cost) {
            continue;
          }
          if (unit_type == FIGHTERS_SEA) {
            SeaUnitSum total_fighters = 0;
#pragma unroll
            for (PlayerIndex player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
              total_fighters += (*player_sea_unit_types[player_idx])[dst_sea][FIGHTERS_SEA];
            }
            if (allied_carriers[dst_sea] * 2 <= total_fighters) {
              continue;
            }
          }
          valid_actions[valid_actions_count++] = unit_type;
        }
        if (valid_actions_count == 1) {
          sea_state.builds_left[dst_air] = 0;
          break;
        }
        Action purchase = 0;
        if (answers_remaining == 0) {
          return true;
        }
#ifdef DEBUG
        purchase = get_user_purchase_input(dst_air);
#else
        purchase = getAIInput();
#endif
        if (purchase == SEA_UNIT_TYPES_COUNT) { // pass all units
          sea_state.builds_left[dst_air] = 0;
          break;
        }
#ifdef DEBUG
        // print which player is buying which unit at which location
        printf("Player %d buying %s at %s\n", sea_state.player_index, NAMES_UNIT_SEA[purchase],
               get_sea_name(dst_sea));
#endif
        SeaConnections* land_to_sea_conn = get_l2s_conn(dst_land);
        SeaIndex land_to_sea_count = get_l2s_count(dst_land);
#pragma unroll
        for (SeaConnIndex conn_idx2 = conn_idx; conn_idx2 < MAX_LAND_TO_SEA_CONNECTIONS;
             conn_idx2++) {
          if (conn_idx2 >= land_to_sea_count) {
            break;
          }
          sea_state
              .builds_left[convert_sea_to_air(get_sea_from_conn(land_to_sea_conn, conn_idx2))]--;
        }
        sea_state.builds_left[dst_land]--;
        *factory_hp[dst_land] += repair_cost;
        sea_state.money[0] -= COST_UNIT_SEA[purchase] + repair_cost;
        my_sea_unit_states[dst_sea][purchase][0]++;
        player_navies[0][dst_sea]++;
        my_sea_unit_types[dst_sea][purchase]++;
        last_purchased = purchase;
      }
    }
    // buy land units
    valid_actions[0] = LAND_UNIT_TYPES_COUNT; // pass all units
    LandUnitType unit_type = 0;
    Action last_purchased = 0;
    while (sea_state.builds_left[dst_land] > 0) {
      if (sea_state.money[0] < INFANTRY_COST) {
        sea_state.builds_left[dst_land] = 0;
        break;
      }
      ArmySum units_built = *factory_max[dst_land] - sea_state.builds_left[dst_land];
      if (*factory_hp[dst_land] <= units_built) {
        repair_cost = 1 + units_built - *factory_hp[dst_land];
      } // subtracting a negative
      // add all units that can be bought
      valid_actions_count = 1;
#pragma unroll
      for (LandUnitType unit_type = LAND_UNIT_TYPES_COUNT - 1; unit_type >= 0; unit_type--) {
        if (unit_type < last_purchased) {
          break;
        }
        if (sea_state.money[0] < COST_UNIT_LAND[unit_type] + repair_cost) {
          continue;
        }
        valid_actions[valid_actions_count++] = unit_type;
      }
      if (valid_actions_count == 1) {
        sea_state.builds_left[dst_land] = 0;
        break;
      }
      if (answers_remaining == 0) {
        return true;
      }
      Action purchase = 0;
#ifdef DEBUG
      purchase = get_user_purchase_input(convert_land_to_air(dst_land));
#else
      purchase = getAIInput();
#endif
      if (purchase == LAND_UNIT_TYPES_COUNT) { // pass all units
        sea_state.builds_left[dst_land] = 0;
        break;
      }
#ifdef DEBUG
      // print which player is buying which unit at which location
      printf("Player %d buying %s at %s\n", sea_state.player_index, NAMES_UNIT_LAND[purchase],
             get_land_name(dst_land));
#endif
      sea_state.builds_left[dst_land]--;
      *factory_hp[dst_land] += repair_cost;
      sea_state.money[0] -= COST_UNIT_LAND[purchase] + repair_cost;
      my_land_unit_states[dst_land][purchase][0]++;
      player_armies[0][dst_land]++;
      my_land_unit_types[dst_land][purchase]++;
      last_purchased = purchase;
    }
  }
#ifdef DEBUG
  setPrintableStatus();
  printf("%s\n", printableGameStatus);
#endif
  return false;
}

void crash_air_units() {
  // crash planes not on friendly land
  for (AirIndex air_idx = 0; air_idx < LANDS_COUNT; air_idx++) {
    if (canFighterLandHere[air_idx]) {
      continue;
    }
    if (my_land_unit_types[air_idx][FIGHTERS_LAND] == 0) {
      continue;
    }
#ifdef DEBUG
    printf("DEBUG: Crashing fighters at %d\n", air_idx);
#endif
    player_armies[0][air_idx] -= my_land_unit_types[air_idx][FIGHTERS_LAND];
    my_land_unit_types[air_idx][FIGHTERS_LAND] = 0;
    my_air_unit_states[air_idx][FIGHTERS_AIR][0] = 0;
  }
  // crash planes not on allied carriers
  for (AirIndex air_idx = LANDS_COUNT; air_idx < AIRS_COUNT; air_idx++) {
    // units_air_ptr[air_idx][BOMBERS_LAND_AIR][0] = 0;
    SeaIndex sea_idx = convert_air_to_sea(air_idx);
    CarrierSpace free_space = allied_carriers[sea_idx] * 2;
    for (PlayerIndex player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      free_space -= (*player_sea_unit_types[player_idx])[sea_idx][FIGHTERS_SEA];
    }
    AirUnitStateSum* total_fighter_sum = my_air_unit_states[air_idx][FIGHTERS_AIR][0];
    if (free_space < *total_fighter_sum) {
      SeaUnitSum fighters_lost = *total_fighter_sum - free_space;
#ifdef DEBUG
      printf("DEBUG: Crashing %d fighters at %d\n", fighters_lost, air_idx);
#endif
      player_navies[0][sea_idx] -= fighters_lost;
      my_sea_unit_types[sea_idx][FIGHTERS_SEA] -= fighters_lost;
      *total_fighter_sum = free_space;
    }
  }
}
void reset_units_fully() {
// reset battleship health
#pragma unroll
  for (SeaIndex sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    my_sea_unit_states[sea_idx][BATTLESHIPS][0] += my_sea_unit_types[sea_idx][BS_DAMAGED];
    my_sea_unit_types[sea_idx][BATTLESHIPS] += my_sea_unit_types[sea_idx][BS_DAMAGED];
    my_sea_unit_states[sea_idx][BS_DAMAGED][0] = 0;
    my_sea_unit_states[sea_idx][BS_DAMAGED][1] = 0;
    my_sea_unit_types[sea_idx][BS_DAMAGED] = 0;
  }
}
// todo BUY FACTORY
void buy_factory() {}
void collect_money() {
  // if player still owns their capital, collect income
  sea_state.money[0] +=
      (income_per_turn[0] * (get_land_owner(get_player_capital(sea_state.player_index)) == 0));
}
void rotate_turns() {
  // rotate units
#ifdef DEBUG
  setPrintableStatus();
  printf("%s\n", printableGameStatus);
  printf("DEBUG: Rotating turns\n");
#pragma unroll
  for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    if (player_armies[0][land_idx] > 0 && enemy_units_count[land_idx] > 0) {
      printf("DEBUG: Player %s has %d units at %s\n", get_player_name(sea_state.player_index),
             player_armies[0][land_idx], get_land_name(land_idx));
    }
  }
#pragma unroll
  for (SeaIndex sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    if (player_navies[0][sea_idx] > 0 && enemy_units_count[sea_idx + LANDS_COUNT] > 0) {
      printf("DEBUG: Player %s has %d units at %s\n", get_player_name(sea_state.player_index),
             player_navies[0][sea_idx], get_sea_name(sea_idx));
    }
  }
#endif
  memcpy(&temp_land_unit_types, &my_land_unit_types, sizeof(sea_state.other_land_units[0]));
  memcpy(&my_land_unit_types, &sea_state.other_land_units[0],
         sizeof(sea_state.other_land_units[0]));
  memmove(&sea_state.other_land_units[0], &sea_state.other_land_units[1],
          sizeof(sea_state.other_land_units[0]) * (PLAYERS_COUNT - 2));
  memcpy(&sea_state.other_land_units[PLAYERS_COUNT - 2], &temp_land_unit_types,
         sizeof(sea_state.other_land_units[0]));
#pragma unroll
  for (LandIndex dst_land = 0; dst_land < LANDS_COUNT; dst_land++) {
    LandTerr* land0 = &sea_state.land_terr[dst_land];
    LandUnitSumArray* land1 = &my_land_unit_types[dst_land];
    // todo optimize restructuring
    memset(&sea_state.land_terr[dst_land].fighters, 0, sizeof(sea_state.land_terr[0].fighters));
    land0->fighters[FIGHTER_STATES - 1] = (*land1)[FIGHTERS_LAND];
    memset(&sea_state.land_terr[dst_land].bombers, 0, sizeof(sea_state.land_terr[0].bombers));
    land0->bombers[BOMBER_LAND_STATES - 1] = (*land1)[BOMBERS_LAND];
    memset(&sea_state.land_terr[dst_land].infantry, 0, sizeof(sea_state.land_terr[0].infantry));
    land0->infantry[INFANTRY_STATES - 1] = (*land1)[INFANTRY];
    memset(&sea_state.land_terr[dst_land].artillery, 0, sizeof(sea_state.land_terr[0].artillery));
    land0->artillery[ARTILLERY_STATES - 1] = (*land1)[ARTILLERY];
    memset(&sea_state.land_terr[dst_land].tanks, 0, sizeof(sea_state.land_terr[0].tanks));
    land0->tanks[TANK_STATES - 1] = (*land1)[TANKS];
    memset(&sea_state.land_terr[dst_land].aa_guns, 0, sizeof(sea_state.land_terr[0].aa_guns));
    land0->aa_guns[AA_GUN_STATES - 1] = (*land1)[AA_GUNS];
  }
  memcpy(&temp_sea_unit_types, &my_sea_unit_types, sizeof(temp_sea_unit_types));
  //  memcpy(&other_sea_units_0, &data.other_sea_units[0], OTHER_SEA_UNITS_SIZE);
#pragma unroll
  for (SeaIndex dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
    memcpy(&my_sea_unit_types[dst_sea], &sea_state.other_sea_units[0][dst_sea],
           SEA_UNIT_TYPES_COUNT - 1);
  }
  memmove(&sea_state.other_sea_units[0], &sea_state.other_sea_units[1],
          sizeof(sea_state.other_sea_units[0]) * (PLAYERS_COUNT - 2));
  //  memcpy(&data.other_sea_units[PLAYERS_COUNT - 2], &other_sea_units_temp,
  //  OTHER_SEA_UNITS_SIZE);
  memset(&sea_state.sea_terr, 0, sizeof(sea_state.sea_terr));
  for (SeaIndex dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
    memcpy(&sea_state.other_sea_units[PLAYERS_COUNT - 2][dst_sea], &temp_sea_unit_types[dst_sea],
           SEA_UNIT_TYPES_COUNT - 1);
#pragma unroll
    for (LandUnitType unit_type = 0; unit_type < LAND_UNIT_TYPES_COUNT; unit_type++) {
      SeaTerr* sea0 = &sea_state.sea_terr[dst_sea];
      SeaUnitSumArray* sea1 = &my_sea_unit_types[dst_sea];
      sea0->fighters[FIGHTER_STATES - 1] = (*sea1)[FIGHTERS_SEA];
      sea0->trans_empty[TRANS_EMPTY_STATES - 1] = (*sea1)[TRANS_EMPTY];
      sea0->trans_1i[TRANS_1I_STATES - 1] = (*sea1)[TRANS_1I];
      sea0->trans_1a[TRANS_1A_STATES - 1] = (*sea1)[TRANS_1A];
      sea0->trans_1t[TRANS_1T_STATES - 1] = (*sea1)[TRANS_1T];
      sea0->trans_2i[TRANS_2I_STATES - 1] = (*sea1)[TRANS_2I];
      sea0->trans_1i_1a[TRANS_1I_1A_STATES - 1] = (*sea1)[TRANS_1I_1A];
      sea0->trans_1i_1t[TRANS_1I_1T_STATES - 1] = (*sea1)[TRANS_1I_1T];
      sea0->submarines[SUBMARINE_STATES - 1] = (*sea1)[SUBMARINES];
      sea0->destroyers[DESTROYER_STATES - 1] = (*sea1)[DESTROYERS];
      sea0->carriers[CARRIER_STATES - 1] = (*sea1)[CARRIERS];
      sea0->cruisers[CRUISER_STATES - 1] = (*sea1)[CRUISERS];
      sea0->battleships[BATTLESHIP_STATES - 1] = (*sea1)[BATTLESHIPS];
      sea0->bs_damaged[BATTLESHIP_STATES - 1] = (*sea1)[BS_DAMAGED];
      sea0->bombers[BOMBER_SEA_STATES - 1] = (*sea1)[BOMBERS_SEA];
    }
  }
  Dollars temp_money = sea_state.money[0];
  memmove(&sea_state.money[0], &sea_state.money[1],
          sizeof(sea_state.money[0]) * (PLAYERS_COUNT - 1));
  sea_state.money[PLAYERS_COUNT - 1] = temp_money;
  income_per_turn[PLAYERS_COUNT] = income_per_turn[0];
  memmove(&income_per_turn[0], &income_per_turn[1], sizeof(income_per_turn[0]) * PLAYERS_COUNT);
  factory_count[PLAYERS_COUNT] = factory_count[0];
  memmove(&factory_count[0], &factory_count[1], sizeof(factory_count[0]) * PLAYERS_COUNT);
  memcpy(&factory_locations[PLAYERS_COUNT], &factory_locations[0], sizeof(factory_locations[0]));
  memmove(&factory_locations[0], &factory_locations[1],
          sizeof(factory_locations[0]) * PLAYERS_COUNT);
  memcpy(&player_armies[PLAYERS_COUNT], &player_armies[0], sizeof(player_armies[0]));
  memmove(&player_armies[0], &player_armies[1], sizeof(player_armies[0]) * PLAYERS_COUNT);
  memcpy(&player_navies[PLAYERS_COUNT], &player_navies[0], sizeof(player_navies[0]));
  memmove(&player_navies[0], &player_navies[1], sizeof(player_navies[0]) * PLAYERS_COUNT);
  // reset combat flags
  memset(&sea_state.flagged_for_combat, 0, sizeof(sea_state.flagged_for_combat));
  sea_state.player_index = (sea_state.player_index + 1) % PLAYERS_COUNT;

#pragma unroll
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    sea_state.land_terr[land_idx].owner_idx =
        (sea_state.land_terr[land_idx].owner_idx + PLAYERS_COUNT - 1) % PLAYERS_COUNT;
  }

  for (int factory_idx = 0; factory_idx < factory_count[0]; factory_idx++) {
    int dst_land = factory_locations[0][factory_idx];
    sea_state.builds_left[dst_land] = *factory_max[dst_land];
    SeaConnections* land_to_sea_conn = get_l2s_conn(dst_land);
    SeaIndex land_to_sea_count = get_l2s_count(dst_land);
#pragma unroll
    for (int conn_idx = 0; conn_idx < MAX_LAND_TO_SEA_CONNECTIONS; conn_idx++) {
      if (conn_idx >= land_to_sea_count) {
        break;
      }
      sea_state.builds_left[convert_sea_to_air(get_sea_from_conn(land_to_sea_conn, conn_idx))] +=
          *factory_max[dst_land];
    }
  }

  refresh_cache();
#ifdef DEBUG
  printf("DEBUG: Cache refreshed. Player %s's turn\n", get_player_name(sea_state.player_index));
  setPrintableStatus();
  printf("%s\n", printableGameStatus);
  for (PlayerIndex player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    LandConnIndex factory_count_temp = factory_count[player_idx];
#pragma unroll
    for (FactoryLocIndex factory_index = 0; factory_index < LANDS_COUNT; factory_index++) {
      if (factory_index >= factory_count_temp) {
        break;
      }
      LandIndex factory_location = factory_locations[player_idx][factory_index];
      if (get_land_owner(factory_location) != player_idx) {
        printf("DEBUG: Player %s has a unowned factory at %s\n",
               get_player_name(sea_state.player_index), get_land_name(factory_location));
      }
    }
  }
#endif

  // json = serialize_game_data_to_json(&data);
  // write_json_to_file("backup_game.json", json);
  // cJSON_Delete(json);
}

double get_score() {
  Dollars allied_score = 1; // one helps prevent division by zero
  Dollars enemy_score = 1;
  for (PlayerIndex player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    Dollars score = sea_state.money[player_idx];
    for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
#pragma unroll
      for (LandUnitType unit_type = 0; unit_type < LAND_UNIT_TYPES_COUNT; unit_type++) {
        score +=
            (*player_land_unit_types[player_idx])[land_idx][unit_type] * COST_UNIT_LAND[unit_type];
      }
    }
    for (SeaIndex sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
#pragma unroll
      for (SeaUnitType unit_type = 0; unit_type < SEA_UNIT_TYPES_COUNT; unit_type++) {
        score +=
            (*player_sea_unit_types[player_idx])[sea_idx][unit_type] * COST_UNIT_SEA[unit_type];
      }
    }
    if (is_allied(player_idx)) {
      allied_score += score;
    } else {
      enemy_score += score;
    }
  }
  // return ((double)2 * (double)allied_score / (double)(enemy_score + allied_score)) - (double)1;
  return ((double)allied_score / (double)(enemy_score + allied_score));
}

void play_full_turn() {
  // clear printableGameStatus
  move_fighter_units();
  setPrintableStatus();
  move_bomber_units();
#ifdef DEBUG
  debug_checks();
#endif
  stage_transport_units();
#ifdef DEBUG
  debug_checks();
#endif
  move_land_unit_type(TANKS);
  move_land_unit_type(ARTILLERY);
  move_land_unit_type(INFANTRY);
#ifdef DEBUG
  debug_checks();
#endif
  move_transport_units();
  move_subs();
  move_destroyers_battleships();
  resolve_sea_battles();
#ifdef DEBUG
  debug_checks();
#endif
  unload_transports();
#ifdef DEBUG
  debug_checks();
#endif
  resolve_land_battles();
#ifdef DEBUG
  debug_checks();
#endif
  move_land_unit_type(AA_GUNS);
#ifdef DEBUG
  debug_checks();
#endif
  land_fighter_units();
#ifdef DEBUG
  debug_checks();
#endif
  land_bomber_units();
#ifdef DEBUG
  debug_checks();
#endif
  buy_units();
#ifdef DEBUG
  debug_checks();
#endif
  crash_air_units();
#ifdef DEBUG
  debug_checks();
#endif
  reset_units_fully();
#ifdef DEBUG
  debug_checks();
#endif
  buy_factory();
#ifdef DEBUG
  debug_checks();
#endif
  collect_money();
  rotate_turns();
}

GameState* get_game_state_copy() {
  GameState* game_state = (GameState*)malloc(sizeof(GameState));
  memcpy(game_state, &sea_state, sizeof(GameState));
  return game_state;
}

// Implement these functions based on your game logic
GameState* clone_state(GameState* game_state) {
  // Deep copy the game state
  GameState* new_state = (GameState*)malloc(sizeof(GameState));
  // Copy the basic structure
  memcpy(new_state, game_state, sizeof(GameState));
  return new_state;
}

void free_state(GameState* game_state) {
  // Free the memory allocated for the game state
  free(game_state);
}

ActionArray* get_possible_actions(GameState* game_state, int* num_actions) {
  // Return the list of possible actions from the given state
  memcpy(&sea_state, game_state, sizeof(GameState));
  refresh_quick_totals();
  refresh_cache();
  answers_remaining = 0;
#pragma unroll
  while (true) {
    if (move_fighter_units()) {
      break;
    }
    if (move_bomber_units()) {
      break;
    }
    if (stage_transport_units()) {
      break;
    }
    if (move_land_unit_type(TANKS)) {
      break;
    }
    if (move_land_unit_type(ARTILLERY)) {
      break;
    }
    if (move_land_unit_type(INFANTRY)) {
      break;
    }
    if (move_transport_units()) {
      break;
    }
    if (move_subs()) {
      break;
    }
    if (move_destroyers_battleships()) {
      break;
    }
    if (resolve_sea_battles()) {
      break;
    }
    if (unload_transports()) {
      break;
    }
    if (resolve_land_battles()) {
      break;
    }
    if (move_land_unit_type(AA_GUNS)) {
      break;
    }
    if (land_fighter_units()) {
      break;
    }
    if (land_bomber_units()) {
      break;
    }
    if (buy_units()) {
      break;
    }
    crash_air_units();
    reset_units_fully();
    buy_factory();
    collect_money();
    rotate_turns();
  }
  *num_actions = valid_actions_count;
  return &valid_actions;
}

void apply_action(GameState* game_state, Action action) {
  // Apply the action to the game state
#ifdef DEBUG
  printf("DEBUG: copying state and Applying action %d\n", action);
#endif
  memcpy(&sea_state, game_state, sizeof(GameState));
  refresh_quick_totals();
  refresh_cache();
  answers_remaining = 1;
  selected_answer = action;
#pragma unroll
  while (true) {
    if (move_fighter_units()) {
      break;
    }
    if (move_bomber_units()) {
      break;
    }
    if (stage_transport_units()) {
      break;
    }
    if (move_land_unit_type(TANKS)) {
      break;
    }
    if (move_land_unit_type(ARTILLERY)) {
      break;
    }
    if (move_land_unit_type(INFANTRY)) {
      break;
    }
    if (move_transport_units()) {
      break;
    }
    if (move_subs()) {
      break;
    }
    if (move_destroyers_battleships()) {
      break;
    }
    if (resolve_sea_battles()) {
      break;
    }
    if (unload_transports()) {
      break;
    }
    if (resolve_land_battles()) {
      break;
    }
    if (move_land_unit_type(AA_GUNS)) {
      break;
    }
    if (land_fighter_units()) {
      break;
    }
    if (land_bomber_units()) {
      break;
    }
    if (buy_units()) {
      break;
    }
    crash_air_units();
    reset_units_fully();
    buy_factory();
    collect_money();
    rotate_turns();
  }
  memcpy(game_state, &sea_state, sizeof(GameState));
}
#define MAX_LOOPS 100000
#define MAX_ANSWERS 10000
#define MIN_SCORE 0.01
#define MAX_SCORE 0.99
double random_play_until_terminal(GameState* game_state) {
  memcpy(&sea_state, game_state, sizeof(GameState));
  PlayerIndex starting_player = sea_state.player_index;
  refresh_quick_totals();
  refresh_cache();
  answers_remaining = MAX_ANSWERS;
  double score = get_score();
  int max_loops = MAX_LOOPS;
#pragma unroll 4
  while (max_loops-- > 0) {
    if (score > MIN_SCORE && score < MAX_SCORE) {
      break;
    }
    // printf("max_loops: %d\n", max_loops);
    //  if(max_loops == 2) {
    //    setPrintableStatus();
    //    printf("%s\n", printableGameStatus);
    //    printf("DEBUG: max_loops reached\n");
    //  }
    move_fighter_units();
    move_bomber_units();
    stage_transport_units();
    move_land_unit_type(TANKS);
    move_land_unit_type(ARTILLERY);
    move_land_unit_type(INFANTRY);
    move_transport_units();
    move_subs();
    move_destroyers_battleships();
    resolve_sea_battles();
    unload_transports();
    resolve_land_battles();
    move_land_unit_type(AA_GUNS);
    land_fighter_units();
    land_bomber_units();
    buy_units();
    crash_air_units();
    reset_units_fully();
    buy_factory();
    collect_money();
    rotate_turns();
    score = get_score();
  }
  if (are_players_allied(game_state->player_index,
                         (game_state->player_index + starting_player) % PLAYERS_COUNT)) {
    score = 1 - score;
  }
  return score;
}

bool is_terminal_state(GameState* game_state) {
  // Return true if the game is over
  double score = evaluate_state(game_state);
  return (score > MAX_SCORE || score < MIN_SCORE);
}

double evaluate_state(GameState* game_state) {
  // Evaluate the game state and return a score
  Dollars allied_score = 1; // one helps prevent division by zero
  Dollars enemy_score = 1;
  allied_score += game_state->money[0];
  for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    LandTerr* land_state = &game_state->land_terr[land_idx];
    ArmySum total_units = 0;
#pragma unroll
    for (FighterState unit_state = 0; unit_state < FIGHTER_STATES; unit_state++) {
      total_units += land_state->fighters[unit_state];
    }
    allied_score += total_units * FIGHTER_COST;
#pragma unroll
    for (BomberLandState unit_state = 0; unit_state < BOMBER_LAND_STATES; unit_state++) {
      total_units += land_state->bombers[unit_state];
    }
    allied_score += total_units * BOMBER_COST;
#pragma unroll
    for (InfantryState unit_state = 0; unit_state < INFANTRY_STATES; unit_state++) {
      total_units += land_state->infantry[unit_state];
    }
    allied_score += total_units * INFANTRY_COST;
    total_units = 0;
#pragma unroll
    for (ArtilleryState unit_state = 0; unit_state < ARTILLERY_STATES; unit_state++) {
      total_units += land_state->artillery[unit_state];
    }
    allied_score += total_units * ARTILLERY_COST;
    total_units = 0;
#pragma unroll
    for (TankState unit_state = 0; unit_state < TANK_STATES; unit_state++) {
      total_units += land_state->tanks[unit_state];
    }
    allied_score += total_units * TANK_COST;
    total_units = 0;
#pragma unroll
    for (AAGunState unit_state = 0; unit_state < AA_GUN_STATES; unit_state++) {
      total_units += land_state->aa_guns[unit_state];
    }
    allied_score += total_units * AA_GUN_COST;
  }
  for (SeaIndex sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    SeaTerr* sea_state = &game_state->sea_terr[sea_idx];
    NavySum total_units = 0;
#pragma unroll
    for (FighterState unit_state = 0; unit_state < FIGHTER_STATES; unit_state++) {
      total_units += sea_state->fighters[unit_state];
    }
    allied_score += total_units * FIGHTER_COST;
#pragma unroll
    for (TransEmptyState unit_state = 0; unit_state < TRANS_EMPTY_STATES; unit_state++) {
      total_units += sea_state->trans_empty[unit_state];
    }
    allied_score += total_units * TRANSPORT_COST;
#pragma unroll
    for (Trans1IState unit_state = 0; unit_state < TRANS_1I_STATES; unit_state++) {
      total_units += sea_state->trans_1i[unit_state];
    }
    allied_score += total_units * (TRANSPORT_COST + INFANTRY_COST);
#pragma unroll
    for (Trans1AState unit_state = 0; unit_state < TRANS_1A_STATES; unit_state++) {
      total_units += sea_state->trans_1a[unit_state];
    }
    allied_score += total_units * (TRANSPORT_COST + ARTILLERY_COST);
#pragma unroll
    for (Trans1TState unit_state = 0; unit_state < TRANS_1T_STATES; unit_state++) {
      total_units += sea_state->trans_1t[unit_state];
    }
    allied_score += total_units * (TRANSPORT_COST + TANK_COST);
#pragma unroll
    for (Trans2IState unit_state = 0; unit_state < TRANS_2I_STATES; unit_state++) {
      total_units += sea_state->trans_2i[unit_state];
    }
    allied_score += total_units * (TRANSPORT_COST + INFANTRY_COST + INFANTRY_COST);
#pragma unroll
    for (Trans1I1AState unit_state = 0; unit_state < TRANS_1I_1A_STATES; unit_state++) {
      total_units += sea_state->trans_1i_1a[unit_state];
    }
    allied_score += total_units * (TRANSPORT_COST + INFANTRY_COST + ARTILLERY_COST);
#pragma unroll
    for (Trans1I1TState unit_state = 0; unit_state < TRANS_1I_1T_STATES; unit_state++) {
      total_units += sea_state->trans_1i_1t[unit_state];
    }
    allied_score += total_units * (TRANSPORT_COST + INFANTRY_COST + TANK_COST);
#pragma unroll
    for (SubmarineState unit_state = 0; unit_state < SUBMARINE_STATES; unit_state++) {
      total_units += sea_state->submarines[unit_state];
    }
    allied_score += total_units * SUB_COST;
#pragma unroll
    for (DestroyerState unit_state = 0; unit_state < DESTROYER_STATES; unit_state++) {
      total_units += sea_state->destroyers[unit_state];
    }
    allied_score += total_units * DESTROYER_COST;
#pragma unroll
    for (CarrierState unit_state = 0; unit_state < CARRIER_STATES; unit_state++) {
      total_units += sea_state->carriers[unit_state];
    }
    allied_score += total_units * CARRIER_COST;
#pragma unroll
    for (CruiserState unit_state = 0; unit_state < CRUISER_STATES; unit_state++) {
      total_units += sea_state->cruisers[unit_state];
    }
    allied_score += total_units * CRUISER_COST;
#pragma unroll
    for (BattleshipState unit_state = 0; unit_state < BATTLESHIP_STATES; unit_state++) {
      total_units += sea_state->battleships[unit_state];
    }
    allied_score += total_units * BATTLESHIP_COST;
#pragma unroll
    for (BattleshipState unit_state = 0; unit_state < BATTLESHIP_STATES; unit_state++) {
      total_units += sea_state->bs_damaged[unit_state];
    }
    allied_score += total_units * BATTLESHIP_COST;
#pragma unroll
    for (BomberSeaState unit_state = 0; unit_state < BOMBER_SEA_STATES; unit_state++) {
      total_units += sea_state->bombers[unit_state];
    }
    allied_score += total_units * BOMBER_COST;
  }
  for (PlayerIndex player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
    Dollars score = game_state->money[player_idx];
    for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
#pragma unroll
      for (LandUnitType unit_type = 0; unit_type < LAND_UNIT_TYPES_COUNT; unit_type++) {
        score +=
            (*player_land_unit_types[player_idx])[land_idx][unit_type] * COST_UNIT_LAND[unit_type];
      }
    }
    for (SeaIndex sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
#pragma unroll
      for (SeaUnitType unit_type = 0; unit_type < SEA_UNIT_TYPES_COUNT; unit_type++) {
        score +=
            (*player_sea_unit_types[player_idx])[sea_idx][unit_type] * COST_UNIT_SEA[unit_type];
      }
    }
    if (are_players_allied(game_state->player_index,
                           (game_state->player_index + player_idx) % PLAYERS_COUNT)) {
      allied_score += score;
    } else {
      enemy_score += score;
    }
  }
  return ((double)allied_score / (double)(enemy_score + allied_score));
}
