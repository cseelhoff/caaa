#include "engine.h"
#include "air.h"
#include "game_state.h"
#include "land.h"
#include "mcts.h"
#include "player.h"
#include "sea.h"
#include "serialize_data.h"
#include "typedefs.h"
#include "units/artillery.h"
#include "units/battleship.h"
#include "units/bomber.h"
#include "units/fighter.h"
#include "units/infantry.h"
#include "units/sub.h"
#include "units/transport.h"
#include "units/units.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
//#include <cjson/cJSON.h> todo does this work?

typedef uint8_t Action;
typedef uint8_t ActionCount;

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

#define RANDOM_NUMBERS_SIZE 65536
Action RANDOM_NUMBERS[RANDOM_NUMBERS_SIZE] = {0};
u_short random_number_index = 0;
u_short seed = 0;
char printableGameStatus[PRINTABLE_GAME_STATUS_SIZE] = "";
GameState state = {0};
cJSON* json;

LandUnitTypesSumArrayLands my_land_unit_types;   // temp
SeaUnitTypesSumArraySeas my_sea_unit_types;      // temp
LandUnitTypesSumArrayLands temp_land_unit_types; // temp
SeaUnitTypesSumArraySeas temp_sea_unit_types;    // temp
LandUnitTypesSumArrayLands* player_land_unit_types[PLAYERS_COUNT] = {
    &my_land_unit_types, &state.other_land_units[0], &state.other_land_units[1],
    &state.other_land_units[2], &state.other_land_units[3]};
SeaUnitTypesSumArraySeas* player_sea_unit_types[PLAYERS_COUNT] = {
    &my_sea_unit_types, &state.other_sea_units[0], &state.other_sea_units[1],
    &state.other_sea_units[2], &state.other_sea_units[3]};
typedef LandUnitStateSum LandUnitStateSums[MAX_LAND_UNIT_STATES];
typedef LandUnitStateSums* LandUnitStates[LAND_UNIT_TYPES_COUNT];
LandUnitStates my_land_unit_states[LANDS_COUNT] = {0};
typedef SeaUnitStateSum SeaUnitStateSums[MAX_SEA_UNIT_STATES];
typedef SeaUnitStateSums* SeaUnitStates[SEA_UNIT_TYPES_COUNT];
SeaUnitStates my_sea_unit_states[SEAS_COUNT] = {0};

PlayerIndex* owner_idx[LANDS_COUNT];
uint8_t* factory_max[LANDS_COUNT];
uint8_t* bombard_max[LANDS_COUNT];
int8_t* factory_hp[LANDS_COUNT]; // allow negative
Dollars income_per_turn[PLAYERS_COUNT + 1];
LandIndexCount factory_count[PLAYERS_COUNT + 1] = {0};

typedef LandIndex FactoryLocations[LANDS_COUNT];
FactoryLocations factory_locations[PLAYERS_COUNT + 1] = {0};

typedef ArmySum LandArmySumArray[LANDS_COUNT];
LandArmySumArray player_armies[PLAYERS_COUNT + 1];
typedef NavySum SeaNavySumArray[SEAS_COUNT];
SeaNavySumArray player_navies[PLAYERS_COUNT + 1];

typedef AirIndex AirIndexArray[AIRS_COUNT];
AirIndexArray hist_skipped_airs[AIRS_COUNT] = {0};
AirIndexArray hist_source_territories[AIRS_COUNT] = {0};
AirIndexCount hist_source_territories_count[AIRS_COUNT] = {0};

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

typedef uint8_t PlayerIndexCount;
typedef uint8_t AirMilitaryCount;

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
ActionCount answers_remaining = 0;
#define VALID_ACTIONS_SIZE 256
Action valid_actions[VALID_ACTIONS_SIZE] = {0};
ActionCount valid_actions_count = 0;

void initializeGameData() {
  generate_total_land_distance();
  generate_total_sea_distance();
  generate_total_air_distance();
  generate_landMoveAllDestination();
  generate_seaMoveAllDestination();
  generate_airMoveAllDestination();
  generate_within_x_moves();
  generate_random_numbers();
  generate_land_info_pointers();
  json = serialize_game_data_to_json(&state);
  // write_json_to_file("game_data_0.json", json);
  cJSON_Delete(json);
#ifdef DEBUG
  printf("Exiting initializeGameData");
#endif
}

#define PATH_MAX 4096
void load_game_data(char* filename) {
  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("Current working directory: ");
    printf("%s", cwd);
  } else {
    perror("getcwd() error");
  }
  json = read_json_from_file(filename);
  memset(&state, 0, sizeof(state));
  memset(&my_land_unit_types, 0, sizeof(my_land_unit_types));
  memset(&my_sea_unit_types, 0, sizeof(my_sea_unit_types));
  memset(&player_armies, 0, sizeof(player_armies));
  memset(&player_navies, 0, sizeof(player_navies));
  deserialize_game_data_from_json(&state, json);
  cJSON_Delete(json);
  refresh_quick_totals();
  refresh_cache();
  printf("Exiting load_game_data");
}

void set_seed(u_short new_seed) {
  seed = new_seed;
  random_number_index = new_seed;
}

void play_full_turn() {
  // clear printableGameStatus
  move_fighter_units();
  setPrintableStatus();
  move_bomber_units();
  debug_checks();
  stage_transport_units();
  debug_checks();
  move_land_unit_type(TANKS);
  move_land_unit_type(ARTILLERY);
  move_land_unit_type(INFANTRY);
  debug_checks();
  move_transport_units();
  move_subs();
  move_destroyers_battleships();
  resolve_sea_battles();
  debug_checks();
  unload_transports();
  debug_checks();
  resolve_land_battles();
  debug_checks();
  move_land_unit_type(AA_GUNS);
  debug_checks();
  land_fighter_units();
  debug_checks();
  land_bomber_units();
  debug_checks();
  buy_units();
  debug_checks();
  crash_air_units();
  debug_checks();
  reset_units_fully();
  debug_checks();
  buy_factory();
  debug_checks();
  collect_money();
  rotate_turns();
}

void generate_total_sea_distance() {
  initialize_sea_connections();
#pragma unroll
  for (uint8_t canal_idx = 0; canal_idx < CANAL_STATES; canal_idx++) {
    initialize_sea_dist(canal_idx);
    populate_initial_distances_sea(canal_idx);
    process_canals(canal_idx);
    floyd_warshall(canal_idx);
  }
}
void generate_total_air_distance() {
  initialize_air_dist();
  populate_initial_distances();
  floyd_warshall_air();
}

void generate_random_numbers() {
  for (int i = 0; i < 65536; i++) {
    RANDOM_NUMBERS[i] = rand() % 256;
  }
}

void generate_land_info_pointers() {
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    owner_idx[land_idx] = &state.land_terr[land_idx].owner_idx;
    bombard_max[land_idx] = &state.land_terr[land_idx].bombard_max;
    factory_hp[land_idx] = &state.land_terr[land_idx].factory_hp;
    factory_max[land_idx] = &state.land_terr[land_idx].factory_max;
  }
}

inline void set_income_per_turn(PlayerIndex player_idx, Dollars income) {
  income_per_turn[player_idx] = income;
}

inline void set_factory_count(PlayerIndex player_idx, LandIndexCount count) {
  factory_count[player_idx] = count;
}

inline uint8_t get_factory_max(LandIndex land_idx) { return *factory_max[land_idx]; }

inline void add_factory_location(PlayerIndex player_idx, LandIndex land_idx) {
  factory_locations[player_idx][factory_count[player_idx]++] = land_idx;
}

inline void acc_income_from_land(PlayerIndex player_idx, LandIndex land_idx) {
  income_per_turn[player_idx] += get_land_value(land_idx);
}

void refresh_quick_totals() {
  for (PlayerIndex player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    set_income_per_turn(player_idx, 0);
    set_factory_count(player_idx, 0);
  }
}

inline void clear_LandUnitSumArray(LandUnitSumArray* sumArray, LandUnitType unit) {
  (*sumArray)[unit] = 0;
}

inline PlayerIndex get_land_owner(LandIndex land_idx) { return *owner_idx[land_idx]; }

inline LandUnitSumArray* get_my_land_unit_types(LandIndex land_idx) {
  return &my_land_unit_types[land_idx];
}

inline LandTerr* get_land_terr(LandIndex land_idx) { return &state.land_terr[land_idx]; }

inline ArmySum* get_player_armies_ref(PlayerIndex player_idx, LandIndex land_idx) {
  return &player_armies[player_idx][land_idx];
}

inline LandUnitSumArray* get_player_land_unit_types(PlayerIndex player_idx, LandIndex land_idx) {
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
  (*land_units)[unit_type] += *(land_unit_states)[unit_type][land_unit_state];
}
inline LandUnitStates* get_my_land_unit_states(LandIndex land_idx) {
  return &my_land_unit_states[land_idx];
}

void refresh_quick_totals_land() {
  for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    PlayerIndex land_owner = get_land_owner(land_idx);
    if (get_factory_max(land_idx) > 0) {
      add_factory_location(land_owner, land_idx);
    }
    acc_income_from_land(land_owner, land_idx);
    ArmySum* army_sum = get_player_armies_ref(0, land_idx);
    *army_sum = 0;
    LandTerr* land_terr = get_land_terr(land_idx);
    LandUnitSumArray* land_units = get_my_land_unit_types(land_idx);
    LandUnitStates* land_unit_states = get_my_land_unit_states(land_idx);

    (*land_unit_states)[FIGHTERS_LAND] = land_terr->fighters;
    (*land_unit_states)[BOMBERS_LAND] = land_terr->bombers;
    (*land_unit_states)[INFANTRY] = land_terr->infantry;
    (*land_unit_states)[ARTILLERY] = land_terr->artillery;
    (*land_unit_states)[TANKS] = land_terr->tanks;
    (*land_unit_states)[AA_GUNS] = land_terr->aa_guns;

    for (LandUnitType unit_type = 0; unit_type < LAND_UNIT_TYPES_COUNT; unit_type++) {
      clear_LandUnitSumArray(land_units, unit_type);
      for (uint8_t land_unit_state = 0; land_unit_state < STATES_MOVE_LAND[unit_type];
           land_unit_state++) {
        acc_LandUnitSumArray(land_units, unit_type, land_unit_states, land_unit_state);
      }
      acc_ArmySumArray(army_sum, land_units, unit_type);
    }

    for (PlayerIndex player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      army_sum = get_player_armies_ref(player_idx, land_idx);
      *army_sum = 0;
      land_units = get_player_land_unit_types(player_idx, land_idx);
      for (LandUnitType unit_type = 0; unit_type < LAND_UNIT_TYPES_COUNT; unit_type++) {
        acc_ArmySumArray(army_sum, land_units, unit_type);
      }
    }
  }
}

inline NavySum* get_player_navies_ref(PlayerIndex player_idx, SeaIndex sea_idx) {
  return &player_navies[player_idx][sea_idx];
}

inline void set_sea_unit_type_sum(SeaUnitSumArray* sea_units, SeaUnitType unit_type,
                                  SeaUnitStateSum sum) {
  (*sea_units)[unit_type] = sum;
}

inline SeaUnitSumArray* get_my_sea_unit_types(SeaIndex sea_idx) {
  return &my_sea_unit_types[sea_idx];
}

inline SeaTerr* get_sea_terr(SeaIndex sea_idx) { return &state.sea_terr[sea_idx]; }

inline SeaUnitSumArray* get_player_sea_unit_types_ref(PlayerIndex player_idx, SeaIndex sea_idx) {
  return player_sea_unit_types[player_idx][sea_idx];
}

inline void clear_SeaUnitSumArray(SeaUnitSumArray* sea_units, SeaUnitType unit) {
  (*sea_units)[unit] = 0;
}

inline void acc_SeaNavySum(NavySum* sum, SeaUnitSumArray* seaUnitTypesSumArray,
                           SeaUnitType sea_unit_type) {
  *sum += (*seaUnitTypesSumArray)[sea_unit_type];
}

inline void acc_SeaUnitSumArray(SeaUnitSumArray* sea_units, SeaUnitType unit_type,
                                SeaUnitStates* sea_unit_states, GenericSeaUnitState sea_unit_state) {
  (*sea_units)[unit_type] += (*sea_unit_states)[unit_type][sea_unit_state];
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

void refresh_quick_totals_sea() {
  for (SeaIndex sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    NavySum* navy_sum = get_player_navies_ref(0, sea_idx);
    *navy_sum = 0;
    SeaTerr* sea_terr = get_sea_terr(sea_idx);
    SeaUnitSumArray* sea_units = get_my_sea_unit_types(sea_idx);
    SeaUnitStates* sea_unit_states = get_my_sea_unit_states(sea_idx);

    (*sea_unit_states)[FIGHTERS_SEA] = sea_terr->fighters;
    (*sea_unit_states)[TRANS_EMPTY] = sea_terr->trans_empty;
    (*sea_unit_states)[TRANS_1I] = sea_terr->trans_1i;
    (*sea_unit_states)[TRANS_1A] = sea_terr->trans_1a;
    (*sea_unit_states)[TRANS_1T] = sea_terr->trans_1t;
    (*sea_unit_states)[TRANS_2I] = sea_terr->trans_2i;
    (*sea_unit_states)[TRANS_1I_1A] = sea_terr->trans_1i_1a;
    (*sea_unit_states)[TRANS_1I_1T] = sea_terr->trans_1i_1t;
    (*sea_unit_states)[SUBMARINES] = sea_terr->submarines;
    (*sea_unit_states)[DESTROYERS] = sea_terr->destroyers;
    (*sea_unit_states)[CARRIERS] = sea_terr->carriers;
    (*sea_unit_states)[CRUISERS] = sea_terr->cruisers;
    (*sea_unit_states)[BATTLESHIPS] = sea_terr->battleships;
    (*sea_unit_states)[BS_DAMAGED] = sea_terr->bs_damaged;
    (*sea_unit_states)[BOMBERS_SEA] = sea_terr->bombers;

    for (SeaUnitType unit_type = 0; unit_type < SEA_UNIT_TYPES_COUNT; unit_type++) {
      clear_SeaUnitSumArray(sea_units, unit_type);
      for (uint8_t sea_unit_state = 0; sea_unit_state < STATES_MOVE_SEA[unit_type];
           sea_unit_state++) {
        acc_SeaUnitSumArray(sea_units, unit_type, sea_unit_states, sea_unit_state);
      }
      acc_NavySumArray(navy_sum, sea_units, unit_type);
    }

    for (PlayerIndex player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      navy_sum = get_player_navies_ref(player_idx, sea_idx);
      *navy_sum = 0;
      sea_units = get_player_sea_unit_types_ref(player_idx, sea_idx);
      for (SeaUnitType unit_type = 0; unit_type < LAND_UNIT_TYPES_COUNT; unit_type++) {
        *navy_sum += get_sea_unit_sum(sea_units, unit_type);
      }
    }
  }
}

inline void set_ally(PlayerIndex player_idx) {
  is_allied_0[player_idx] =
      PLAYERS[state.player_index].is_allied[(state.player_index + player_idx) % PLAYERS_COUNT];
}
inline bool is_allied(PlayerIndex player_idx) { return is_allied_0[player_idx]; }
inline void add_enemy(PlayerIndex player_idx) { enemies_0[enemies_count_0++] = player_idx; }
void refresh_cache_alliance() {
  // copy for quick cache lookups
  enemies_count_0 = 0;
  for (PlayerIndex player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    set_ally(player_idx);
    if (!is_allied(player_idx)) {
      add_enemy(player_idx);
    }
  }
}

inline LandIndex get_canal_land0(CanalState canal_idx) { return CANALS[canal_idx].lands[0]; }
inline LandIndex get_canal_land1(CanalState canal_idx) { return CANALS[canal_idx].lands[1]; }
inline bool is_canal_controlled(CanalState canal_idx) {
  return is_allied(get_land_owner(get_canal_land0(canal_idx))) &&
         is_allied(get_land_owner(get_canal_land1(canal_idx)));
}
void refresh_cache_canals() {
  canal_state = 0;
  for (CanalState canal_idx = 0; canal_idx < CANALS_COUNT; canal_idx++) {
    if (is_canal_controlled(canal_idx)) {
      canal_state += 1 << canal_idx;
    }
  }
}

typedef uint8_t EnemyIndex;
inline void clear_enemy_unit_count(AirIndex air_idx) { enemy_units_count[air_idx] = 0; }
inline PlayerIndex get_enemy_player(EnemyIndex enemy_idx) { return enemies_0[enemy_idx]; }
inline ArmySum get_player_armies(PlayerIndex player_idx, LandIndex land_idx) {
  return player_armies[player_idx][land_idx];
}
inline AirIndex convert_land_to_air(LandIndex land_idx) { return (AirIndex)land_idx; }

inline void acc_enemy_units_count(AirIndex air_idx, AirMilitaryCount sum) {
  enemy_units_count[air_idx] += sum;
}

inline AirMilitaryCount get_enemy_units_count(AirIndex air_idx) {
  return enemy_units_count[air_idx];
}

inline void set_is_land_path_blocked(LandIndex src_land, LandIndex dst_land) {
  LandIndex nextLandMovement = get_land_path1(src_land, dst_land);
  LandIndex nextLandMovementAlt = get_land_path_alt(src_land, dst_land);
  is_land_path_blocked[src_land][dst_land] =
      (get_enemy_units_count(nextLandMovement) > 0 || get_factory_max(nextLandMovement) > 0) &&
      (get_enemy_units_count(nextLandMovementAlt) > 0 || get_factory_max(nextLandMovementAlt) > 0);
}

void refresh_cache_enemy_armies() {
  for (LandIndex src_land = 0; src_land < LANDS_COUNT; src_land++) {
    clear_enemy_unit_count(convert_land_to_air(src_land));
    for (EnemyIndex enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
      acc_enemy_units_count(convert_land_to_air(src_land),
                            get_player_armies(get_enemy_player(enemy_idx), src_land));
    }
    for (LandIndex dst_land = 0; dst_land < LANDS_COUNT; dst_land++) {
      set_is_land_path_blocked(src_land, dst_land);
    }
  }
}

inline void set_allied_carriers(SeaIndex sea_idx, NavySum carriers) {
  allied_carriers[sea_idx] = carriers;
}

inline void clear_enemy_destroyers(SeaIndex sea_idx) { enemy_destroyers[sea_idx] = 0; }
inline void clear_enemy_blockade(SeaIndex sea_idx) { enemy_blockade[sea_idx] = 0; }

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

inline void acc_allied_carriers(SeaIndex sea_idx, SeaUnitSumArray* sea_units) {
  allied_carriers[sea_idx] += (*sea_units)[CARRIERS];
}

inline void set_transports_cargo_space(SeaIndex sea_idx, SeaUnitSumArray* sea_units) {
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

void refresh_cache() {
  refresh_cache_alliance();
  refresh_cache_canals();
  refresh_cache_enemy_armies();

  for (SeaIndex src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    SeaUnitSumArray* sea_units_0 = get_my_sea_unit_types(src_sea);
    set_allied_carriers(src_sea, *sea_units_0[CARRIERS]);
    AirIndex air_idx = convert_sea_to_air(src_sea);
    clear_enemy_unit_count(air_idx);
    clear_enemy_destroyers(src_sea);
    clear_enemy_blockade(src_sea);
    for (PlayerIndex player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      SeaUnitSumArray* sea_units = get_player_sea_unit_types_ref(player_idx, src_sea);
      if (!is_allied(player_idx)) {
        acc_enemy_units_count(air_idx, get_player_navies(player_idx, src_sea));
        acc_enemy_destroyers(src_sea, sea_units);
        acc_enemy_blockade(src_sea, sea_units);
      } else {
        acc_allied_carriers(src_sea, sea_units);
      }
    }
    set_transports_cargo_space(src_sea, sea_units_0);
    for (SeaIndex dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
      SeaIndex nextSeaMovement = get_sea_path1(canal_state, src_sea, dst_sea);
      SeaIndex nextSeaMovementAlt = get_sea_path1_alt(canal_state, src_sea, dst_sea);
      set_is_sea_path_blocked(src_sea, dst_sea, nextSeaMovement, nextSeaMovementAlt);
      set_is_sub_path_blocked(src_sea, dst_sea, nextSeaMovement, nextSeaMovementAlt);
    }
  }
}
/*
void debug_checks() {
#ifdef DEBUG
  step_id++;
  printf("step_id: %d  seed: %d\n", step_id, seed);
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    for (int unit_idx = 0; unit_idx < LAND_UNIT_TYPES_COUNT; unit_idx++) {
      int temp_unit_type_total = 0;
      for (int cur_unit_state = 0; cur_unit_state < STATES_MOVE_LAND[unit_idx]; cur_unit_state++) {
        temp_unit_type_total += land_units_state[land_idx][unit_idx][cur_unit_state];
      }
      if (temp_unit_type_total != my_land_unit_types[land_idx][unit_idx]) {
        printf("temp_unit_type_total %d != current_player_land_unit_types[land_idx][unit_idx] %d",
               temp_unit_type_total, my_land_unit_types[land_idx][unit_idx]);
      }
    }
  }
  for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    for (int unit_idx = 0; unit_idx < SEA_UNIT_TYPES_COUNT; unit_idx++) {
      int temp_unit_type_total = 0;
      for (int cur_unit_state = 0; cur_unit_state < STATES_MOVE_SEA[unit_idx]; cur_unit_state++) {
        temp_unit_type_total += sea_units_state[sea_idx][unit_idx][cur_unit_state];
      }
      if (temp_unit_type_total != my_sea_unit_types[sea_idx][unit_idx]) {
        printf("temp_unit_type_total %d != current_player_sea_unit_types[sea_idx][unit_idx] %d",
               temp_unit_type_total, my_sea_unit_types[sea_idx][unit_idx]);
      }
    }
  }
  for (int player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
      int total = 0;
      int total_data = 0;
      for (int unit_idx = 0; unit_idx < LAND_UNIT_TYPES_COUNT; unit_idx++) {
        if (player_idx == 0 || unit_idx < LAND_UNIT_TYPES_COUNT)
          total += player_land_unit_types[player_idx][land_idx][unit_idx];
        if (player_idx == 0) {
          total_data += my_land_unit_types[land_idx][unit_idx];
        } else {
          if (unit_idx < LAND_UNIT_TYPES_COUNT)
            total_data += state.other_land_units[player_idx - 1][land_idx][unit_idx];
        }
      }
      if (total != player_armies[player_idx][land_idx] || total != total_data) {
        printf("total %d != total_player_land_units[player_idx][land_idx] %d != total_data %d",
               total, player_armies[player_idx][land_idx], total_data);
      }

      if (player_armies[player_idx][land_idx] < 0 || player_armies[player_idx][land_idx] > 240) {
        printf("units_land_player_total[player_idx][land_idx] < 0");
      }
      if (enemy_units_count[land_idx] < 0 || enemy_units_count[land_idx] > 240) {
        printf("enemy_units_count[land_idx] < 0");
      }
      total = 0;
      for (int unit_idx = 0; unit_idx < LAND_UNIT_TYPES_COUNT; unit_idx++) {
        if (player_land_unit_types[player_idx][land_idx][unit_idx] < 0 ||
            player_land_unit_types[player_idx][land_idx][unit_idx] > 240) {
          printf("total_player_land_unit_types[player_idx][land_idx][unit_idx] < 0");
        }
        total += player_land_unit_types[player_idx][land_idx][unit_idx];
      }
      if (total != player_armies[player_idx][land_idx]) {
        printf("total %d != total_player_land_units[player_idx][land_idx] %d", total,
               player_armies[player_idx][land_idx]);
      }
      int enemy_total = 0;
      for (uint8_t enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
        uint8_t enemy_player_idx = enemies_0[enemy_idx];
        enemy_total += player_armies[enemy_player_idx][land_idx];
      }
      if (enemy_total != enemy_units_count[land_idx]) {
        printf("enemy_total %d != enemy_units_count[land_idx] %d", enemy_total,
               enemy_units_count[land_idx]);
      }
    }
    for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
      if (player_navies[player_idx][sea_idx] < 0 || player_navies[player_idx][sea_idx] > 240) {
        printf("units_sea_player_total[player_idx][sea_idx] < 0");
      }
      for (int unit_idx = 0; unit_idx < SEA_UNIT_TYPES_COUNT; unit_idx++) {
        if (player_sea_unit_types[player_idx][sea_idx][unit_idx] < 0 ||
            player_sea_unit_types[player_idx][sea_idx][unit_idx] > 240) {
          printf("total_player_sea_unit_types[player_idx][sea_idx][unit_idx] < 0");
        }
      }
      if (enemy_units_count[sea_idx + LANDS_COUNT] < 0 ||
          enemy_units_count[sea_idx + LANDS_COUNT] > 240) {
        printf("enemy_units_count[land_idx] < 0");
      }
      int total = 0;
      int total_data = 0;
      for (int unit_idx = 0; unit_idx < SEA_UNIT_TYPES_COUNT; unit_idx++) {
        if (player_idx == 0 || unit_idx < SEA_UNIT_TYPES_COUNT - 1)
          total += player_sea_unit_types[player_idx][sea_idx][unit_idx];
        if (player_idx == 0) {
          total_data += my_sea_unit_types[sea_idx][unit_idx];
        } else {
          if (unit_idx < SEA_UNIT_TYPES_COUNT - 1)
            total_data += state.other_sea_units[player_idx - 1][sea_idx][unit_idx];
        }
      }
      if (total != player_navies[player_idx][sea_idx] || total != total_data) {
        printf("total %d != total_player_sea_units[player_idx][sea_idx] %d != total_data %d", total,
               player_navies[player_idx][sea_idx], total_data);
      }
      int enemy_total = 0;
      for (uint8_t enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
        uint8_t enemy_player_idx = enemies_0[enemy_idx];
        enemy_total += player_navies[enemy_player_idx][sea_idx];
      }
      if (enemy_total != enemy_units_count[sea_idx + LANDS_COUNT]) {
        printf("enemy_total %d != enemy_units_count[sea_idx + LANDS_COUNT] %d", enemy_total,
               enemy_units_count[sea_idx + LANDS_COUNT]);
      }
    }
  }
#endif
}
*/

inline Dollars get_money(PlayerIndex player_idx) { return state.money[player_idx]; }

void setPrintableStatus() {
  debug_checks();
  char threeCharStr[4];
  printableGameStatus[0] = '\0';

  strcat(printableGameStatus, "---\n");
  setPrintableStatusLands();
  setPrintableStatusSeas();
  strcat(printableGameStatus, "\n");
  strcat(printableGameStatus, PLAYERS[state.player_index].color);
  strcat(printableGameStatus, PLAYERS[state.player_index].name);
  strcat(printableGameStatus, "\033[0m");
  strcat(printableGameStatus, ": ");
  sprintf(threeCharStr, "%d", get_money(0));
  strcat(printableGameStatus, threeCharStr);
  strcat(printableGameStatus, " IPC\n");
}

inline Player get_player(PlayerIndex player_idx) {
  return PLAYERS[(player_idx + state.player_index) % PLAYERS_COUNT];
}
inline char* get_player_color(PlayerIndex player_idx) { return get_player(player_idx).color; }
inline char* get_player_name(PlayerIndex player_idx) { return get_player(player_idx).name; }

inline int8_t get_factory_hp(LandIndex land_idx) { return *factory_hp[land_idx]; }

inline bool is_flagged_for_combat(LandIndex land_idx) { return state.flagged_for_combat[land_idx]; }

inline LandUnitStateSums* get_land_unit_state_sums(LandUnitStates* land_unit_states, LandUnitType unit_type) {
  return (*land_unit_states)[unit_type];
}

inline LandUnitStateSum get_land_unit_state_sum_at(LandUnitStateSums* landUnitStateSums,
                                                   GenericLandUnitState unit_state) {
  return (*landUnitStateSums)[unit_state];
}

void setPrintableStatusLands() {
  char threeCharStr[6];
  char paddedStr[32];
  char* my_color = PLAYERS[state.player_index].color;
  char* my_name = PLAYERS[state.player_index].name;
  for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    LandUnitStates* land_unit_states = get_my_land_unit_states(land_idx);
    //    LandState land_state = gameData.land_state[i];
    PlayerIndex land_owner = get_land_owner(land_idx);
    strcat(printableGameStatus, get_player_color(land_owner));
    sprintf(threeCharStr, "%d ", land_idx);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, get_land_name(land_idx));
    strcat(printableGameStatus, ": ");
    strcat(printableGameStatus, get_player_name(land_owner));
    strcat(printableGameStatus, " ");
    sprintf(threeCharStr, "%d", state.builds_left[land_idx]);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, "/");
    sprintf(threeCharStr, "%d", get_factory_hp(land_idx));
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, "/");
    sprintf(threeCharStr, "%d", get_factory_max(land_idx));
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, "/");
    sprintf(threeCharStr, "%d", get_land_value(land_idx));
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, " Combat:");
    if (is_flagged_for_combat(land_idx)) {
      strcat(printableGameStatus, "true\n");
    } else {
      strcat(printableGameStatus, "false\n");
    }
    int all_players_armies = 0;
    for (uint8_t player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
      all_players_armies += get_player_armies(player_idx, land_idx);
    }
    if (all_players_armies == 0) {
      strcat(printableGameStatus, "\033[0m");
      continue;
    }
    strcat(printableGameStatus, "                 |Tot| 0| 1| 2| 3| 4| 5| 6|\n");
    if (get_player_armies(0, land_idx) > 0) {
      strcat(printableGameStatus, my_color);
      LandUnitSumArray* land_units = get_my_land_unit_types(land_idx);
      for (LandUnitType unit_type = 0; unit_type < LAND_UNIT_TYPES_COUNT; unit_type++) {
        LandUnitSum unit_sum = get_land_unit_sum(land_units, unit_type);
        if (unit_sum > 0) {
          strcat(printableGameStatus, my_name);
          strcat(printableGameStatus, " ");
          sprintf(paddedStr, "%-14s", NAMES_UNIT_LAND[unit_type]);
          strcat(printableGameStatus, paddedStr);
          sprintf(threeCharStr, "%3d", unit_sum);
          strcat(printableGameStatus, threeCharStr);
          LandUnitStateSums* landUnitStateSums = get_land_unit_state_sums(land_unit_states, unit_type);
          for (int unit_state = 0; unit_state < STATES_MOVE_LAND[unit_type]; unit_state++) {
            sprintf(threeCharStr, "%3d", get_land_unit_state_sum_at(landUnitStateSums, unit_state));
            strcat(printableGameStatus, threeCharStr);
          }
          strcat(printableGameStatus, "\n");
        }
      }
    }
    strcat(printableGameStatus, "\033[0m");
    for (int player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      if (player_armies[player_idx][land_idx] == 0)
        continue;
      int current_player_idx = (player_idx + player_idx) % PLAYERS_COUNT;
      strcat(printableGameStatus, PLAYERS[current_player_idx].color);
      uint8_t* other_land_units_here = player_land_unit_types[player_idx][land_idx];
      for (int land_unit_idx = 0; land_unit_idx < LAND_UNIT_TYPES_COUNT; land_unit_idx++) {
        uint8_t unit_count = other_land_units_here[land_unit_idx];
        if (unit_count > 0) {
          strcat(printableGameStatus, PLAYERS[current_player_idx].name);
          strcat(printableGameStatus, " ");
          sprintf(paddedStr, "%-14s", NAMES_UNIT_LAND[land_unit_idx]);
          strcat(printableGameStatus, paddedStr);
          sprintf(threeCharStr, "%3d", unit_count);
          strcat(printableGameStatus, threeCharStr);
          strcat(printableGameStatus, "\n");
        }
      }
      strcat(printableGameStatus, "\033[0m");
    }
    strcat(printableGameStatus, "\n");
  }
}
void setPrintableStatusSeas() {
  char threeCharStr[4];
  char paddedStr[32];
  for (int sea_index = 0; sea_index < SEAS_COUNT; sea_index++) {
    int grand_total = 0;
    for (uint8_t player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
      grand_total += player_navies[player_idx][sea_index];
    }
    if (grand_total == 0) {
      continue;
    }
    SeaTerr units_sea = state.sea_terr[sea_index];
    sprintf(threeCharStr, "%d ", LANDS_COUNT + sea_index);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, SEAS[sea_index].name);
    strcat(printableGameStatus, " Combat:");
    if (state.flagged_for_combat[sea_index + LANDS_COUNT]) {
      strcat(printableGameStatus, "true\n");
    } else {
      strcat(printableGameStatus, "false\n");
    }
    strcat(printableGameStatus, "                 |Tot| 0| 1| 2| 3| 4| 5| 6|\n");
    strcat(printableGameStatus, PLAYERS[state.player_index].color);
    if (player_navies[0][sea_index] > 0) {
      for (int sea_unit_idx = 0; sea_unit_idx < SEA_UNIT_TYPES_COUNT; sea_unit_idx++) {
        uint8_t unit_count = my_sea_unit_types[sea_index][sea_unit_idx];
        if (unit_count > 0) {
          strcat(printableGameStatus, PLAYERS[state.player_index].name);
          strcat(printableGameStatus, " ");
          sprintf(paddedStr, "%-14s", NAMES_UNIT_SEA[sea_unit_idx]);
          strcat(printableGameStatus, paddedStr);
          sprintf(threeCharStr, "%3d", unit_count);
          strcat(printableGameStatus, threeCharStr);
          for (int cur_state = 0; cur_state < STATES_MOVE_SEA[sea_unit_idx]; cur_state++) {
            sprintf(threeCharStr, "%3d", sea_units_state[sea_index][sea_unit_idx][cur_state]);
            strcat(printableGameStatus, threeCharStr);
          }
          strcat(printableGameStatus, "\n");
        }
      }
    }
    strcat(printableGameStatus, "\033[0m");
    for (int player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      if (player_navies[player_idx][sea_index] == 0)
        continue;
      strcat(printableGameStatus, PLAYERS[(state.player_index + player_idx) % PLAYERS_COUNT].color);
      for (int sea_unit_idx = 0; sea_unit_idx < SEA_UNIT_TYPES_COUNT - 1; sea_unit_idx++) {
        uint8_t unit_count = player_sea_unit_types[player_idx][sea_index][sea_unit_idx];
        if (unit_count > 0) {
          strcat(printableGameStatus,
                 PLAYERS[(state.player_index + player_idx) % PLAYERS_COUNT].name);
          strcat(printableGameStatus, " ");
          sprintf(paddedStr, "%-14s", NAMES_UNIT_SEA[sea_unit_idx]);
          strcat(printableGameStatus, paddedStr);
          sprintf(threeCharStr, "%3d", unit_count);
          strcat(printableGameStatus, threeCharStr);
          strcat(printableGameStatus, "\n");
        }
      }
      strcat(printableGameStatus, "\033[0m");
    }
  }
}
uint8_t getUserInput() {
  char buffer[4]; // Buffer to hold input string (3 digits + null terminator)
  int user_input;

  while (true) {
    // 0-valid_moves_count
    // printf("Enter a number between 0 and 255: ");
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
      if (sscanf(buffer, "%d", &user_input) == 1 && user_input >= 0) {
        for (int i = 0; i < valid_actions_count; i++) {
          if (user_input == valid_actions[i]) {
            return (uint8_t)user_input;
          }
        }
        return valid_actions[0];
      }
    }
  }
}
uint8_t getAIInput() {
  answers_remaining--;
#ifdef DEBUG
  printf("selecting random action %d\n", RANDOM_NUMBERS[random_number_index]);
#endif
  return valid_actions[RANDOM_NUMBERS[random_number_index++] % valid_actions_count];
}

void add_valid_land_move_if_history_allows_1(uint8_t dst_air, uint8_t src_land) {
  // get a list of all of the source territories that moved a unit into a territory that I can
  // also reach for each source territory, get a list of possible territory moves that were
  // purposefully skipped these skipped territories are also invalid moves
  bool has_checked_territory[AIRS_COUNT] = {0};
  AirIndex shared_dst;
  AirIndex src_air;
  AirIndexCount hist_source_territories_count_shared_dst;
  LandIndexCount land_to_land_count = get_land_to_land_count(src_land);
  AirIndex* hist_source_territories_shared_dst;
  LandIndex* land_to_land_conn = get_land_to_land_conn(src_land);
  for (LandIndex land_idx = 0; land_idx < land_to_land_count; land_idx++) {
    shared_dst = land_to_land_conn[land_idx];
    hist_source_territories_count_shared_dst = hist_source_territories_count[shared_dst];
    hist_source_territories_shared_dst = hist_source_territories[shared_dst];
    for (int i = 0; i < hist_source_territories_count_shared_dst; i++) {
      src_air = hist_source_territories_shared_dst[i];
      if (has_checked_territory[src_air]) {
        continue;
      }
      has_checked_territory[src_air] = true;
      if (hist_skipped_airs[src_air][dst_air]) {
        return;
      }
    }
  }
  SeaIndex* land_to_sea_conn = get_land_to_sea_conn(src_land);
  SeaIndexCount land_to_sea_count = get_land_to_sea_count(src_land);
  for (SeaConnectionIndex sea_idx = 0; sea_idx < land_to_sea_count; sea_idx++) {
    shared_dst = convert_sea_to_air(land_to_sea_conn[sea_idx]);
    hist_source_territories_count_shared_dst = hist_source_territories_count[shared_dst];
    hist_source_territories_shared_dst = hist_source_territories[shared_dst];
    for (int i = 0; i < hist_source_territories_count_shared_dst; i++) {
      src_air = hist_source_territories_shared_dst[i];
      if (has_checked_territory[src_air]) {
        continue;
      }
      has_checked_territory[src_air] = true;
      if (hist_skipped_airs[src_air][dst_air]) {
        return;
      }
    }
  }
  valid_actions[valid_actions_count++] = dst_air;
}

void add_valid_land_move_if_history_allows_2(uint8_t dst_air, uint8_t src_land) {
  // get a list of all of the source territories that moved a unit into a territory that I can
  // also reach for each source territory, get a list of possible territory moves that were
  // purposefully skipped these skipped territories are also invalid moves
  bool has_checked_territory[AIRS_COUNT] = {0};
  AirIndex shared_dst;
  AirIndex src_air;
  AirIndexCount hist_source_territories_count_shared_dst;
  LandIndexCount lands_within_2_moves_count = get_lands_within_2_moves_count(src_land);
  LandIndexArray lands_within_2_moves = get_lands_within_2_moves(src_land);

  for (int land_idx = 0; land_idx < lands_within_2_moves_count; land_idx++) {
    shared_dst = lands_within_2_moves[land_idx];
    hist_source_territories_count_shared_dst = hist_source_territories_count[shared_dst];
    int* hist_source_territories_shared_dst = hist_source_territories[shared_dst];
    for (int i = 0; i < hist_source_territories_count_shared_dst; i++) {
      src_air = hist_source_territories[shared_dst][i];
      if (has_checked_territory[src_air]) {
        continue;
      }
      has_checked_territory[src_air] = true;
      if (hist_skipped_airs[src_air][dst_air] && !is_land_path_blocked[src_land][shared_dst]) {
        return;
      }
    }
  }
  int load_within_2_moves_count = LOAD_WITHIN_2_MOVES_COUNT[src_land];
  uint8_t* load_within_2_moves = LOAD_WITHIN_2_MOVES[src_land];
  for (int sea_idx = 0; sea_idx < load_within_2_moves_count; sea_idx++) {
    shared_dst = load_within_2_moves[sea_idx] + LANDS_COUNT;
    hist_source_territories_count_shared_dst = hist_source_territories_count[shared_dst];
    int* hist_source_territories_shared_dst = hist_source_territories[shared_dst];
    for (int i = 0; i < hist_source_territories_count_shared_dst; i++) {
      src_air = hist_source_territories_shared_dst[i];
      if (has_checked_territory[src_air]) {
        continue;
      }
      has_checked_territory[src_air] = true;
      if (hist_skipped_airs[src_air][dst_air] && !is_land_path_blocked[src_land][shared_dst]) {
        return;
      }
    }
  }
  valid_actions[valid_actions_count++] = dst_air;
}

void add_valid_sea_move_if_history_allows_1(uint8_t dst_sea, uint8_t src_sea) {
  // get a list of all of the source territories that moved a unit into a territory that I can
  // also reach for each source territory, get a list of possible territory moves that were
  // purposefully skipped these skipped territories are also invalid moves
  bool has_checked_air[AIRS_COUNT] = {0};
  int seas_within_1_move_count = SEAS_WITHIN_1_MOVE_COUNT[canal_state][src_sea];
  uint8_t* seas_within_1_move = SEAS_WITHIN_1_MOVE[canal_state][src_sea];
  for (int sea_idx = 0; sea_idx < seas_within_1_move_count; sea_idx++) {
    uint8_t shared_dst = seas_within_1_move[sea_idx];
    int hist_source_territories_count_shared_dst = hist_source_territories_count[shared_dst];
    int* hist_source_territories_shared_dst = hist_source_territories[shared_dst];
    for (int i = 0; i < hist_source_territories_count_shared_dst; i++) {
      uint8_t src_air = hist_source_territories_shared_dst[i];
      if (has_checked_air[src_air]) {
        continue;
      }
      has_checked_air[src_air] = true;
      if (hist_skipped_airs[src_air][dst_sea + LANDS_COUNT]) {
        return;
      }
    }
  }
  valid_actions[valid_actions_count++] = dst_sea + LANDS_COUNT;
}

void add_valid_sea_move_if_history_allows_2(uint8_t dst_sea, uint8_t src_sea) {
  // get a list of all of the source territories that moved a unit into a territory that I can
  // also reach for each source territory, get a list of possible territory moves that were
  // purposefully skipped these skipped territories are also invalid moves
  bool has_checked_territory[AIRS_COUNT] = {0};
  int seas_within_2_moves_count = SEAS_WITHIN_2_MOVES_COUNT[canal_state][src_sea];
  uint8_t* seas_within_2_moves = SEAS_WITHIN_2_MOVES[canal_state][src_sea];
  for (int sea_idx = 0; sea_idx < seas_within_2_moves_count; sea_idx++) {
    uint8_t shared_dst = seas_within_2_moves[sea_idx];
    int hist_source_territories_count_shared_dst = hist_source_territories_count[shared_dst];
    int* hist_source_territories_shared_dst = hist_source_territories[shared_dst];
    for (int i = 0; i < hist_source_territories_count_shared_dst; i++) {
      uint8_t src_air = hist_source_territories_shared_dst[i];
      if (has_checked_territory[src_air]) {
        continue;
      }
      has_checked_territory[src_air] = true;
      if (hist_skipped_airs[src_air][dst_sea + LANDS_COUNT]) {
        if (!is_sea_path_blocked[src_sea][dst_sea])
          return;
      }
    }
  }
  valid_actions[valid_actions_count++] = dst_sea + LANDS_COUNT;
}

void add_valid_sub_move_if_history_allows_2(uint8_t dst_sea, uint8_t src_sea) {
  // get a list of all of the source territories that moved a unit into a territory that I can
  // also reach for each source territory, get a list of possible territory moves that were
  // purposefully skipped these skipped territories are also invalid moves
  bool has_checked_territory[AIRS_COUNT] = {0};
  int seas_within_2_moves_count = SEAS_WITHIN_2_MOVES_COUNT[canal_state][src_sea];
  uint8_t* seas_within_2_moves = SEAS_WITHIN_2_MOVES[canal_state][src_sea];
  for (int sea_idx = 0; sea_idx < seas_within_2_moves_count; sea_idx++) {
    uint8_t shared_dst = seas_within_2_moves[sea_idx];
    int hist_source_territories_count_shared_dst = hist_source_territories_count[shared_dst];
    int* hist_source_territories_shared_dst = hist_source_territories[shared_dst];
    for (int i = 0; i < hist_source_territories_count_shared_dst; i++) {
      uint8_t src_air = hist_source_territories_shared_dst[i];
      if (has_checked_territory[src_air]) {
        continue;
      }
      has_checked_territory[src_air] = true;
      if (hist_skipped_airs[src_air][dst_sea + LANDS_COUNT]) {
        if (!is_sub_path_blocked[src_sea][dst_sea])
          return;
      }
    }
  }
  valid_actions[valid_actions_count++] = dst_sea + LANDS_COUNT;
}

void add_valid_air_move_if_history_allows_X(uint8_t dst_air, uint8_t src_air, uint8_t moves) {
  // get a list of all of the source territories that moved a unit into a territory that I can
  // also reach for each source territory, get a list of possible territory moves that were
  // purposefully skipped these skipped territories are also invalid moves
  bool has_checked_territory[AIRS_COUNT] = {0};
  int air_within_x_moves_count = AIR_WITHIN_X_MOVES_COUNT[moves][src_air];
  uint8_t* air_within_x_moves = AIR_WITHIN_X_MOVES[moves][src_air];
  for (int land_idx = 0; land_idx < air_within_x_moves_count; land_idx++) {
    uint8_t shared_dst = air_within_x_moves[land_idx];
    int hist_source_territories_count_shared_dst = hist_source_territories_count[shared_dst];
    int* hist_source_territories_shared_dst = hist_source_territories[shared_dst];
    for (int i = 0; i < hist_source_territories_count_shared_dst; i++) {
      uint8_t src_air = hist_source_territories_shared_dst[i];
      if (has_checked_territory[src_air]) {
        continue;
      }
      has_checked_territory[src_air] = true;
      if (hist_skipped_airs[src_air][dst_air]) {
        return;
      }
    }
  }
  valid_actions[valid_actions_count++] = dst_air;
}

void clear_move_history() {
  memset(hist_skipped_airs, 0, sizeof(hist_skipped_airs));
  memset(hist_source_territories_count, 0, sizeof(hist_source_territories_count));
}

uint8_t get_user_purchase_input(uint8_t src_air) {
  uint8_t user_input;
  if (PLAYERS[state.player_index].is_human) {
    char stringBuffer[32];
    setPrintableStatus();
    strcat(printableGameStatus, "Purchasing at ");
    if (src_air < LANDS_COUNT) {
      strcat(printableGameStatus, LANDS[src_air].name);
      sprintf(stringBuffer, " %d=Finished ", valid_actions[0]);
      strcat(printableGameStatus, stringBuffer);
      for (int i = 1; i < valid_actions_count; i++) {
        sprintf(stringBuffer, "%d=%s ", valid_actions[i], NAMES_UNIT_LAND[valid_actions[i]]);
        strcat(printableGameStatus, stringBuffer);
      }
    } else {
      strcat(printableGameStatus, SEAS[src_air - LANDS_COUNT].name);
      sprintf(stringBuffer, " %d=Finished ", valid_actions[0]);
      strcat(printableGameStatus, stringBuffer);
      for (int i = 1; i < valid_actions_count; i++) {
        sprintf(stringBuffer, "%d=%s ", valid_actions[i], NAMES_UNIT_SEA[valid_actions[i]]);
        strcat(printableGameStatus, stringBuffer);
      }
    }
    printf("%s\n", printableGameStatus);
    return getUserInput();
  }
  return getAIInput();
}

uint8_t get_user_move_input(uint8_t unit_type, uint8_t src_air) {
  uint8_t user_input;
  if (PLAYERS[state.player_index].is_human) {
    setPrintableStatus();
    strcat(printableGameStatus, "Moving ");
    if (src_air < LANDS_COUNT) {
      strcat(printableGameStatus, NAMES_UNIT_LAND[unit_type]);
      strcat(printableGameStatus, " From: ");
      strcat(printableGameStatus, LANDS[src_air].name);
    } else {
      strcat(printableGameStatus, NAMES_UNIT_SEA[unit_type]);
      strcat(printableGameStatus, " From: ");
      strcat(printableGameStatus, SEAS[src_air - LANDS_COUNT].name);
    }
    strcat(printableGameStatus, " Valid Moves: ");
    for (int i = 0; i < valid_actions_count; i++) {
      char threeCharStr[6];
      sprintf(threeCharStr, "%d ", valid_actions[i]);
      strcat(printableGameStatus, threeCharStr);
    }
    printf("%s\n", printableGameStatus);
    return getUserInput();
  }
  return getAIInput();
}

void update_move_history(uint8_t user_input, uint8_t src_air) {
  // hist_source_territories is a list of terrirtories that moved a unit into a specific territory
  // format is:
  //  hist_source_territories[dst_air][0] = src_air
  //  hist_source_territories[dst_air][1] = src_air2
  if (hist_source_territories[user_input][hist_source_territories_count[user_input]] == src_air)
    return;
  hist_source_territories[user_input][hist_source_territories_count[user_input]] = src_air;
  hist_source_territories_count[user_input]++;
  for (int move_index = valid_actions_count - 1; move_index >= 0; move_index--) {
    uint8_t move = valid_actions[move_index];
    if (move == user_input) {
      break;
    } else {
      hist_skipped_airs[src_air][move] = true;
      valid_actions_count -= 1;
    }
  }
}

bool load_transport(uint8_t unit_type, uint8_t src_land, uint8_t dst_sea, uint8_t land_unit_state) {
#ifdef DEBUG
  printf("load_transport: unit_type=%d src_land=%d dst_sea=%d land_unit_state=%d\n", unit_type,
         src_land, dst_sea, land_unit_state);
#endif
  const uint8_t* load_unit_type = LOAD_UNIT_TYPE[unit_type];
  uint8_t** units_sea_ptr_dst_sea = sea_units_state[dst_sea];
  for (uint8_t trans_type = (UNIT_WEIGHTS[unit_type] > 2) ? TRANS_1I : TRANS_1T;
       trans_type >= TRANS_EMPTY; trans_type--) {
    uint8_t* units_sea_ptr_dst_sea_trans_type = units_sea_ptr_dst_sea[trans_type];
    uint8_t states_unloading = STATES_UNLOADING[trans_type];
    for (uint8_t trans_state = STATES_MOVE_SEA[trans_type] - STATES_STAGING[trans_type];
         trans_state >= states_unloading && trans_state != 255;
         trans_state--) { // TODO - fix more underflows
      if (units_sea_ptr_dst_sea_trans_type[trans_state] > 0) {
        uint8_t new_trans_type = load_unit_type[trans_type];
        units_sea_ptr_dst_sea[trans_type][trans_state]--;
        if (trans_type == TRANS_EMPTY && trans_state == TRANS_EMPTY_UNLOADING_STATES) {
          trans_state = STATES_UNLOADING[new_trans_type]; // empty transports doesn't have an
                                                          // "unloading" state
        }
        units_sea_ptr_dst_sea[new_trans_type][trans_state]++;
        my_sea_unit_types[dst_sea][trans_type]--;
        my_sea_unit_types[dst_sea][new_trans_type]++;
        player_armies[0][src_land]--;
        my_land_unit_types[src_land][unit_type]--;
        land_units_state[src_land][unit_type][land_unit_state]--;
        transports_with_large_cargo_space[dst_sea] =
            my_sea_unit_types[dst_sea][TRANS_EMPTY] + my_sea_unit_types[dst_sea][TRANS_1I];
        transports_with_small_cargo_space[dst_sea] = transports_with_large_cargo_space[dst_sea] +
                                                     my_sea_unit_types[dst_sea][TRANS_1A] +
                                                     my_sea_unit_types[dst_sea][TRANS_1T];
        debug_checks();
        return true;
      }
    }
  }
  printf("Error: Failed to load transport\n");
  return false;
}

void add_valid_land_moves(uint8_t src_land, uint8_t moves_remaining, uint8_t unit_type) {
  if (moves_remaining == 2) {
    // check for moving from land to land (two moves away)
    uint8_t lands_within_2_moves_count = LANDS_WITHIN_2_MOVES_COUNT[src_land];
    uint8_t* lands_within_2_moves = LANDS_WITHIN_2_MOVES[src_land];
    uint8_t* land_dist = LAND_DIST[src_land];
    bool* is_land_path_blocked_src_land = is_land_path_blocked[src_land];
    for (int land_idx = 0; land_idx < lands_within_2_moves_count; land_idx++) {
      uint8_t dst_land = lands_within_2_moves[land_idx];
      if (land_dist[dst_land] == 1) {
        add_valid_land_move_if_history_allows_2(dst_land, src_land);
        continue;
      }
      if (is_land_path_blocked_src_land[dst_land]) {
        continue;
      }
      add_valid_land_move_if_history_allows_2(dst_land, src_land);
    }
    // check for moving from land to sea (two moves away)
    uint8_t load_within_2_moves_count = LOAD_WITHIN_2_MOVES_COUNT[src_land];
    uint8_t* load_within_2_moves = LOAD_WITHIN_2_MOVES[src_land];
    for (int sea_idx = 0; sea_idx < load_within_2_moves_count; sea_idx++) {
      uint8_t dst_sea = load_within_2_moves[sea_idx];
      if (transports_with_large_cargo_space[dst_sea] == 0) { // assume large, only tanks move 2
        continue;
      }
      uint8_t dst_air = dst_sea + LANDS_COUNT;
      if (land_dist[dst_air] == 1) {
        add_valid_land_move_if_history_allows_2(dst_air, src_land);
        continue;
      }
      if (is_land_path_blocked_src_land[dst_air]) {
        continue;
      }
      add_valid_land_move_if_history_allows_2(dst_air, src_land);
    }
  } else {
    // check for moving from land to land (one move away)
    uint8_t lands_within_1_move_count = LANDS_WITHIN_1_MOVE_COUNT[src_land];
    uint8_t* lands_within_1_move = LANDS_WITHIN_1_MOVE[src_land];
    bool is_non_combat_unit = ATTACK_UNIT_LAND[unit_type] == 0;
    bool is_unloadable_unit = UNIT_WEIGHTS[unit_type] > 5;
    bool is_heavy_unit = UNIT_WEIGHTS[unit_type] > 2;
    for (int land_idx = 0; land_idx < lands_within_1_move_count; land_idx++) {
      uint8_t dst_land = lands_within_1_move[land_idx];
      if (is_non_combat_unit && !is_allied_0[*owner_idx[dst_land]]) {
        continue;
      }
      add_valid_land_move_if_history_allows_1(dst_land, src_land);
    }
    // check for moving from land to sea (one move away)
    if (is_unloadable_unit)
      return;
    uint8_t land_to_sea_count = LAND_TO_SEA_COUNT[src_land];
    uint8_t* land_to_sea_conn = LAND_TO_SEA_CONN[src_land];
    for (int sea_idx = 0; sea_idx < land_to_sea_count; sea_idx++) {
      uint8_t dst_sea = land_to_sea_conn[sea_idx];
      if (transports_with_small_cargo_space[dst_sea] == 0) {
        continue;
      }
      if (is_heavy_unit && transports_with_large_cargo_space[dst_sea] == 0) {
        continue;
      }
      add_valid_land_move_if_history_allows_1(dst_sea + LANDS_COUNT, src_land);
    }
  }
}

void add_valid_sea_moves(uint8_t src_sea, uint8_t moves_remaining) {
  if (moves_remaining == 2) {
    // check for moving from sea to sea (two moves away)
    uint8_t seas_within_2_moves_count = SEAS_WITHIN_2_MOVES_COUNT[canal_state][src_sea];
    uint8_t* seas_within_2_moves =
        SEAS_WITHIN_2_MOVES[canal_state][src_sea]; // TODO optimize canal_state
    bool* is_sea_path_blocked_src_sea = is_sea_path_blocked[src_sea];
    for (uint8_t sea_idx = 0; sea_idx < seas_within_2_moves_count; sea_idx++) {
      uint8_t dst_sea = seas_within_2_moves[sea_idx];
      if (is_sea_path_blocked_src_sea[dst_sea]) {
        continue;
      }
      add_valid_sea_move_if_history_allows_2(dst_sea, src_sea);
    }
  } else {
    // check for moving from sea to sea (one move away)
    uint8_t seas_within_1_move_count = SEAS_WITHIN_1_MOVE_COUNT[canal_state][src_sea];
    uint8_t* seas_within_1_move =
        SEAS_WITHIN_1_MOVE[canal_state][src_sea]; // TODO optimize canal_state
    for (uint8_t sea_idx = 0; sea_idx < seas_within_1_move_count; sea_idx++) {
      uint8_t dst_sea = seas_within_1_move[sea_idx];
      add_valid_sea_move_if_history_allows_1(dst_sea, src_sea);
    }
  }
}

void add_valid_sub_moves(uint8_t src_sea, uint8_t moves_remaining) {
  if (moves_remaining == 2) {
    // check for moving from sea to sea (two moves away)
    uint8_t seas_within_2_moves_count = SEAS_WITHIN_2_MOVES_COUNT[canal_state][src_sea];
    uint8_t* seas_within_2_moves =
        SEAS_WITHIN_2_MOVES[canal_state][src_sea]; // TODO optimize canal_state
    bool* is_sub_path_blocked_src_sea = is_sub_path_blocked[src_sea];
    for (uint8_t sea_idx = 0; sea_idx < seas_within_2_moves_count; sea_idx++) {
      uint8_t dst_sea = seas_within_2_moves[sea_idx];
      if (is_sub_path_blocked_src_sea[dst_sea]) {
        continue;
      }
      add_valid_sub_move_if_history_allows_2(dst_sea, src_sea);
    }
  } else {
    // check for moving from sea to sea (one move away)
    uint8_t seas_within_1_move_count = SEAS_WITHIN_1_MOVE_COUNT[canal_state][src_sea];
    uint8_t* seas_within_1_move =
        SEAS_WITHIN_1_MOVE[canal_state][src_sea]; // TODO optimize canal_state
    for (uint8_t sea_idx = 0; sea_idx < seas_within_1_move_count; sea_idx++) {
      uint8_t dst_sea = seas_within_1_move[sea_idx];
      add_valid_sea_move_if_history_allows_1(dst_sea, src_sea);
    }
  }
}

bool stage_transport_units() {
  // loop through transports with "3" moves remaining (that aren't full),
  // start at sea 0 to n
  // TODO: optimize with cache - only loop through regions with transports
  for (uint8_t unit_type = TRANS_EMPTY; unit_type <= TRANS_1T; unit_type++) {
    uint8_t staging_state = STATES_MOVE_SEA[unit_type] - 1;
    uint8_t done_staging = staging_state - 1;
    clear_move_history();
    // TODO CHECKPOINT
    for (uint8_t src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      uint8_t* total_ships = &sea_units_state[src_sea][unit_type][staging_state];
      if (*total_ships == 0)
        continue;
      uint8_t src_air = src_sea + LANDS_COUNT;
      valid_actions[0] = src_air;
      valid_actions_count = 1;
      add_valid_sea_moves(src_sea, 2);
      uint8_t* units_sea_ptr_src_sea_unit_type = sea_units_state[src_sea][unit_type];
      while (*total_ships > 0) {
        uint8_t dst_air;
        if (valid_actions_count == 1) {
          dst_air = valid_actions[0];
        } else {
          if (answers_remaining == 0)
            return true;
          dst_air = get_user_move_input(unit_type, src_air);
        }
#ifdef DEBUG
        printf("stage_transport_units: unit_type=%d src_air=%d dst_air=%d\n", unit_type, src_air,
               dst_air);
        setPrintableStatus();
        printf("%s\n", printableGameStatus);
#endif
        update_move_history(dst_air, src_sea);
        if (src_air == dst_air) {
          units_sea_ptr_src_sea_unit_type[done_staging] += *total_ships;
          *total_ships = 0;
          continue;
        }
        uint8_t dst_sea = dst_air - LANDS_COUNT;
        uint8_t sea_distance = sea_dist[src_sea][dst_air];
        if (enemy_blockade[dst_sea] > 0) {
          state.flagged_for_combat[dst_air] = true;
          sea_distance = MAX_MOVE_SEA[unit_type];
          continue;
        }
        sea_units_state[dst_sea][unit_type][done_staging - sea_distance]++;
        my_sea_unit_types[dst_sea][unit_type]++;
        player_navies[0][dst_sea]++;
        transports_with_small_cargo_space[dst_sea]++;
        sea_units_state[src_sea][unit_type][staging_state]--;
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

void pre_move_fighter_units() {
#ifdef DEBUG
  setPrintableStatus();
  printf("%s\n", printableGameStatus);
  if (enemy_units_count[2] == -1) {
    printf("DEBUG: enemy_units_count[2] == -1\n");
  }
#endif
  clear_move_history();
  // refresh_canFighterLandHere
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    int land_owner = *owner_idx[land_idx];
    // is allied owned and not recently conquered?
    canFighterLandHere[land_idx] = (is_allied_0[land_owner] && !state.flagged_for_combat[land_idx]);
    // check for possiblity to build carrier under fighter
    if (land_owner == state.player_index && *factory_max[land_idx] > 0) {
      int land_to_sea_count = LAND_TO_SEA_COUNT[land_idx];
      uint8_t* land_to_sea_conn = LAND_TO_SEA_CONN[land_idx];
      for (int conn_idx = 0; conn_idx < land_to_sea_count; conn_idx++) {
        canFighterLandHere[LANDS_COUNT + land_to_sea_conn[conn_idx]] = true;
      }
    }
  }
  for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    if (allied_carriers[sea_idx] > 0) {
      canFighterLandHere[sea_idx] = true;
      // if player owns these carriers, then landing area is 2 spaces away
      if (sea_units_state[sea_idx][CARRIERS][CARRIER_MOVES_MAX] > 0) {
        uint8_t* sea_to_sea_conn = SEA_TO_SEA_CONN[sea_idx];
        int sea_to_sea_count = SEA_TO_SEA_COUNT[sea_idx];
        for (int conn_idx = 0; conn_idx < sea_to_sea_count; conn_idx++) {
          uint8_t connected_sea1 = sea_to_sea_conn[conn_idx];
          canFighterLandHere[LANDS_COUNT + connected_sea1] = true;
          int sea_to_sea_count2 = SEA_TO_SEA_COUNT[connected_sea1];
          uint8_t* sea_to_sea_conn2 = SEA_TO_SEA_CONN[connected_sea1];
          for (int conn2_idx = 0; conn2_idx < sea_to_sea_count2; conn2_idx++) {
            canFighterLandHere[LANDS_COUNT + sea_to_sea_conn2[conn2_idx]] = true;
          }
        }
      }
    }
  }
  // refresh_canFighterLandIn1Move
  for (int air_idx = 0; air_idx < AIRS_COUNT; air_idx++) {
    canFighterLandIn1Move[air_idx] = false;
    int air_conn_count = AIR_CONN_COUNT[air_idx];
    for (int conn_idx = 0; conn_idx < air_conn_count; conn_idx++) {
      if (canFighterLandHere[AIR_CONNECTIONS[air_idx][conn_idx]]) {
        canFighterLandIn1Move[air_idx] = true;
        break;
      }
    }
  }
}
bool move_fighter_units() {
  pre_move_fighter_units();
  for (int src_air = 0; src_air < AIRS_COUNT; src_air++) {
    uint8_t* total_fighters = &air_units_state[src_air][FIGHTERS_AIR][FIGHTER_MOVES_MAX];
    if (*total_fighters == 0) {
      continue;
    }
    valid_actions[0] = src_air;
    valid_actions_count = 1;
    add_valid_fighter_moves(src_air, FIGHTER_MOVES_MAX);
    while (*total_fighters > 0) {
      uint8_t dst_air;
      if (valid_actions_count == 1) {
        dst_air = valid_actions[0];
      } else {
        if (answers_remaining == 0)
          return true;
        dst_air = get_user_move_input(FIGHTERS_AIR, src_air);
      }
#ifdef DEBUG
      setPrintableStatus();
      printf("%s\n", printableGameStatus);
      printf("DEBUG: player: %s moving fighters %d, src_air: %d, dst_air: %d\n",
             PLAYERS[state.player_index].name, FIGHTERS_AIR, src_air, dst_air);
#endif
      update_move_history(dst_air, src_air);
      if (src_air == dst_air) {
        air_units_state[src_air][FIGHTERS_AIR][0] += *total_fighters;
        *total_fighters = 0;
        continue;
      }
      uint8_t airDistance = AIR_DIST[src_air][dst_air];
      if (dst_air < LANDS_COUNT) {
        if (!is_allied_0[*owner_idx[dst_air]]) {
#ifdef DEBUG
          printf("Fighter moving to enemy territory. Automatically flagging for combat");
#endif
          state.flagged_for_combat[dst_air] =
              true; // assuming enemy units are present based on valid moves
        } else {
          airDistance = 4; // use up all moves if this is a friendly rebase
        }
      } else {
#ifdef DEBUG
        printf("Fighter moving to sea. Possibly flagging for combat");
#endif
        state.flagged_for_combat[dst_air] = enemy_units_count[dst_air] > 0;
      }
      if (dst_air < LANDS_COUNT) {
        land_units_state[dst_air][FIGHTERS_LAND][FIGHTER_MOVES_MAX - airDistance]++;
        my_land_unit_types[dst_air][FIGHTERS_LAND]++;
        player_armies[0][dst_air]++;
      } else {
        uint8_t dst_sea = dst_air - LANDS_COUNT;
        sea_units_state[dst_sea][FIGHTERS_LAND][FIGHTER_MOVES_MAX - airDistance]++;
        my_sea_unit_types[dst_sea][FIGHTERS_LAND]++;
        player_navies[0][dst_sea]++;
      }
      if (src_air < LANDS_COUNT) {
        my_land_unit_types[src_air][FIGHTERS_LAND]--;
        player_armies[0][src_air]--;
      } else {
        uint8_t src_sea = src_air - LANDS_COUNT;
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
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    // is allied owned and not recently conquered?
    canBomberLandHere[land_idx] =
        (is_allied_0[*owner_idx[land_idx]] && !state.flagged_for_combat[land_idx]);
  }
  //  refresh_canBomberLandIn1Move
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    canBomberLandIn1Move[land_idx] = false;
    uint8_t land_conn_count = LANDS[land_idx].land_conn_count;
    const uint8_t* connected_land_index = LANDS[land_idx].connected_land_index;
    for (int conn_idx = 0; conn_idx < land_conn_count; conn_idx++) {
      if (canBomberLandHere[connected_land_index[conn_idx]]) {
        canBomberLandIn1Move[land_idx] = true;
        break;
      }
    }
  }
  for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    canBomberLandIn1Move[LANDS_COUNT + sea_idx] = false;
    int land_conn_count = SEAS[sea_idx].land_conn_count;
    const uint8_t* connected_land_index = SEAS[sea_idx].connected_land_index;
    for (int conn_idx = 0; conn_idx < land_conn_count; conn_idx++) {
      if (canBomberLandHere[connected_land_index[conn_idx]]) {
        canBomberLandIn1Move[LANDS_COUNT + sea_idx] = true;
        break;
      }
    }
  }
  // refresh_canBomberLandIn2Moves
  for (int air_idx = 0; air_idx < AIRS_COUNT; air_idx++) {
    canBomberLandIn2Moves[air_idx] = false;
    int air_conn_count = AIR_CONN_COUNT[air_idx];
    uint8_t* air_conn = AIR_CONNECTIONS[air_idx];
    for (int conn_idx = 0; conn_idx < air_conn_count; conn_idx++) {
      if (canBomberLandIn1Move[air_conn[conn_idx]]) {
        canBomberLandIn2Moves[air_idx] = true;
        break;
      }
    }
  }
  for (int src_land = 0; src_land < LANDS_COUNT; src_land++) {
    uint8_t* total_bombers = &land_units_state[src_land][BOMBERS_LAND][BOMBER_MOVES_MAX];
    if (*total_bombers == 0) {
      continue;
    }
    valid_actions[0] = src_land;
    valid_actions_count = 1;
    add_valid_bomber_moves(src_land, BOMBER_MOVES_MAX);
    while (*total_bombers > 0) {
      uint8_t dst_air;
      if (valid_actions_count == 1) {
        dst_air = valid_actions[0];
      } else {
        if (answers_remaining == 0)
          return true;
        dst_air = getAIInput();
#ifdef DEBUG
        dst_air = get_user_move_input(BOMBERS_LAND, src_land);
#endif
      }
#ifdef DEBUG
      printf("DEBUG: player: %s bombers fighters %d, src_air: %d, dst_air: %d\n",
             PLAYERS[state.player_index].name, BOMBERS_LAND, src_land, dst_air);
#endif
      update_move_history(dst_air, src_land);
      if (src_land == dst_air) {
        land_units_state[src_land][BOMBERS_LAND][0] += *total_bombers;
        *total_bombers = 0;
        continue;
      }
      if (dst_air < LANDS_COUNT) {
        if (!is_allied_0[*owner_idx[dst_air]]) {
#ifdef DEBUG
          printf("Bomber moving to enemy territory. Automatically flagging for combat");
#endif
          state.flagged_for_combat[dst_air] =
              true; // assuming enemy units are present based on valid moves
        }
      } else {
#ifdef DEBUG
        printf("Bomber moving to sea. Possibly flagging for combat");
#endif
        uint8_t dst_sea = dst_air - LANDS_COUNT;
        state.flagged_for_combat[dst_air] = enemy_units_count[dst_air] > 0;
      }
      uint8_t airDistance = AIR_DIST[src_land][dst_air];
      if (dst_air < LANDS_COUNT) {
        land_units_state[dst_air][BOMBERS_LAND][BOMBER_MOVES_MAX - airDistance]++;
        player_land_unit_types[0][dst_air][BOMBERS_LAND]++;
        player_armies[0][dst_air]++;
      } else {
        uint8_t dst_sea = dst_air - LANDS_COUNT;
        sea_units_state[dst_sea][BOMBERS_SEA][BOMBER_MOVES_MAX - 1 - airDistance]++;
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

void conquer_land(uint8_t dst_land) {
#ifdef DEBUG
  setPrintableStatus();
  printf("%s\n", printableGameStatus);
  printf("conquer_land: dst_land=%d\n", dst_land);
#endif
  uint8_t old_owner_id = *owner_idx[dst_land];
  if (PLAYERS[(state.player_index + old_owner_id) % PLAYERS_COUNT].capital_territory_index ==
      dst_land) {
    state.money[0] += state.money[old_owner_id];
    state.money[old_owner_id] = 0;
  }
  income_per_turn[old_owner_id] -= LAND_VALUE[dst_land];
  uint8_t new_owner_id = 0;
  uint8_t orig_owner_id =
      (LANDS[dst_land].original_owner_index + PLAYERS_COUNT - state.player_index) % PLAYERS_COUNT;
  if (is_allied_0[orig_owner_id]) {
    new_owner_id = orig_owner_id;
  }
#ifdef DEBUG
  printf("conquer_land: old_owner_id=%d new_owner_id=%d orig_owner_id=%d\n", old_owner_id,
         new_owner_id, orig_owner_id);
#endif
  *owner_idx[dst_land] = new_owner_id;
  income_per_turn[new_owner_id] += LAND_VALUE[dst_land];
  factory_locations[new_owner_id][factory_count[new_owner_id]++] = dst_land;
  factory_count[old_owner_id]--;
  for (int i = 0; i < factory_count[orig_owner_id]; i++) {
    if (factory_locations[old_owner_id][i] == dst_land) {
#ifdef DEBUG
      printf("DEBUG: Found factory at %s\n", LANDS[dst_land].name);
#endif
      for (int j = i; j < factory_count[old_owner_id]; j++) {
#ifdef DEBUG
        printf("DEBUG: Moving factory at %s\n", LANDS[factory_locations[old_owner_id][j]].name);
#endif
        factory_locations[old_owner_id][j] = factory_locations[old_owner_id][j + 1];
      }
      break;
    }
  }
#ifdef DEBUG
  setPrintableStatus();
  printf("%s\n", printableGameStatus);
  for (uint8_t player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    for (uint8_t factory_index = 0; factory_index < factory_count[player_idx]; factory_index++) {
      uint8_t factory_location = factory_locations[player_idx][factory_index];
      if (*owner_idx[factory_location] != player_idx) {
        printf("DEBUG: Player %s has a unowned factory at %s\n", PLAYERS[state.player_index].name,
               LANDS[factory_location].name);
      }
    }
  }
#endif
}

bool move_land_unit_type(uint8_t unit_type) {
  clear_move_history();
  for (int src_land = 0; src_land < LANDS_COUNT; src_land++) {
    for (int moves_remaining = MAX_MOVE_LAND[unit_type]; moves_remaining > 0; moves_remaining--) {
      uint8_t* total_units = &land_units_state[src_land][unit_type][moves_remaining];
      if (*total_units == 0) {
        continue;
      }
      valid_actions[0] = src_land;
      valid_actions_count = 1;
      add_valid_land_moves(src_land, moves_remaining, unit_type);
      while (*total_units > 0) {
        uint8_t dst_air;
        if (valid_actions_count == 1) {
          dst_air = valid_actions[0];
        } else {
          if (answers_remaining == 0)
            return true;
          dst_air = get_user_move_input(unit_type, src_land);
        }
#ifdef DEBUG
        setPrintableStatus();
        printf("%s\n", printableGameStatus);
        printf("DEBUG: player: %s moving land unit %d, src_air: %d, dst_air: %d enemy count: %d\n",
               PLAYERS[state.player_index].name, unit_type, src_land, dst_air,
               enemy_units_count[dst_air]);
#endif
        update_move_history(dst_air, src_land);
        if (src_land == dst_air) {
          land_units_state[src_land][unit_type][0] += *total_units;
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
        state.flagged_for_combat[dst_air] = enemy_units_count[dst_air] > 0;
        // if the destination is not blitzable, then end unit turn
        uint8_t landDistance = LAND_DIST[src_land][dst_air];
        if (is_allied_0[*owner_idx[dst_air]] || enemy_units_count[dst_air] > 0) {
          landDistance = moves_remaining;
        }
        land_units_state[dst_air][unit_type][moves_remaining - landDistance]++;
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
          state.flagged_for_combat[dst_air] = true;
        }
        debug_checks();
      }
    }
  }
  return false;
}

bool move_transport_units() {
  for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    for (int cur_state = TRANS_EMPTY_STATES - TRANS_EMPTY_STAGING_STATES;
         cur_state >= TRANS_EMPTY_UNLOADING_STATES + 1; cur_state--) {
      uint8_t* total_ships = &sea_units_state[src_sea][TRANS_EMPTY][cur_state];
      if (*total_ships == 0)
        continue;
      sea_units_state[src_sea][TRANS_EMPTY][0] += *total_ships;
      *total_ships = 0;
    }
  }
  for (int unit_type = TRANS_1I; unit_type <= TRANS_1I_1T;
       unit_type++) { // there should be no TRANS_EMPTY
    int max_state = STATES_MOVE_SEA[unit_type] - STATES_STAGING[unit_type];
    int done_moving = STATES_UNLOADING[unit_type];
    int min_state = STATES_UNLOADING[unit_type] + 1;
    clear_move_history();
    // TODO CHECKPOINT
    for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      for (int cur_state = max_state; cur_state >= min_state; cur_state--) {
        uint8_t* total_ships = &sea_units_state[src_sea][unit_type][cur_state];
        if (*total_ships == 0)
          continue;
        int moves_remaining = cur_state - STATES_UNLOADING[unit_type];
        uint8_t src_air = src_sea + LANDS_COUNT;
        valid_actions[0] = src_air;
        valid_actions_count = 1;
        add_valid_sea_moves(src_sea, moves_remaining);
        while (*total_ships > 0) {
          uint8_t dst_air;
          if (valid_actions_count == 1) {
            dst_air = valid_actions[0];
          } else {
            if (answers_remaining == 0)
              return true;
            dst_air = get_user_move_input(unit_type, src_air);
          }
#ifdef DEBUG
          printf("DEBUG: moving transport units unit_type: %d, src_air: %d, dst_air: %d\n",
                 unit_type, src_air, dst_air);
#endif
          update_move_history(dst_air, src_air);
          uint8_t dst_sea = dst_air - LANDS_COUNT;
          if (enemy_blockade[dst_sea] > 0) {
#ifdef DEBUG
            printf("Enemy units detected, flagging for combat");
#endif
            state.flagged_for_combat[dst_air] = true;
          }
          if (src_air == dst_air) {
            sea_units_state[src_sea][unit_type][done_moving] += *total_ships;
            *total_ships = 0;
            continue;
          }
          sea_units_state[dst_sea][unit_type][done_moving]++;
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
    uint8_t* total_subs = &sea_units_state[src_sea][SUBMARINES][SUB_UNMOVED];
    if (*total_subs == 0)
      continue;
    uint8_t src_air = src_sea + LANDS_COUNT;
    valid_actions[0] = src_air;
    valid_actions_count = 1;
    add_valid_sub_moves(src_sea, SUB_MOVES_MAX);
    while (*total_subs > 0) {
      uint8_t dst_air;
      if (valid_actions_count == 1) {
        dst_air = valid_actions[0];
      } else {
        if (answers_remaining == 0)
          return true;
        dst_air = get_user_move_input(SUBMARINES, src_air);
      }
#ifdef DEBUG
      setPrintableStatus();
      printf("%s\n", printableGameStatus);
      printf("DEBUG: moving sub units unit_type: %d, src_air: %d, dst_air: %d\n", SUBMARINES,
             src_air, dst_air);
#endif
      update_move_history(dst_air, src_air);
      uint8_t dst_sea = dst_air - LANDS_COUNT;
      if (enemy_units_count[dst_sea] > 0) {
#ifdef DEBUG
        printf("Submarine moving to where enemy units are present, flagging for combat");
#endif
        state.flagged_for_combat[dst_sea] = true;
        // break;
      }
      if (src_air == dst_air) {
        sea_units_state[src_sea][SUBMARINES][SUB_DONE_MOVING] += *total_subs;
        *total_subs = 0;
        continue;
      }
      sea_units_state[dst_sea][SUBMARINES][SUB_DONE_MOVING]++;
      my_sea_unit_types[dst_sea][SUBMARINES]++;
      player_navies[0][dst_sea]++;
      *total_subs -= 1;
      my_sea_unit_types[src_sea][SUBMARINES]--;
      player_navies[0][src_sea]--;
    }
  }
  return false;
}

bool move_destroyers_battleships() {
  for (int unit_type = DESTROYERS; unit_type <= BS_DAMAGED; unit_type++) {
    uint8_t unmoved = UNMOVED_SEA[unit_type];
    uint8_t done_moving = DONE_MOVING_SEA[unit_type];
    uint8_t moves_remaining = MAX_MOVE_SEA[unit_type];
    // TODO CHECKPOINT
    clear_move_history();
    for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      uint8_t* total_ships = &sea_units_state[src_sea][unit_type][unmoved];
      if (*total_ships == 0)
        continue;
      uint8_t src_air = src_sea + LANDS_COUNT;
      valid_actions[0] = src_air;
      valid_actions_count = 1;
      add_valid_sea_moves(src_sea, moves_remaining);
      while (*total_ships > 0) {
        uint8_t dst_air;
        if (valid_actions_count == 1) {
          dst_air = valid_actions[0];
        } else {
          if (answers_remaining == 0)
            return true;
          dst_air = get_user_move_input(unit_type, src_air);
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
          state.flagged_for_combat[dst_air] = true;
          // break;
        }
        if (src_air == dst_air) {
          sea_units_state[src_sea][unit_type][done_moving] += *total_ships;
          *total_ships = 0;
          continue;
        }
        uint8_t dst_sea = dst_air - LANDS_COUNT;
        sea_units_state[dst_sea][unit_type][done_moving]++;
        my_sea_unit_types[dst_sea][unit_type]++;
        player_navies[0][dst_sea]++;
        *total_ships -= 1;
        my_sea_unit_types[src_sea][unit_type]--;
        player_navies[0][src_sea]--;
        if (unit_type == CARRIERS)
          carry_allied_fighters(src_sea, dst_sea);
        debug_checks();
      }
    }
  }
  return false;
}

void carry_allied_fighters(uint8_t src_sea, uint8_t dst_sea) {
#ifdef DEBUG
  printf("DEBUG: carry_allied_fighters: src_sea: %d, dst_sea: %d\n", src_sea, dst_sea);
  setPrintableStatus();
  printf("%s\n", printableGameStatus);
#endif
  int allied_fighters_moved = 0;
  for (int other_player_idx = 1; other_player_idx < PLAYERS_COUNT; other_player_idx++) {
    while (player_sea_unit_types[other_player_idx][src_sea][FIGHTERS_SEA] > 0) {
      player_sea_unit_types[other_player_idx][src_sea][FIGHTERS_SEA]--;
      player_navies[other_player_idx][src_sea]--;
      player_sea_unit_types[other_player_idx][dst_sea][FIGHTERS_SEA]++;
      player_navies[other_player_idx][dst_sea]++;
      if (allied_fighters_moved == 1)
        return;
      allied_fighters_moved++;
    }
  }
}

bool resolve_sea_battles() {
  for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    // check if battle is over (e.g. untargetable subs/air/trans or zero units)
    // target options:
    // 1. attacker has air, defender has std ships or air
    // 2. attacker has any ships, defender has any non-transports
    // 3. defender has air, attacker has std ships or air
    // 4. defender has any ships, attacker has any non-transports
    uint8_t src_air = src_sea + LANDS_COUNT;
    // if not flagged for combat, continue
    if (!state.flagged_for_combat[src_air]) {
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
      uint8_t total_enemy_subs = 0;
      for (uint8_t enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
        total_enemy_subs += player_sea_unit_types[enemies_0[enemy_idx]][src_sea][SUBMARINES];
      }
      if (total_enemy_subs == enemy_units_count[src_sea]) {
        continue;
      }
    }
    // combat is proceeding, so disable bombardment capabilities of ships
    for (uint8_t unit_type = CRUISERS; unit_type <= BS_DAMAGED; unit_type++) {
      sea_units_state[src_sea][unit_type][0] += sea_units_state[src_sea][unit_type][1];
      sea_units_state[src_sea][unit_type][1] = 0;
    }

    while (true) {
#ifdef DEBUG
      setPrintableStatus();
      printf("%s\n", printableGameStatus);
      if (my_land_unit_types[2][FIGHTERS_LAND] == 255) {
        printf("DEBUG: units_land_ptr[0][FIGHTERS][0] == 255\n");
      }
      if (enemy_units_count[2] == -1) {
        printf("DEBUG: enemy_units_count[2] == -1\n");
      }
#endif
      uint8_t* units11 = &sea_units_state[src_sea][DESTROYERS][0];
      bool targets_exist = false;
      if (my_sea_unit_types[src_sea][DESTROYERS]) {
        if (enemy_units_count[src_air] > 0) {
          targets_exist = true;
        }
      } else if (my_sea_unit_types[src_sea][CARRIERS] + my_sea_unit_types[src_sea][CRUISERS] +
                     my_sea_unit_types[src_sea][BATTLESHIPS] +
                     my_sea_unit_types[src_sea][BS_DAMAGED] >
                 0) {
        if (enemy_units_count[src_air] > 0) {
          for (int enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
            int enemy_player_idx = enemies_0[enemy_idx];
            if (player_navies[enemy_player_idx][src_sea] -
                    player_sea_unit_types[enemy_player_idx][src_sea][SUBMARINES] >
                0) {
              targets_exist = true;
              break;
            }
          }
        }
      } else if (my_sea_unit_types[src_sea][SUBMARINES] > 0) { // no blockade ships, only subs
        for (int enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
          int enemy_player_idx = enemies_0[enemy_idx];
          if (player_navies[enemy_player_idx][src_sea] -
                  (player_sea_unit_types[enemy_player_idx][src_sea][FIGHTERS_SEA] +
                   player_sea_unit_types[enemy_player_idx][src_sea][SUBMARINES]) >
              0) {
            targets_exist = true;
            break;
          }
        }
      } else if (my_sea_unit_types[src_sea][FIGHTERS_SEA] +
                     my_sea_unit_types[src_sea][BOMBERS_SEA] >
                 0) { // no ships, only air
        for (int enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
          int enemy_player_idx = enemies_0[enemy_idx];
          if (player_navies[enemy_player_idx][src_sea] -
                  player_sea_unit_types[enemy_player_idx][src_sea][SUBMARINES] >
              0) {
            targets_exist = true;
            break;
          }
        }
      }
      // TODO only allow hits to hit valid targets
      // untargetable battle
      if (!targets_exist) {
        // allied_carriers[src_sea] =
        //     data.units_sea[src_sea].carriers[0] + data.units_sea[src_sea].carriers[1];
        // for (int player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
        //   allied_carriers[src_sea] +=
        //       other_sea_units_ptr[player_idx][src_sea][CARRIERS] * is_allied_0[player_idx];
        // }
        state.flagged_for_combat[src_air] = false;
        break;
      }
      // fire subs (defender always submerges if possible)
      int attacker_damage = sea_units_state[src_sea][SUBMARINES][0] * SUB_ATTACK; // TODO FIX
      // int attacker_damage = current_player_sea_unit_types[src_sea][SUBMARINES] * SUB_ATTACK;
      // //TODO FIX
      uint8_t attacker_hits =
          (attacker_damage / 6) +
          (RANDOM_NUMBERS[random_number_index++] % 6 < attacker_damage % 6 ? 1 : 0);

      int defender_damage;
      int defender_hits;
      if (!defender_submerged) {
        defender_damage = 0; // TODO skip if no subs
        for (int enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
          defender_damage += player_sea_unit_types[enemies_0[enemy_idx]][src_sea][SUBMARINES];
        }
        defender_hits = (defender_damage / 6) +
                        (RANDOM_NUMBERS[random_number_index++] % 6 < defender_damage % 6 ? 1 : 0);
        if (defender_hits > 0)
          remove_sea_attackers(src_sea, defender_hits);
      }
      // remove casualties
      if (attacker_hits > 0)
        remove_sea_defenders(src_sea, attacker_hits, defender_submerged);
      // fire all ships and air for both sides
      attacker_damage = 0;
      for (uint8_t unit_type_idx = 0; unit_type_idx < BLOCKADE_UNIT_TYPES_COUNT; unit_type_idx++) {
        uint8_t unit_type = BLOCKADE_UNIT_TYPES[unit_type_idx];
        attacker_damage += my_sea_unit_types[src_sea][unit_type] * ATTACK_UNIT_SEA[unit_type];
      }
      attacker_damage += my_sea_unit_types[src_sea][FIGHTERS_SEA] * ATTACK_UNIT_SEA[FIGHTERS_SEA];
      attacker_damage += my_sea_unit_types[src_sea][BOMBERS_SEA] * ATTACK_UNIT_SEA[BOMBERS_SEA];
      attacker_hits = (attacker_damage / 6) +
                      (RANDOM_NUMBERS[random_number_index++] % 6 < attacker_damage % 6 ? 1 : 0);

      defender_damage = 0;
      for (uint8_t enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
        uint8_t* enemy_units = player_sea_unit_types[enemies_0[enemy_idx]][src_sea];
        for (uint8_t unit_type = 0; unit_type < BLOCKADE_UNIT_TYPES_COUNT; unit_type++) {
          defender_damage += enemy_units[unit_type] * DEFENSE_UNIT_SEA[unit_type];
        }
        defender_damage += enemy_units[FIGHTERS_SEA] * DEFENSE_UNIT_SEA[FIGHTERS_SEA];
      }
      defender_hits = (defender_damage / 6) +
                      (RANDOM_NUMBERS[random_number_index++] % 6 < defender_damage % 6 ? 1 : 0);
      // remove casualties
      if (defender_hits > 0)
        remove_sea_attackers(src_sea, defender_hits);
      if (attacker_hits > 0)
        remove_sea_defenders(src_sea, attacker_hits, defender_submerged);

      if (enemy_units_count[src_air] == 0 || player_navies[0][src_sea] == 0) {
        state.flagged_for_combat[src_air] = false;
        break;
      }

      // ask to retreat (0-255, any non valid retreat zone is considered a no)
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
      uint8_t* sea_to_sea_conn = SEA_TO_SEA_CONN[src_sea];
      for (int sea_conn_idx = 0; sea_conn_idx < SEA_TO_SEA_COUNT[src_sea]; sea_conn_idx++) {
        uint8_t sea_dst = sea_to_sea_conn[sea_conn_idx];
        if (enemy_blockade_total[sea_dst] == 0)
          valid_actions[valid_actions_count++] = sea_dst + LANDS_COUNT;
      }
      if (valid_actions_count == 0)
        continue;
      uint8_t dst_air;
      if (valid_actions_count == 1) {
        dst_air = valid_actions[0];
      } else {
        if (answers_remaining == 0)
          return true;
        dst_air = ask_to_retreat();
      }
      // if retreat, move units to retreat zone immediately and end battle
      uint8_t dst_sea = dst_air - LANDS_COUNT;
      if (sea_dist[src_sea][dst_sea] == 1 && !state.flagged_for_combat[dst_air]) {
        sea_retreat(src_sea, dst_sea);
        break;
      }
      // loop
    }
  }
  return false;
}

void sea_retreat(uint8_t src_sea, uint8_t dst_sea) {
#ifdef DEBUG
  debug_checks();
  printf("DEBUG: retreating to sea: %d\n", dst_sea);
#endif
  for (uint8_t unit_type = TRANS_EMPTY; unit_type <= BS_DAMAGED; unit_type++) {
    sea_units_state[dst_sea][unit_type][0] += current_player_sea_unit_types[src_sea][unit_type];
    my_sea_unit_types[dst_sea][unit_type] += my_sea_unit_types[src_sea][unit_type];
    player_navies[0][dst_sea] += my_sea_unit_types[src_sea][unit_type];
    player_navies[0][src_sea] -= my_sea_unit_types[src_sea][unit_type];
    sea_units_state[src_sea][unit_type][STATES_UNLOADING[unit_type]] = 0;
    my_sea_unit_types[src_sea][unit_type] = 0;
  }
  state.flagged_for_combat[src_sea + LANDS_COUNT] = false;
  debug_checks();
}

uint8_t ask_to_retreat() {
  if (PLAYERS[state.player_index].is_human) {
    setPrintableStatus();
    strcat(printableGameStatus, "To where do you want to retreat (255 for no)? ");
    printf("%s\n", printableGameStatus);
    return getUserInput();
  }
  return getAIInput();
}
void remove_land_defenders(uint8_t src_land, uint8_t hits) {
  for (uint8_t unit_idx = 0; unit_idx < DEFENDER_LAND_UNIT_TYPES_COUNT; unit_idx++) {
    for (uint8_t enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
      uint8_t enemy_player_idx = enemies_0[enemy_idx];
      uint8_t* total_units = &total_player_land_unit_types[enemy_player_idx][src_land]
                                                          [ORDER_OF_LAND_DEFENDERS[unit_idx]];
      if (*total_units > 0) {
#ifdef DEBUG
        printf(
            "DEBUG: remove_land_defenders: src_land: %d, hits: %d, unit_idx: %d, enemy_idx: %d\n",
            src_land, hits, ORDER_OF_LAND_DEFENDERS[unit_idx], enemy_idx);
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
void remove_land_attackers(uint8_t src_land, uint8_t hits) {
  uint8_t* total_units;
  for (uint8_t unit_idx = 0; unit_idx < ATTACKER_LAND_UNIT_TYPES_COUNT_1; unit_idx++) {
    uint8_t unit_type = ORDER_OF_LAND_ATTACKERS_1[unit_idx];
    // TODO fix - why are AA guns with 1 move remaining here?
    total_units = &land_units_state[src_land][unit_type][0];
    if (*total_units > 0) {
#ifdef DEBUG
      printf("DEBUG: remove_land_attackers: unit_type: %d src_land: %d, hits: %d\n", unit_type,
             src_land, hits);
#endif
      if (*total_units < hits) {
        hits -= *total_units;
        player_armies[0][src_land] -= *total_units;
        *total_units = 0;
        player_land_unit_types[0][src_land][unit_type] = 0;
      } else {
        *total_units -= hits;
        player_land_unit_types[0][src_land][unit_type] -= hits;
        player_armies[0][src_land] -= hits;
        hits = 0;
        return;
      }
    }
  }
  for (uint8_t unit_idx = 0; unit_idx < ATTACKER_LAND_UNIT_TYPES_COUNT_2; unit_idx++) {
    uint8_t unit_type = ORDER_OF_LAND_ATTACKERS_2[unit_idx];
    if (player_land_unit_types[0][src_land][unit_type] == 0)
      continue;
    for (uint8_t cur_state = 1; cur_state < STATES_MOVE_LAND[unit_type] - 1; cur_state++) {
      total_units = &land_units_state[src_land][unit_type][cur_state];
      if (*total_units > 0) {
#ifdef DEBUG
        printf("DEBUG: remove_land_attackers: unit_type: %d src_land: %d, hits: %d\n", unit_type,
               src_land, hits);
#endif

        if (*total_units < hits) {
          hits -= *total_units;
          player_land_unit_types[0][src_land][unit_type] -= *total_units;
          player_armies[0][src_land] -= *total_units;
          *total_units = 0;
        } else {
          *total_units -= hits;
          player_land_unit_types[0][src_land][unit_type] -= hits;
          player_armies[0][src_land] -= hits;
          hits = 0;
          return;
        }
      }
    }
  }
}
void remove_sea_defenders(uint8_t src_sea, uint8_t hits, bool defender_submerged) {
#ifdef DEBUG
  printf("DEBUG: remove_sea_defenders: src_sea: %d, hits: %d\n", src_sea, hits);
#endif
  uint8_t src_air = src_sea + LANDS_COUNT;
  for (uint8_t enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
    uint8_t enemy_player_idx = enemies_0[enemy_idx];
    uint8_t* total_battleships = &player_sea_unit_types[enemy_player_idx][src_sea][BATTLESHIPS];
    uint8_t* total_bs_damaged = &player_sea_unit_types[enemy_player_idx][src_sea][BS_DAMAGED];
    if (*total_battleships > 0) {
#ifdef DEBUG
      printf("DEBUG: remove_sea_defenders: src_sea: %d, hits: %d, total_battleships: %d\n", src_sea,
             hits, *total_battleships);
#endif
      if (*total_battleships < hits) {
        hits -= *total_battleships;
        *total_bs_damaged += *total_battleships;
        *total_battleships = 0;
        debug_checks();
      } else {
        *total_bs_damaged += hits;
        *total_battleships -= hits;
        hits = 0;
        debug_checks();
        return;
      }
    }
  }
  if (!defender_submerged) {
    for (uint8_t enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
      uint8_t enemy_player_idx = enemies_0[enemy_idx];
      uint8_t* total_units = &player_sea_unit_types[enemy_player_idx][src_sea][SUBMARINES];
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
          debug_checks();
        } else {
          *total_units -= hits;
          player_navies[enemy_player_idx][src_sea] -= hits;
          enemy_units_count[src_air] -= hits;
          hits = 0;
          debug_checks();
          return;
        }
      }
    }
  }

  // skipping submarines
  for (uint8_t unit_idx = 1; unit_idx < DEFENDER_SEA_UNIT_TYPES_COUNT; unit_idx++) {
    for (uint8_t enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
      uint8_t enemy_player_idx = enemies_0[enemy_idx];
      uint8_t* total_units =
          &total_player_sea_unit_types[enemy_player_idx][src_sea][ORDER_OF_SEA_DEFENDERS[unit_idx]];
      if (*total_units > 0) {
#ifdef DEBUG
        printf(
            "DEBUG: remove_sea_defenders: unit_type: %d src_sea: %d, hits: %d, total_units: %d\n",
            ORDER_OF_SEA_DEFENDERS[unit_idx], src_sea, hits, *total_units);
#endif
        if (*total_units < hits) {
          hits -= *total_units;
          player_navies[enemy_player_idx][src_sea] -= *total_units;
          enemy_units_count[src_air] -= *total_units;
          if (unit_idx >= DESTROYERS && unit_idx <= BS_DAMAGED) {
            enemy_blockade[src_sea] -= *total_units;
          }
          *total_units = 0;
          debug_checks();
        } else {
          *total_units -= hits;
          player_navies[enemy_player_idx][src_sea] -= hits;
          enemy_units_count[src_air] -= hits;
          if (unit_idx >= DESTROYERS && unit_idx <= BS_DAMAGED) {
            enemy_blockade[src_sea] -= hits;
          }
          hits = 0;
          debug_checks();
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

void remove_sea_attackers(uint8_t src_sea, uint8_t hits) {
#ifdef DEBUG
  printf("DEBUG: remove_sea_attackers: src_sea: %d, hits: %d\n", src_sea, hits);
#endif
  uint8_t* total_battleships = &sea_units_state[src_sea][BATTLESHIPS][0];
  uint8_t* total_bs_damaged = &sea_units_state[src_sea][BS_DAMAGED][0];
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
  uint8_t* total_units;
  for (uint8_t unit_idx = 0; unit_idx < ATTACKER_SEA_UNIT_TYPES_COUNT_1; unit_idx++) {
    uint8_t unit_type = ORDER_OF_SEA_ATTACKERS_1[unit_idx];
    uint8_t* total_units = &sea_units_state[src_sea][unit_type][0];
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
  total_units = &sea_units_state[src_sea][CARRIERS][0];
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
      allied_carriers[src_sea] = 0;
    } else {
      *total_units -= hits;
      my_sea_unit_types[src_sea][CARRIERS] -= hits;
      allied_carriers[src_sea] -= hits;
      player_navies[0][src_sea] -= hits;
      hits = 0;
      return; // TODO: do allied fighters have a chance to land?
    }
  }
  total_units = &sea_units_state[src_sea][CRUISERS][0];
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
  for (uint8_t unit_idx = 0; unit_idx < ATTACKER_SEA_UNIT_TYPES_COUNT_2; unit_idx++) {
    uint8_t unit_type = ORDER_OF_SEA_ATTACKERS_2[unit_idx];
    if (player_land_unit_types[0][src_sea][unit_type] == 0)
      continue;
    for (uint8_t cur_state = 0; cur_state < STATES_MOVE_SEA[unit_type]; cur_state++) {
      total_units = &sea_units_state[src_sea][unit_type][cur_state];
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
  for (uint8_t unit_idx = 0; unit_idx < ATTACKER_SEA_UNIT_TYPES_COUNT_3; unit_idx++) {
    uint8_t unit_type = ORDER_OF_SEA_ATTACKERS_3[unit_idx];
    uint8_t* total_units = &sea_units_state[src_sea][unit_type][0];
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

bool unload_transports() {
  for (uint8_t unit_type = TRANS_1I; unit_type <= TRANS_1I_1T; unit_type++) {
    uint8_t unloading_state = STATES_UNLOADING[unit_type];
    uint8_t unload_cargo1 = UNLOAD_CARGO1[unit_type];
    uint8_t unload_cargo2 = UNLOAD_CARGO2[unit_type];
    clear_move_history();
    for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      uint8_t* total_units = &sea_units_state[src_sea][unit_type][unloading_state];
      if (*total_units == 0)
        continue;
      uint8_t src_air = src_sea + LANDS_COUNT;

      valid_actions[0] = src_air;
      valid_actions_count = 1;
      add_valid_unload_moves(src_sea);
      while (*total_units > 0) {
        uint8_t dst_air;
        if (valid_actions_count == 1) {
          dst_air = valid_actions[0];
        } else {
          if (answers_remaining == 0)
            return true;
          dst_air = get_user_move_input(unit_type, src_air);
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
          sea_units_state[src_sea][unit_type][0] += *total_units;
          *total_units = 0;
          continue;
        }
        bombard_max[dst_air]++;
        land_units_state[dst_air][unload_cargo1][0]++;
        my_land_unit_types[dst_air][unload_cargo1]++;
        player_armies[0][dst_air]++;
        sea_units_state[src_sea][TRANS_EMPTY][0]++;
        my_sea_unit_types[src_sea][TRANS_EMPTY]++;
        my_sea_unit_types[src_sea][unit_type]--;
        *total_units -= 1;
        if (unit_type > TRANS_1T) {
          bombard_max[dst_air]++;
          land_units_state[dst_air][unload_cargo2][0]++;
          my_land_unit_types[dst_air][unload_cargo2]++;
          player_armies[0][dst_air]++;
        }
        if (!is_allied_0[*owner_idx[dst_air]]) {
          state.flagged_for_combat[dst_air] = true;
          if (enemy_units_count[dst_air] == 0) {
            conquer_land(dst_air);
          }
        }
        debug_checks();
      }
    }
  }
  return false;
}
bool resolve_land_battles() {
  for (uint8_t src_land = 0; src_land < LANDS_COUNT; src_land++) {
    // check if battle is over
    if (!state.flagged_for_combat[src_land]) {
      continue;
    }
#ifdef DEBUG
    // debug print the current src_land and its name
    printf("Resolve land combat in: %d, Name: %s\n", src_land, LANDS[src_land].name);
    setPrintableStatus();
    printf("%s\n", printableGameStatus);
#endif
    int* units_land_player_total_0_src_land = &player_armies[0][src_land];
    // check if no friendlies remain
    if (*units_land_player_total_0_src_land == 0) {
#ifdef DEBUG
      printf("No friendlies remain");
#endif
      continue;
    }
    uint8_t attacker_damage;
    uint8_t attacker_hits;
    // only bombers exist
    uint8_t* other_land_units_ptr_0_src_land = player_land_unit_types[0][src_land];
    uint8_t* bombers_count = &other_land_units_ptr_0_src_land[BOMBERS_LAND];
    if (*bombers_count > 0 && player_armies[0][src_land] == *bombers_count) {
      if (*factory_hp[src_land] > -*factory_max[src_land]) {
#ifdef DEBUG
        printf("Strategic Bombing");
#endif
        // fire_strat_aa_guns();
        uint8_t defender_damage = *bombers_count;
        uint8_t defender_hits =
            (defender_damage / 6) +
            (RANDOM_NUMBERS[random_number_index++] % 6 < defender_damage % 6 ? 1 : 0);
        if (defender_hits > 0) {
          uint8_t* units_land_ptr_src_land_bombers = land_units_state[src_land][BOMBERS_LAND];
          for (uint8_t cur_state = 1; cur_state < BOMBER_LAND_STATES - 1; cur_state++) {
            uint8_t* total_units = &units_land_ptr_src_land_bombers[cur_state];
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
        attacker_damage = *bombers_count * 21;
        attacker_hits = (attacker_damage / 6) +
                        (RANDOM_NUMBERS[random_number_index++] % 6 < attacker_damage % 6 ? 1 : 0);
        *factory_hp[src_land] =
            fmax(*factory_hp[src_land] - attacker_hits, -*factory_max[src_land]);
        continue;
      }
    }
#ifdef DEBUG
    printf("Normal Land Combat");
#endif
    uint8_t* other_land_units_0_src_land = my_land_unit_types[src_land];
    // bombard_shores
    if (bombard_max[src_land] > 0) {
      attacker_damage = 0;
#ifdef DEBUG
      printf("Sea Bombardment");
#endif
      for (uint8_t unit_type = BS_DAMAGED; unit_type >= CRUISERS; unit_type--) {
        for (uint8_t sea_idx = 0; sea_idx < LAND_TO_SEA_COUNT[src_land]; sea_idx++) {
          uint8_t src_sea = LAND_TO_SEA_CONN[src_land][sea_idx];
          uint8_t* total_bombard_ships = sea_units_state[src_sea][unit_type];
          while (total_bombard_ships[1] > 0 && bombard_max[src_land] > 0) {
            attacker_damage += ATTACK_UNIT_SEA[unit_type];
            total_bombard_ships[0]++;
            total_bombard_ships[1]--;
            bombard_max[src_land]--;
          }
        }
      }
      bombard_max[src_land] = 0;
      attacker_hits = (attacker_damage / 6) +
                      (RANDOM_NUMBERS[random_number_index++] % 6 < attacker_damage % 6 ? 1 : 0);
      if (attacker_hits > 0) {
        remove_land_defenders(src_land, attacker_hits);
      }
    }
    // check if can fire tactical aa_guns
    uint8_t total_air_units = other_land_units_ptr_0_src_land[FIGHTERS_LAND] +
                              other_land_units_ptr_0_src_land[BOMBERS_LAND];
    uint8_t defender_damage;
    uint8_t defender_hits;
    if (total_air_units > 0) {
      int total_aa_guns = 0;
      for (uint8_t enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
        total_aa_guns += player_land_unit_types[enemies_0[enemy_idx]][src_land][AA_GUNS];
      }
      if (total_aa_guns > 0) {
        // fire_tact_aa_guns();
        defender_damage = total_air_units * 3;
        defender_hits = (defender_damage / 6) +
                        (RANDOM_NUMBERS[random_number_index++] % 6 < defender_damage % 6 ? 1 : 0);
        if (defender_hits > 0) {
          for (uint8_t cur_state = 0; cur_state < FIGHTER_STATES; cur_state++) {
            uint8_t* total_units = &land_units_state[src_land][FIGHTERS_LAND][cur_state];
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
          for (uint8_t cur_state = 0; cur_state < BOMBER_LAND_STATES; cur_state++) {
            uint8_t* total_units = &land_units_state[src_land][BOMBERS_LAND][cur_state];
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
        debug_checks();
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
      printf("Current land battle start src_land: %d, Name: %s", src_land, LANDS[src_land].name);
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
      int infantry_count = my_land_unit_types[src_land][INFANTRY];
      int artillery_count = my_land_unit_types[src_land][ARTILLERY];
      attacker_damage = (my_land_unit_types[src_land][FIGHTERS_LAND] * FIGHTER_ATTACK) +
                        (my_land_unit_types[src_land][BOMBERS_LAND] * BOMBER_ATTACK) +
                        (infantry_count * INFANTRY_ATTACK) + (artillery_count * ARTILLERY_ATTACK) +
                        (my_land_unit_types[src_land][TANKS] * TANK_ATTACK);
      // add damage for the minimum of count of infantry/artillery
      attacker_damage += infantry_count < artillery_count ? infantry_count : artillery_count;
      attacker_hits = (attacker_damage / 6) +
                      (RANDOM_NUMBERS[random_number_index++] % 6 < attacker_damage % 6 ? 1 : 0);
      defender_damage = 0;
#ifdef DEBUG
      printf("Enemy Count: %d\n", enemy_units_count[src_land]);
#endif
      for (uint8_t enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
        uint8_t* land_units = player_land_unit_types[enemies_0[enemy_idx]][src_land];
        defender_damage += (land_units[INFANTRY] * INFANTRY_DEFENSE) +
                           (land_units[ARTILLERY] * ARTILLERY_DEFENSE) +
                           (land_units[TANKS] * TANK_DEFENSE) +
                           (land_units[FIGHTERS_LAND] * FIGHTER_DEFENSE) +
                           (land_units[BOMBERS_LAND] * BOMBER_DEFENSE);
        defender_hits = (defender_damage / 6) +
                        (RANDOM_NUMBERS[random_number_index++] % 6 < defender_damage % 6 ? 1 : 0);
      }
      if (defender_hits > 0) {
#ifdef DEBUG
        printf("Defender Hits: %d", defender_hits);
#endif
        remove_land_attackers(src_land, defender_hits);
        debug_checks();
      }
      if (attacker_hits > 0) {
#ifdef DEBUG
        printf("Attacker Hits: %d", attacker_hits);
#endif
        remove_land_defenders(src_land, attacker_hits);
        debug_checks();
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
      uint8_t* land_to_land_conn = LAND_TO_LAND_CONN[src_land];
      uint8_t land_to_land_count = LAND_TO_LAND_COUNT[src_land];
      for (int land_conn_idx = 0; land_conn_idx < land_to_land_count; land_conn_idx++) {
        uint8_t land_dst = land_to_land_conn[land_conn_idx];
        if (enemy_units_count[land_dst] == 0 && !state.flagged_for_combat[land_dst] &&
            is_allied_0[*owner_idx[land_dst]])
          valid_actions[valid_actions_count++] = land_dst;
      }
      uint8_t dst_air;
      if (valid_actions_count == 1) {
        dst_air = valid_actions[0];
      } else {
        if (answers_remaining == 0)
          return true;
        dst_air = ask_to_retreat();
      }
      // if retreat, move units to retreat zone immediately and end battle
      if (src_land != dst_air) {
#ifdef DEBUG
        printf("Retreating land_battle from: %d to: %d\n", src_land, dst_air);
#endif
        for (uint8_t unit_type = INFANTRY; unit_type <= TANKS; unit_type++) {
          int total_units = land_units_state[src_land][unit_type][0];
          land_units_state[dst_air][unit_type][0] += total_units;
          my_land_unit_types[dst_air][unit_type] += total_units;
          player_armies[0][dst_air] += total_units;
          my_land_unit_types[src_land][unit_type] -= total_units;
          *units_land_player_total_0_src_land -= total_units;
          land_units_state[src_land][unit_type][0] = 0;
        }
        state.flagged_for_combat[src_land] = false;
        debug_checks();
        break;
      }
    }
  }
  return false;
}

void add_valid_unload_moves(uint8_t src_sea) {
  uint8_t* near_land = SEA_TO_LAND_CONN[src_sea];
  uint8_t near_land_count = SEA_TO_LAND_COUNT[src_sea];
  for (int land_idx = 0; land_idx < near_land_count; land_idx++) {
    uint8_t dst_land = near_land[land_idx];
    add_valid_air_move_if_history_allows_X(dst_land, src_sea + LANDS_COUNT, 1);
  }
}

void add_valid_fighter_moves(uint8_t src_air, uint8_t remaining_moves) {
  uint8_t* near_air = AIR_WITHIN_X_MOVES[remaining_moves - 1][src_air];
  uint8_t near_air_count = AIR_WITHIN_X_MOVES_COUNT[remaining_moves - 1][src_air];
  for (int i = 0; i < near_air_count; i++) {
    uint8_t dst_air = near_air[i];
    uint8_t air_dist = AIR_DIST[src_air][dst_air];
    if (air_dist <= 2 || canFighterLandHere[dst_air] ||
        (air_dist == 3 && canFighterLandIn1Move[dst_air])) {
      if (!canFighterLandHere[dst_air] && enemy_units_count[dst_air] == 0) // waste of a move
        continue;
      add_valid_air_move_if_history_allows_X(dst_air, src_air, air_dist);
    }
  }
}

void add_valid_fighter_landing(uint8_t src_air, uint8_t remaining_moves) {
  uint8_t* near_air = AIR_WITHIN_X_MOVES[remaining_moves - 1][src_air];
  uint8_t near_air_count = AIR_WITHIN_X_MOVES_COUNT[remaining_moves - 1][src_air];
  for (int i = 0; i < near_air_count; i++) {
    uint8_t dst_air = near_air[i];
    if (canFighterLandHere[dst_air]) {
      add_valid_air_move_if_history_allows_X(dst_air, src_air, remaining_moves);
    }
  }
}

void add_valid_bomber_moves(uint8_t src_air, uint8_t remaining_moves) {
  uint8_t* near_air = AIR_WITHIN_X_MOVES[remaining_moves - 1][src_air];
  uint8_t near_air_count = AIR_WITHIN_X_MOVES_COUNT[remaining_moves - 1][src_air];
  for (int i = 0; i < near_air_count; i++) {
    uint8_t dst_air = near_air[i];
    uint8_t air_dist = AIR_DIST[src_air][dst_air];
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

bool land_fighter_units() {
  //  refresh_canFighterLandHere_final
  for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    // if (allied_carriers[sea_idx] > 0) {
    canFighterLandHere[sea_idx + LANDS_COUNT] = allied_carriers[sea_idx] > 0;
    //}
  }
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    int land_owner = *owner_idx[land_idx];
    // is allied owned and not recently conquered?
    canFighterLandHere[land_idx] = is_allied_0[land_owner] && !state.flagged_for_combat[land_idx];
    // check for possiblity to build carrier under fighter
    if (*factory_max[land_idx] > 0 && land_owner == state.player_index) {
      int land_to_sea_count = LAND_TO_SEA_COUNT[land_idx];
      for (int conn_idx = 0; conn_idx < land_to_sea_count; conn_idx++) {
        canFighterLandHere[LANDS_COUNT + LAND_TO_SEA_CONN[land_idx][conn_idx]] = true;
      }
    }
  }
  //  check if any fighters have moves remaining
  for (uint8_t cur_state = 1; cur_state < FIGHTER_STATES - 1;
       cur_state++) { // TODO optimize to find next fighter faster
    clear_move_history();
    for (int src_air = 0; src_air < AIRS_COUNT; src_air++) {
      uint8_t* total_fighter_count = &air_units_state[src_air][FIGHTERS_AIR][cur_state];
      if (*total_fighter_count == 0)
        continue;

      // valid_moves[0] = src_air;
      valid_actions_count = 0;
      add_valid_fighter_landing(src_air, cur_state);
      if (valid_actions_count == 0 || canFighterLandHere[src_air]) {
        valid_actions[valid_actions_count++] = src_air;
      }
      while (*total_fighter_count > 0) {
        uint8_t dst_air;
        if (valid_actions_count == 1) {
          dst_air = valid_actions[0];
        } else {
          if (answers_remaining == 0)
            return true;
          dst_air = getAIInput();
#ifdef DEBUG
          dst_air = get_user_move_input(FIGHTERS_AIR, src_air);
#endif
        }
#ifdef DEBUG
        setPrintableStatus();
        printf("%s\n", printableGameStatus);
        printf("DEBUG: player: %s landing fighters %d, src_air: %d, dst_air: %d\n",
               PLAYERS[state.player_index].name, FIGHTERS_AIR, src_air, dst_air);
#endif
        update_move_history(dst_air, src_air);
        if (src_air == dst_air) {
          air_units_state[src_air][FIGHTERS_AIR][0]++;
          *total_fighter_count -= 1;
          continue;
        }
        air_units_state[dst_air][FIGHTERS_AIR][0]++;
        if (dst_air < LANDS_COUNT) {
          player_armies[0][dst_air]++;
          my_land_unit_types[dst_air][FIGHTERS_LAND]++;
        } else {
          uint8_t dst_sea = dst_air - LANDS_COUNT;
          player_navies[0][dst_sea]++;
          my_sea_unit_types[dst_sea][FIGHTERS_SEA]++;
        }
        if (src_air < LANDS_COUNT) {
          player_armies[0][src_air]--;
          my_land_unit_types[src_air][FIGHTERS_LAND]--;
        } else {
          uint8_t src_sea = src_air - LANDS_COUNT;
          player_navies[0][src_sea]--;
          my_sea_unit_types[src_sea][FIGHTERS_SEA]--;
        }
        *total_fighter_count -= 1;
        debug_checks();
      }
    }
  }
  return false;
}

void add_valid_bomber_landing(uint8_t src_air, uint8_t movement_remaining) {
#ifdef DEBUG
  printf("movement_remaining: %d\n", movement_remaining);
#endif
  uint8_t* near_land = AIR_TO_LAND_WITHIN_X_MOVES[movement_remaining - 1][src_air];
  for (int i = 0; i < AIR_TO_LAND_WITHIN_X_MOVES_COUNT[movement_remaining - 1][src_air]; i++) {
    uint8_t dst_air = near_land[i];
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
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    // is allied owned and not recently conquered?
    canBomberLandHere[land_idx] =
        is_allied_0[*owner_idx[land_idx]] && !state.flagged_for_combat[land_idx];
  }

  // check if any bombers have moves remaining
  for (uint8_t cur_state1 = 0; cur_state1 < BOMBER_LAND_STATES - 2;
       cur_state1++) { // TODO optimize to find next bomber faster
    clear_move_history();
    for (int src_air = 0; src_air < AIRS_COUNT; src_air++) {
      uint8_t cur_state = (src_air < LANDS_COUNT ? cur_state1 + 1 : cur_state1);
      uint8_t* total_bomber_count = &air_units_state[src_air][BOMBERS_AIR][cur_state];
      if (*total_bomber_count == 0)
        continue;
#ifdef DEBUG
      printf("Bomber Count: %d with state %d, in location %d\n", *total_bomber_count, cur_state,
             src_air);
#endif
      // valid_moves[0] = src_air;
      valid_actions_count = 0;
      uint8_t movement_remaining = cur_state + (src_air < LANDS_COUNT ? 0 : 1);
      add_valid_bomber_landing(src_air, movement_remaining);
      while (*total_bomber_count > 0) {
        if (valid_actions_count == 0) {
          valid_actions[valid_actions_count++] = src_air;
        }
        uint8_t dst_air;
        if (valid_actions_count == 1) {
          dst_air = valid_actions[0];
        } else {
          if (answers_remaining == 0)
            return true;
          dst_air = getAIInput();
#ifdef DEBUG
          dst_air = get_user_move_input(BOMBERS_AIR, src_air);
#endif
        }
#ifdef DEBUG
        setPrintableStatus();
        printf("%s\n", printableGameStatus);
        printf("valid_moves_count: %d\n", valid_actions_count);
        printf("valid_moves: ");
        for (int i = 0; i < valid_actions_count; i++) {
          printf("%d ", valid_actions[i]);
        }
        printf("\n");
        printf("DEBUG: player: %s landing bombers %d, src_air: %d, dst_air: %d\n",
               PLAYERS[state.player_index].name, BOMBERS_AIR, src_air, dst_air);
#endif
        update_move_history(dst_air, src_air);
        if (src_air == dst_air) {
          air_units_state[src_air][BOMBERS_AIR][0]++;
          *total_bomber_count -= 1;
          debug_checks();
          continue;
        }
        air_units_state[dst_air][BOMBERS_AIR][0]++;
        player_armies[0][dst_air]++;
        my_land_unit_types[dst_air][BOMBERS_LAND]++;
        if (src_air < LANDS_COUNT) {
          player_armies[0][src_air]--;
          my_land_unit_types[src_air][BOMBERS_LAND]--;
        } else {
          uint8_t src_sea = src_air - LANDS_COUNT;
          player_navies[0][src_sea]--;
          my_land_unit_types[src_sea][BOMBERS_LAND]--;
        }
        *total_bomber_count -= 1;
        debug_checks();
      }
      debug_checks();
    }
  }
  return false;
}
bool buy_units() {
#ifdef DEBUG
  setPrintableStatus();
  printf("%s\nBuying Units\n", printableGameStatus);
#endif
  for (uint8_t factory_idx = 0; factory_idx < factory_count[0]; factory_idx++) {
    uint8_t dst_land = factory_locations[0][factory_idx];
#ifdef DEBUG
    if (*owner_idx[dst_land] != 0) {
      printf("DEBUG: player: %s cannot buy units at %s\n", PLAYERS[state.player_index].name,
             LANDS[dst_land].name);
    }
#endif
    if (state.builds_left[dst_land] == 0) {
      continue;
    }
    uint8_t repair_cost = 0;
    // buy sea units
    for (uint8_t sea_idx = 0; sea_idx < LAND_TO_SEA_COUNT[dst_land]; sea_idx++) {
      uint8_t dst_sea = LAND_TO_SEA_CONN[dst_land][sea_idx];
      uint8_t dst_air = dst_sea + LANDS_COUNT;
      valid_actions[0] = SEA_UNIT_TYPES_COUNT; // pass all units
      uint8_t last_purchased = 0;
      while (state.builds_left[dst_air] > 0) {
        if (state.money[0] < TRANSPORT_COST) {
          state.builds_left[dst_air] = 0;
          break;
        }
        uint8_t units_built = *factory_max[dst_land] - state.builds_left[dst_land];
        if (*factory_hp[dst_land] <= units_built)
          repair_cost = 1 + units_built - *factory_hp[dst_land]; // subtracting a negative
        // add all units that can be bought
        valid_actions_count = 1;
        for (int unit_type_idx = COST_UNIT_SEA_COUNT - 1; unit_type_idx >= 0; unit_type_idx--) {
          uint8_t unit_type = BUY_UNIT_SEA[unit_type_idx];
          if (unit_type < last_purchased)
            break;
          if (state.money[0] < COST_UNIT_SEA[unit_type] + repair_cost)
            continue;
          if (unit_type == FIGHTERS_SEA) {
            int total_fighters = 0;
            for (uint8_t player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
              total_fighters += total_player_sea_unit_types[player_idx][dst_sea][FIGHTERS_SEA];
            }
            if (allied_carriers[dst_sea] * 2 <= total_fighters)
              continue;
          }
          valid_actions[valid_actions_count++] = unit_type;
        }
        if (valid_actions_count == 1) {
          state.builds_left[dst_air] = 0;
          break;
        }
        uint8_t purchase;
        if (answers_remaining == 0)
          return true;
        purchase = get_user_purchase_input(dst_air);
        if (purchase == SEA_UNIT_TYPES_COUNT) { // pass all units
          state.builds_left[dst_air] = 0;
          break;
        }
#ifdef DEBUG
        // print which player is buying which unit at which location
        printf("Player %d buying %s at %s\n", state.player_index, NAMES_UNIT_SEA[purchase],
               SEAS[dst_sea].name);
#endif
        for (uint8_t sea_idx2 = sea_idx; sea_idx2 < LAND_TO_SEA_COUNT[dst_land]; sea_idx2++) {
          state.builds_left[LAND_TO_SEA_CONN[dst_land][sea_idx2] + LANDS_COUNT]--;
        }
        state.builds_left[dst_land]--;
        *factory_hp[dst_land] += repair_cost;
        state.money[0] -= COST_UNIT_SEA[purchase] + repair_cost;
        sea_units_state[dst_sea][purchase][0]++;
        total_player_sea_units[0][dst_sea]++;
        current_player_sea_unit_types[dst_sea][purchase]++;
        last_purchased = purchase;
      }
    }
    // buy land units
    valid_actions[0] = LAND_UNIT_TYPES_COUNT; // pass all units
    uint8_t unit_type = 0;
    uint8_t last_purchased = 0;
    while (state.builds_left[dst_land] > 0) {
      if (state.money[0] < INFANTRY_COST) {
        state.builds_left[dst_land] = 0;
        break;
      }
      uint8_t units_built = *factory_max[dst_land] - state.builds_left[dst_land];
      if (*factory_hp[dst_land] <= units_built)
        repair_cost = 1 + units_built - *factory_hp[dst_land]; // subtracting a negative
      // add all units that can be bought
      valid_actions_count = 1;
      for (int unit_type = LAND_UNIT_TYPES_COUNT - 1; unit_type >= 0; unit_type--) {
        if (unit_type < last_purchased)
          break;
        if (state.money[0] < COST_UNIT_LAND[unit_type] + repair_cost)
          continue;
        valid_actions[valid_actions_count++] = unit_type;
      }
      if (valid_actions_count == 1) {
        state.builds_left[dst_land] = 0;
        break;
      }
      if (answers_remaining == 0)
        return true;
      uint8_t purchase = get_user_purchase_input(dst_land);
      if (purchase == LAND_UNIT_TYPES_COUNT) { // pass all units
        state.builds_left[dst_land] = 0;
        break;
      }
#ifdef DEBUG
      // print which player is buying which unit at which location
      printf("Player %d buying %s at %s\n", state.player_index, NAMES_UNIT_LAND[purchase],
             LANDS[dst_land].name);
#endif
      state.builds_left[dst_land]--;
      *factory_hp[dst_land] += repair_cost;
      state.money[0] -= COST_UNIT_LAND[purchase] + repair_cost;
      land_units_state[dst_land][purchase][0]++;
      player_armies[0][dst_land]++;
      player_land_unit_types[0][dst_land][purchase]++;
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
  for (uint8_t air_idx = 0; air_idx < LANDS_COUNT; air_idx++) {
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
    air_units_state[air_idx][FIGHTERS_AIR][0] = 0;
  }
  // crash planes not on allied carriers
  for (uint8_t air_idx = LANDS_COUNT; air_idx < AIRS_COUNT; air_idx++) {
    // units_air_ptr[air_idx][BOMBERS_LAND_AIR][0] = 0;
    uint8_t sea_idx = air_idx - LANDS_COUNT;
    uint8_t free_space = allied_carriers[sea_idx] * 2;
    for (uint8_t player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      free_space -= player_sea_unit_types[player_idx][sea_idx][FIGHTERS_SEA];
    }
    uint8_t* total_fighter_count = &air_units_state[air_idx][FIGHTERS_AIR][0];
    if (free_space < *total_fighter_count) {
      uint8_t fighters_lost = *total_fighter_count - free_space;
#ifdef DEBUG
      printf("DEBUG: Crashing %d fighters at %d\n", fighters_lost, air_idx);
#endif
      player_navies[0][sea_idx] -= fighters_lost;
      my_sea_unit_types[sea_idx][FIGHTERS_SEA] -= fighters_lost;
      *total_fighter_count = free_space;
    }
  }
}
void reset_units_fully() {
  // reset battleship health
  for (uint8_t sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    sea_units_state[sea_idx][BATTLESHIPS][0] += current_player_sea_unit_types[sea_idx][BS_DAMAGED];
    my_sea_unit_types[sea_idx][BATTLESHIPS] += my_sea_unit_types[sea_idx][BS_DAMAGED];
    sea_units_state[sea_idx][BS_DAMAGED][0] = 0;
    sea_units_state[sea_idx][BS_DAMAGED][1] = 0;
    my_sea_unit_types[sea_idx][BS_DAMAGED] = 0;
  }
}
// TODO BUY FACTORY
void buy_factory() {}
void collect_money() {
  // if player still owns their capital, collect income
  state.money[0] +=
      (income_per_turn[0] * (*owner_idx[PLAYERS[state.player_index].capital_territory_index] == 0));
}
void rotate_turns() {
  // rotate units
#ifdef DEBUG
  setPrintableStatus();
  printf("%s\n", printableGameStatus);
  printf("DEBUG: Rotating turns\n");
  for (uint8_t land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    if (player_armies[0][land_idx] > 0 && enemy_units_count[land_idx] > 0) {
      printf("DEBUG: Player %s has %d units at %s\n", PLAYERS[state.player_index].name,
             total_player_land_units[0][land_idx], LANDS[land_idx].name);
    }
  }
  for (uint8_t sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    if (player_navies[0][sea_idx] > 0 && enemy_units_count[sea_idx + LANDS_COUNT] > 0) {
      printf("DEBUG: Player %s has %d units at %s\n", PLAYERS[state.player_index].name,
             player_navies[0][sea_idx], SEAS[sea_idx].name);
    }
  }
#endif
  memcpy(&temp_land_unit_types, &my_land_unit_types, sizeof(state.other_land_units[0]));
  memcpy(&my_land_unit_types, &state.other_land_units[0], sizeof(state.other_land_units[0]));
  memmove(&state.other_land_units[0], &state.other_land_units[1],
          sizeof(state.other_land_units[0]) * (PLAYERS_COUNT - 2));
  memcpy(&state.other_land_units[PLAYERS_COUNT - 2], &temp_land_unit_types,
         sizeof(state.other_land_units[0]));
  for (uint8_t dst_land = 0; dst_land < LANDS_COUNT; dst_land++) {
    LandTerr* land0 = &state.land_terr[dst_land];
    uint8_t* land1 = my_land_unit_types[dst_land];
    // TODO optimize restructuring
    memset(&state.land_terr[dst_land].fighters, 0, sizeof(state.land_terr[0].fighters));
    land0->fighters[FIGHTER_STATES - 1] = land1[FIGHTERS_LAND];
    memset(&state.land_terr[dst_land].bombers, 0, sizeof(state.land_terr[0].bombers));
    land0->bombers[BOMBER_LAND_STATES - 1] = land1[BOMBERS_LAND];
    memset(&state.land_terr[dst_land].infantry, 0, sizeof(state.land_terr[0].infantry));
    land0->infantry[INFANTRY_STATES - 1] = land1[INFANTRY];
    memset(&state.land_terr[dst_land].artillery, 0, sizeof(state.land_terr[0].artillery));
    land0->artillery[ARTILLERY_STATES - 1] = land1[ARTILLERY];
    memset(&state.land_terr[dst_land].tanks, 0, sizeof(state.land_terr[0].tanks));
    land0->tanks[TANK_STATES - 1] = land1[TANKS];
    memset(&state.land_terr[dst_land].aa_guns, 0, sizeof(state.land_terr[0].aa_guns));
    land0->aa_guns[AA_GUN_STATES - 1] = land1[AA_GUNS];
  }
  memcpy(&temp_sea_unit_types, &my_sea_unit_types, SEA_UNIT_TYPES_COUNT * SEAS_COUNT);
  //  memcpy(&other_sea_units_0, &data.other_sea_units[0], OTHER_SEA_UNITS_SIZE);
  for (uint8_t dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
    memcpy(&my_sea_unit_types[dst_sea], &state.other_sea_units[0][dst_sea],
           SEA_UNIT_TYPES_COUNT - 1);
  }
  memmove(&state.other_sea_units[0], &state.other_sea_units[1],
          sizeof(state.other_sea_units[0]) * (PLAYERS_COUNT - 2));
  //  memcpy(&data.other_sea_units[PLAYERS_COUNT - 2], &other_sea_units_temp,
  //  OTHER_SEA_UNITS_SIZE);
  memset(&state.sea_terr, 0, sizeof(state.sea_terr));
  for (uint8_t dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
    memcpy(&state.other_sea_units[PLAYERS_COUNT - 2][dst_sea], &temp_sea_unit_types[dst_sea],
           SEA_UNIT_TYPES_COUNT - 1);
    for (uint8_t unit_type = 0; unit_type < LAND_UNIT_TYPES_COUNT; unit_type++) {
      SeaTerr* sea0 = &state.sea_terr[dst_sea];
      uint8_t* sea1 = my_sea_unit_types[dst_sea];
      sea0->fighters[FIGHTER_STATES - 1] = sea1[FIGHTERS_SEA];
      sea0->trans_empty[TRANS_EMPTY_STATES - 1] = sea1[TRANS_EMPTY];
      sea0->trans_1i[TRANS_1I_STATES - 1] = sea1[TRANS_1I];
      sea0->trans_1a[TRANS_1A_STATES - 1] = sea1[TRANS_1A];
      sea0->trans_1t[TRANS_1T_STATES - 1] = sea1[TRANS_1T];
      sea0->trans_2i[TRANS_2I_STATES - 1] = sea1[TRANS_2I];
      sea0->trans_1i_1a[TRANS_1I_1A_STATES - 1] = sea1[TRANS_1I_1A];
      sea0->trans_1i_1t[TRANS_1I_1T_STATES - 1] = sea1[TRANS_1I_1T];
      sea0->submarines[SUBMARINE_STATES - 1] = sea1[SUBMARINES];
      sea0->destroyers[DESTROYER_STATES - 1] = sea1[DESTROYERS];
      sea0->carriers[CARRIER_STATES - 1] = sea1[CARRIERS];
      sea0->cruisers[CRUISER_STATES - 1] = sea1[CRUISERS];
      sea0->battleships[BATTLESHIP_STATES - 1] = sea1[BATTLESHIPS];
      sea0->bs_damaged[BATTLESHIP_STATES - 1] = sea1[BS_DAMAGED];
      sea0->bombers[BOMBER_SEA_STATES - 1] = sea1[BOMBERS_SEA];
    }
  }
  uint8_t temp_money = state.money[0];
  memmove(&state.money[0], &state.money[1], sizeof(state.money[0]) * (PLAYERS_COUNT - 1));
  state.money[PLAYERS_COUNT - 1] = temp_money;
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
  memset(&state.flagged_for_combat, 0, sizeof(state.flagged_for_combat));
  state.player_index = (state.player_index + 1) % PLAYERS_COUNT;

  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    state.land_terr[land_idx].owner_idx =
        (state.land_terr[land_idx].owner_idx + PLAYERS_COUNT - 1) % PLAYERS_COUNT;
  }

  for (int factory_idx = 0; factory_idx < factory_count[0]; factory_idx++) {
    int dst_land = factory_locations[0][factory_idx];
    state.builds_left[dst_land] = *factory_max[dst_land];
    for (int sea_idx = 0; sea_idx < LAND_TO_SEA_COUNT[dst_land]; sea_idx++) {
      state.builds_left[LAND_TO_SEA_CONN[dst_land][sea_idx] + LANDS_COUNT] +=
          *factory_max[dst_land];
    }
  }

  refresh_cache();
#ifdef DEBUG
  printf("DEBUG: Cache refreshed. Player %s's turn\n", PLAYERS[state.player_index].name);
  setPrintableStatus();
  printf("%s\n", printableGameStatus);
  for (uint8_t player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    for (uint8_t factory_index = 0; factory_index < factory_count[player_idx]; factory_index++) {
      uint8_t factory_location = factory_locations[player_idx][factory_index];
      if (*owner_idx[factory_location] != player_idx) {
        printf("DEBUG: Player %s has a unowned factory at %s\n", PLAYERS[state.player_index].name,
               LANDS[factory_location].name);
      }
    }
  }
#endif

  // json = serialize_game_data_to_json(&data);
  // write_json_to_file("backup_game.json", json);
  // cJSON_Delete(json);
}

double get_score() {
  int allied_score = 1; // one helps prevent division by zero
  int enemy_score = 1;
  for (int player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    int score = state.money[player_idx];
    for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
      for (int unit_type = 0; unit_type < LAND_UNIT_TYPES_COUNT; unit_type++) {
        score +=
            player_land_unit_types[player_idx][land_idx][unit_type] * COST_UNIT_LAND[unit_type];
      }
    }
    for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
      for (int unit_type = 0; unit_type < SEA_UNIT_TYPES_COUNT; unit_type++) {
        score += player_sea_unit_types[player_idx][sea_idx][unit_type] * COST_UNIT_SEA[unit_type];
      }
    }
    if (is_allied_0[player_idx]) {
      allied_score += score;
    } else {
      enemy_score += score;
    }
  }
  // return ((double)2 * (double)allied_score / (double)(enemy_score + allied_score)) - (double)1;
  return ((double)allied_score / (double)(enemy_score + allied_score));
}

GameState* get_game_state_copy() {
  GameState* game_state = (GameState*)malloc(sizeof(GameState));
  memcpy(game_state, &state, sizeof(GameState));
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

uint8_t* get_possible_actions(GameState* game_state, int* num_actions) {
  // Return the list of possible actions from the given state
  memcpy(&state, game_state, sizeof(GameState));
  refresh_quick_totals();
  refresh_cache();
  answers_remaining = 0;
  while (true) {
    if (move_fighter_units())
      break;
    if (move_bomber_units())
      break;
    if (stage_transport_units())
      break;
    if (move_land_unit_type(TANKS))
      break;
    if (move_land_unit_type(ARTILLERY))
      break;
    if (move_land_unit_type(INFANTRY))
      break;
    if (move_transport_units())
      break;
    if (move_subs())
      break;
    if (move_destroyers_battleships())
      break;
    if (resolve_sea_battles())
      break;
    if (unload_transports())
      break;
    if (resolve_land_battles())
      break;
    if (move_land_unit_type(AA_GUNS))
      break;
    if (land_fighter_units())
      break;
    if (land_bomber_units())
      break;
    if (buy_units())
      break;
    crash_air_units();
    reset_units_fully();
    buy_factory();
    collect_money();
    rotate_turns();
  }
  *num_actions = valid_actions_count;
  return valid_actions;
}

void apply_action(GameState* game_state, uint8_t action) {
  // Apply the action to the game state
#ifdef DEBUG
  printf("DEBUG: copying state and Applying action %d\n", action);
#endif
  memcpy(&state, game_state, sizeof(GameState));
  refresh_quick_totals();
  refresh_cache();
  answers_remaining = 1;
  RANDOM_NUMBERS[random_number_index] = action;
  while (true) {
    if (move_fighter_units())
      break;
    if (move_bomber_units())
      break;
    if (stage_transport_units())
      break;
    if (move_land_unit_type(TANKS))
      break;
    if (move_land_unit_type(ARTILLERY))
      break;
    if (move_land_unit_type(INFANTRY))
      break;
    if (move_transport_units())
      break;
    if (move_subs())
      break;
    if (move_destroyers_battleships())
      break;
    if (resolve_sea_battles())
      break;
    if (unload_transports())
      break;
    if (resolve_land_battles())
      break;
    if (move_land_unit_type(AA_GUNS))
      break;
    if (land_fighter_units())
      break;
    if (land_bomber_units())
      break;
    if (buy_units())
      break;
    crash_air_units();
    reset_units_fully();
    buy_factory();
    collect_money();
    rotate_turns();
  }
  memcpy(game_state, &state, sizeof(GameState));
}
double random_play_until_terminal(GameState* game_state) {
  memcpy(&state, game_state, sizeof(GameState));
  uint8_t starting_player = state.player_index;
  refresh_quick_totals();
  refresh_cache();
  answers_remaining = 10000;
  double score = get_score();
  int max_loops = 100000;
  while (score > 0.01 && score < 0.99 && max_loops-- > 0) {
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
  if (PLAYERS[game_state->player_index]
          .is_allied[(game_state->player_index + starting_player) % PLAYERS_COUNT])
    score = 1 - score;
  return score;
}

bool is_terminal_state(GameState* game_state) {
  // Return true if the game is over
  double score = evaluate_state(game_state);
  return (score > 0.99 || score < 0.01);
}

double evaluate_state(GameState* game_state) {
  // Evaluate the game state and return a score
  int allied_score = 1; // one helps prevent division by zero
  int enemy_score = 1;
  allied_score += game_state->money[0];
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    LandTerr* land_state = &game_state->land_terr[land_idx];
    int total_units = 0;
    for (int unit_state = 0; unit_state < FIGHTER_STATES; unit_state++) {
      total_units += land_state->fighters[unit_state];
    }
    allied_score += total_units * FIGHTER_COST;
    for (int unit_state = 0; unit_state < BOMBER_LAND_STATES; unit_state++) {
      total_units += land_state->bombers[unit_state];
    }
    allied_score += total_units * BOMBER_COST;
    for (int unit_state = 0; unit_state < INFANTRY_STATES; unit_state++) {
      total_units += land_state->infantry[unit_state];
    }
    allied_score += total_units * INFANTRY_COST;
    total_units = 0;
    for (int unit_state = 0; unit_state < ARTILLERY_STATES; unit_state++) {
      total_units += land_state->artillery[unit_state];
    }
    allied_score += total_units * ARTILLERY_COST;
    total_units = 0;
    for (int unit_state = 0; unit_state < TANK_STATES; unit_state++) {
      total_units += land_state->tanks[unit_state];
    }
    allied_score += total_units * TANK_COST;
    total_units = 0;
    for (int unit_state = 0; unit_state < AA_GUN_STATES; unit_state++) {
      total_units += land_state->aa_guns[unit_state];
    }
    allied_score += total_units * AA_GUN_COST;
  }
  for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    SeaTerr* sea_state = &game_state->sea_terr[sea_idx];
    int total_units = 0;
    for (int unit_state = 0; unit_state < FIGHTER_STATES; unit_state++) {
      total_units += sea_state->fighters[unit_state];
    }
    allied_score += total_units * FIGHTER_COST;
    for (int unit_state = 0; unit_state < TRANS_EMPTY_STATES; unit_state++) {
      total_units += sea_state->trans_empty[unit_state];
    }
    allied_score += total_units * TRANSPORT_COST;
    for (int unit_state = 0; unit_state < TRANS_1I_STATES; unit_state++) {
      total_units += sea_state->trans_1i[unit_state];
    }
    allied_score += total_units * (TRANSPORT_COST + INFANTRY_COST);
    for (int unit_state = 0; unit_state < TRANS_1A_STATES; unit_state++) {
      total_units += sea_state->trans_1a[unit_state];
    }
    allied_score += total_units * (TRANSPORT_COST + ARTILLERY_COST);
    for (int unit_state = 0; unit_state < TRANS_1T_STATES; unit_state++) {
      total_units += sea_state->trans_1t[unit_state];
    }
    allied_score += total_units * (TRANSPORT_COST + TANK_COST);
    for (int unit_state = 0; unit_state < TRANS_2I_STATES; unit_state++) {
      total_units += sea_state->trans_2i[unit_state];
    }
    allied_score += total_units * (TRANSPORT_COST + INFANTRY_COST + INFANTRY_COST);
    for (int unit_state = 0; unit_state < TRANS_1I_1A_STATES; unit_state++) {
      total_units += sea_state->trans_1i_1a[unit_state];
    }
    allied_score += total_units * (TRANSPORT_COST + INFANTRY_COST + ARTILLERY_COST);
    for (int unit_state = 0; unit_state < TRANS_1I_1T_STATES; unit_state++) {
      total_units += sea_state->trans_1i_1t[unit_state];
    }
    allied_score += total_units * (TRANSPORT_COST + INFANTRY_COST + TANK_COST);
    for (int unit_state = 0; unit_state < SUBMARINE_STATES; unit_state++) {
      total_units += sea_state->submarines[unit_state];
    }
    allied_score += total_units * SUB_COST;
    for (int unit_state = 0; unit_state < DESTROYER_STATES; unit_state++) {
      total_units += sea_state->destroyers[unit_state];
    }
    allied_score += total_units * DESTROYER_COST;
    for (int unit_state = 0; unit_state < CARRIER_STATES; unit_state++) {
      total_units += sea_state->carriers[unit_state];
    }
    allied_score += total_units * CARRIER_COST;
    for (int unit_state = 0; unit_state < CRUISER_STATES; unit_state++) {
      total_units += sea_state->cruisers[unit_state];
    }
    allied_score += total_units * CRUISER_COST;
    for (int unit_state = 0; unit_state < BATTLESHIP_STATES; unit_state++) {
      total_units += sea_state->battleships[unit_state];
    }
    allied_score += total_units * BATTLESHIP_COST;
    for (int unit_state = 0; unit_state < BATTLESHIP_STATES; unit_state++) {
      total_units += sea_state->bs_damaged[unit_state];
    }
    allied_score += total_units * BATTLESHIP_COST;
    for (int unit_state = 0; unit_state < BOMBER_SEA_STATES; unit_state++) {
      total_units += sea_state->bombers[unit_state];
    }
    allied_score += total_units * BOMBER_COST;
  }
  for (int player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
    int score = game_state->money[player_idx];
    for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
      for (int unit_type = 0; unit_type < LAND_UNIT_TYPES_COUNT; unit_type++) {
        score +=
            player_land_unit_types[player_idx][land_idx][unit_type] * COST_UNIT_LAND[unit_type];
      }
    }
    for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
      for (int unit_type = 0; unit_type < SEA_UNIT_TYPES_COUNT; unit_type++) {
        score += player_sea_unit_types[player_idx][sea_idx][unit_type] * COST_UNIT_SEA[unit_type];
      }
    }
    if (PLAYERS[game_state->player_index]
            .is_allied[(game_state->player_index + player_idx) % PLAYERS_COUNT]) {
      allied_score += score;
    } else {
      enemy_score += score;
    }
  }
  return ((double)allied_score / (double)(enemy_score + allied_score));
}
