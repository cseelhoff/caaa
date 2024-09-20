#include "engine.h"
#include "game_state.hpp"
#include "land.h"
#include "mcts.h"
#include "player.h"
#include "sea.h"
#include "serialize_data.h"
#include "units/units.h"
#include <array>
#include <cstdint>
#include <limits>
#include <pybind11/pybind11.h>
#include <sys/types.h>
#include <unistd.h>

#define COPY_FULL_ARRAY(src, dest) std::copy(std::begin(src), std::end(src), std::begin(dest));
#define COPY_SUB_ARRAY(src, dest, count) std::copy_n((src).begin(), count, (dest).begin());
#define FILL_ARRAY(arr, value) std::fill(std::begin(arr), std::end(arr), value);
#define FILL_2D_ARRAY(arr, value)                                                                  \
  for (auto& row : arr) {                                                                          \
    std::fill(std::begin(row), std::end(row), value);                                              \
  }
#define FILL_3D_ARRAY(array, value)                                                                \
  for (auto& matrix : array) {                                                                     \
    for (auto& row : matrix) {                                                                     \
      std::fill(std::begin(row), std::end(row), value);                                            \
    }                                                                                              \
  }

#define MAX_INT std::numeric_limits<int>::max()

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
SeaSeaArray seas_within_1_move_canal = {{{0}}};
SeaArray seas_within_1_move_count_canal = {0};
SeaSeaArray seas_within_2_moves_canal = {{{0}}};
SeaArray seas_within_2_moves_count_canal = {0};
AirhopAirAirArray AIR_WITHIN_X_MOVES = {{{{{0}}}}};
AirhopAirArray AIR_WITHIN_X_MOVES_COUNT = {{{0}}};
AirhopAirLandArray AIR_TO_LAND_WITHIN_X_MOVES = {{{{{0}}}}};
AirhopLandArray AIR_TO_LAND_WITHIN_X_MOVES_COUNT = {{{0}}};
CanalSeaSeaArray SEA_DIST = {{{{{0}}}}};
SeaSeaArray sea_dist = {{{0}}};

CanalSeaSeaArray SEA_PATH = {{{{{MAX_INT}}}}};
CanalSeaSeaArray SEA_PATH_ALT = {{{{{MAX_INT}}}}};
SeaSeaArray sea_path = {{{MAX_INT}}};
SeaSeaArray sea_path_alt = {{{MAX_INT}}};
LandArray LAND_TO_LAND_COUNT = {0};
LandL2LArray LAND_TO_LAND_CONN = {{{0}}};
LandArray LAND_TO_SEA_COUNT = {0};
LandL2SArray LAND_TO_SEA_CONN = {{{0}}};
SeaArray SEA_TO_SEA_COUNT = {0};

SeaS2SArray SEA_TO_SEA_CONN = {{{0}}};
// int SEA_TO_SEA_CONN[SEAS_COUNT][MAX_SEA_TO_SEA_CONNECTIONS] = {{0}};
SeaArray SEA_TO_LAND_COUNT = {0};
SeaS2LArray SEA_TO_LAND_CONN = {{{0}}};
RandomNumberArray RANDOM_NUMBERS = {0};
uint random_number_index = 0;
uint seed = 0;
char printableGameStatus[PRINTABLE_GAME_STATUS_SIZE] = "";
GameState state;
cJSON* json;
// use __llvm_libc
//#define OTHER_LAND_UNITS_SIZE 30
//#define OTHER_SEA_UNITS_SIZE 42
// int MULTI_OTHER_LAND_UNITS_SIZE = (PLAYERS_COUNT - 2) * OTHER_LAND_UNITS_SIZE;
// int MULTI_OTHER_SEA_UNITS_SIZE = (PLAYERS_COUNT - 2) * OTHER_SEA_UNITS_SIZE;

PtrLandUTArray land_units_state;
PtrSeaUTArray sea_units_state;
PtrAirUTArray air_units_state;
PlayersbufLandUTArray total_player_land_unit_types;
PlayersbufSeaUTArray total_player_sea_unit_types;
PtrLandArray owner_idx;
PtrLandArray factory_max;
PtrLandArray bombard_max;
PtrLandArray factory_hp;                    // allow negative
LandUTArray current_player_land_unit_types; // temp
SeaUTArray current_player_sea_unit_types;   // temp
LandUTArray total_land_unit_types_temp;     // temp
SeaUTArray total_sea_units_temp;            // temp
Playersbuf income_per_turn = {0};
Playersbuf total_factory_count = {0};
PlayersbufLandArray factory_locations = {{{0}}};
PlayersbufLandArray total_player_land_units = {{{0}}};
PlayersbufSeaArray total_player_sea_units = {{{0}}};
AirAirArray hist_skipped_airs = {{{0}}};
AirAirArray hist_source_territories = {{{0}}};
AirArray hist_source_territories_count = {0};

// int units_sea_blockade_total[PLAYERS_COUNT][SEAS_COUNT];
SeaArray enemy_blockade_total = {0};
SeaArray enemy_destroyers_total = {0};
LandLandArray is_land_path_blocked = {{{0}}};
SeaSeaArray is_sea_path_blocked = {{{0}}};
SeaSeaArray is_sub_path_blocked = {{{0}}};
SeaArray transports_with_large_cargo_space;
SeaArray transports_with_small_cargo_space;

Playersbuf enemies_0 = {0};
Playersbuf is_allied_0 = {0};
uint enemies_count_0 = 0;
uint canal_state = 0;
SeaArray allied_carriers = {0};
AirArray enemy_units_count = {0};
AirArray canFighterLandHere = {0};
AirArray canFighterLandIn1Move = {0};
AirArray canBomberLandHere = {0};
AirArray canBomberLandIn1Move = {0};
AirArray canBomberLandIn2Moves = {0};
uint step_id = 0;
uint answers_remaining = 0;
bool use_selected_action = false;

Actions valid_moves = {0};
uint valid_moves_count = 0;
uint selected_action = 0;
uint max_loops = 0;
bool actually_print = false;
uint unlucky_player_idx = 0;

void initialize_constants() {
  actually_print = PLAYERS[0].is_human;
  initialize_land_dist();
  land_dist_floyd_warshall();
  initialize_sea_dist();
  initialize_air_dist();
  initialize_land_path();
  initialize_sea_path();
  initialize_within_x_moves();
  initialize_random_numbers();
  initialize_land_pointers();
  initialize_sea_pointers();
  intialize_airs_x_to_4_moves_away();
  initialize_skip_4air_precals();
}

void initialize_land_dist() {
  FILL_2D_ARRAY(LAND_DIST, MAX_INT)
  for (uint src_land = 0; src_land < LANDS_COUNT; src_land++) {
    LAND_VALUE[src_land] = LANDS[src_land].land_value;
    Land land = LANDS[src_land];
    // initialize LAND_TO_LAND_CONN
    uint land_conn_count = land.land_conn_count;
    LAND_TO_LAND_COUNT[src_land] = land_conn_count;
    COPY_SUB_ARRAY(land.land_conns, LAND_TO_LAND_CONN[src_land], LAND_TO_LAND_COUNT[src_land]);
    // set_l2l_land_dist_to_one
    for (uint conn_idx = 0; conn_idx < land_conn_count; conn_idx++) {
      uint dst_land = land.land_conns[conn_idx];
      LAND_DIST[src_land][dst_land] = 1;
      LAND_DIST[dst_land][src_land] = 1;
    }
    // initialize LAND_TO_SEA_CONN
    uint sea_conn_count = land.sea_conn_count;
    LAND_TO_SEA_COUNT[src_land] = sea_conn_count;
    COPY_SUB_ARRAY(land.sea_conns, LAND_TO_SEA_CONN[src_land], LAND_TO_SEA_COUNT[src_land]);
    LAND_DIST[src_land][src_land] = 0;
    // set_l2s_land_dist_to_one
    for (uint conn_idx = 0; conn_idx < sea_conn_count; conn_idx++) {
      uint dst_air = land.sea_conns[conn_idx] + LANDS_COUNT;
      LAND_DIST[src_land][dst_air] = 1;
    }
  }
}

void land_dist_floyd_warshall() {
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    for (uint src_land = 0; src_land < LANDS_COUNT; src_land++) {
      for (uint air_dst = 0; air_dst < AIRS_COUNT; air_dst++) {
        uint new_dist = LAND_DIST[src_land][land_idx] + LAND_DIST[land_idx][air_dst];
        if (new_dist < LAND_DIST[src_land][air_dst]) {
          LAND_DIST[src_land][air_dst] = new_dist;
        }
      }
    }
  }
}

void initialize_sea_dist() {
  FILL_3D_ARRAY(SEA_DIST, MAX_INT);
  for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    initialize_s2s_connections(src_sea);
    initialize_s2l_connections(src_sea);
  }
  for (uint canal_idx = 0; canal_idx < CANAL_STATES; canal_idx++) {
    initialize_sea_dist_zero(canal_idx);
    set_s2s_sea_dist_to_one(canal_idx);
    initialize_canals(canal_idx);
    sea_dist_floyd_warshall(canal_idx);
  }
}
void initialize_s2s_connections(uint src_sea) {
  SEA_TO_SEA_COUNT[src_sea] = SEAS[src_sea].sea_conn_count;
  for (uint conn_idx = 0; conn_idx < SEA_TO_SEA_COUNT[src_sea]; conn_idx++) {
    SEA_TO_SEA_CONN[src_sea][conn_idx] = SEAS[src_sea].sea_conns[conn_idx];
  }
}
void initialize_s2l_connections(uint src_sea) {
  SEA_TO_LAND_COUNT[src_sea] = SEAS[src_sea].land_conn_count;
  for (uint conn_idx = 0; conn_idx < SEA_TO_LAND_COUNT[src_sea]; conn_idx++) {
    SEA_TO_LAND_CONN[src_sea][conn_idx] = SEAS[src_sea].land_conns[conn_idx];
  }
}
void initialize_sea_dist_zero(uint canal_idx) {
  for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    SEA_DIST[canal_idx][src_sea][src_sea] = 0;
  }
}
void set_s2s_sea_dist_to_one(uint canal_idx) {
  for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    for (uint conn_idx = 0; conn_idx < SEAS[src_sea].sea_conn_count; conn_idx++) {
      uint dst_sea = SEAS[src_sea].sea_conns[conn_idx];
      SEA_DIST[canal_idx][src_sea][dst_sea] = 1;
      SEA_DIST[canal_idx][dst_sea][src_sea] = 1;
    }
  }
}
void initialize_canals(uint canal_idx) {
  // convert canal_state to a bitmask and loop through CANALS for those
  // enabled for example if canal_state is 0, do not process any items in
  // CANALS, if canal_state is 1, process the first item in CANALS, if
  // canal_state is 2, process the second item in CANALS, if canal_state is
  // 3, process the first and second items in CANALS, etc.
  for (uint conn_idx = 0; conn_idx < CANALS_COUNT; conn_idx++) {
    if ((canal_idx & (1U << conn_idx)) == 0) {
      continue;
    }
    auto seas = CANALS[conn_idx].seas;
    SEA_DIST[canal_idx][seas[0]][seas[1]] = 1;
    SEA_DIST[canal_idx][seas[1]][seas[0]] = 1;
  }
}
void sea_dist_floyd_warshall(uint canal_idx) {
  for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      for (uint dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
        uint new_dist =
            SEA_DIST[canal_idx][src_sea][sea_idx] + SEA_DIST[canal_idx][sea_idx][dst_sea];
        if (new_dist < SEA_DIST[canal_idx][src_sea][dst_sea]) {
          SEA_DIST[canal_idx][src_sea][dst_sea] = new_dist;
        }
      }
    }
  }
}
void initialize_air_dist() {
  FILL_2D_ARRAY(AIR_DIST, MAX_INT)
  initialize_air_dist_zero();
  for (uint src_land = 0; src_land < LANDS_COUNT; src_land++) {
    set_l2l_air_dist_to_one(src_land);
    set_l2s_air_dist_to_one(src_land);
  }
  for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    set_s2l_air_dist_to_one(src_sea);
    set_s2s_air_dist_to_one(src_sea);
  }
  air_dist_floyd_warshall();
}
void initialize_air_dist_zero() {
  for (uint src_air = 0; src_air < AIRS_COUNT; src_air++) {
    AIR_DIST[src_air][src_air] = 0;
  }
}
void set_l2l_air_dist_to_one(uint src_land) {
  auto l2l_conn = LAND_TO_LAND_CONN[src_land];
  for (uint conn_idx = 0; conn_idx < (uint)LAND_TO_LAND_COUNT[src_land]; conn_idx++) {
    uint dst_air = l2l_conn[conn_idx];
    AIR_CONNECTIONS[src_land][(uint)AIR_CONN_COUNT[src_land]] = dst_air;
    AIR_CONN_COUNT[src_land]++;
    AIR_DIST[src_land][(uint)dst_air] = 1;
    AIR_DIST[(uint)dst_air][src_land] = 1;
  }
}
void set_l2s_air_dist_to_one(uint src_land) {
  for (uint conn_idx = 0; conn_idx < LAND_TO_SEA_COUNT[src_land]; conn_idx++) {
    uint dst_air = LAND_TO_SEA_CONN[src_land][conn_idx] + LANDS_COUNT;
    AIR_CONNECTIONS[src_land][AIR_CONN_COUNT[src_land]] = dst_air;
    AIR_CONN_COUNT[src_land]++;
    AIR_DIST[src_land][dst_air] = 1;
    AIR_DIST[dst_air][src_land] = 1;
  }
}
void set_s2l_air_dist_to_one(uint src_sea) {
  uint src_air = src_sea + LANDS_COUNT;
  for (uint conn_idx = 0; conn_idx < SEA_TO_LAND_COUNT[src_sea]; conn_idx++) {
    uint dst_air = SEA_TO_LAND_CONN[src_sea][conn_idx];
    AIR_CONNECTIONS[src_air][AIR_CONN_COUNT[src_air]] = dst_air;
    AIR_CONN_COUNT[src_air]++;
    AIR_DIST[src_air][dst_air] = 1;
    AIR_DIST[dst_air][src_air] = 1;
  }
}
void set_s2s_air_dist_to_one(uint src_sea) {
  uint src_air = src_sea + LANDS_COUNT;
  for (uint conn_idx = 0; conn_idx < SEA_TO_SEA_COUNT[src_sea]; conn_idx++) {
    uint dst_air = SEA_TO_SEA_CONN[src_sea][conn_idx] + LANDS_COUNT;
    AIR_CONNECTIONS[src_air][AIR_CONN_COUNT[src_air]] = dst_air;
    AIR_CONN_COUNT[src_air]++;
    AIR_DIST[src_air][dst_air] = 1;
    AIR_DIST[dst_air][src_air] = 1;
  }
}
void air_dist_floyd_warshall() {
  for (uint air_index = 0; air_index < AIRS_COUNT; air_index++) {
    for (uint src_air = 0; src_air < AIRS_COUNT; src_air++) {
      for (uint dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
        uint new_dist = AIR_DIST[src_air][air_index] + AIR_DIST[air_index][dst_air];
        if (new_dist < AIR_DIST[src_air][dst_air]) {
          AIR_DIST[src_air][dst_air] = new_dist;
        }
      }
    }
  }
}
void initialize_land_path() {
  for (uint src_land = 0; src_land < LANDS_COUNT; src_land++) {
    for (uint conn_idx = 0; conn_idx < LAND_TO_LAND_COUNT[src_land]; conn_idx++) {
      uint mid_land = LAND_TO_LAND_CONN[src_land][conn_idx];
      set_land_path_for_l2l(src_land, mid_land, LAND_PATH);
      set_land_path_for_l2s(src_land, mid_land, LAND_PATH);
      // Iterate the loop in reverse order for alt path
      uint mid_land_alt = LAND_TO_LAND_CONN[src_land][LAND_TO_LAND_COUNT[src_land] - 1 - conn_idx];
      set_land_path_for_l2l(src_land, mid_land_alt, LAND_PATH_ALT);
      set_land_path_for_l2s(src_land, mid_land_alt, LAND_PATH_ALT);
    }
  }
}
void set_land_path_for_l2l(uint src_land, uint intermediate_land, LandAirArray land_path) {
  for (uint conn_idx2 = 0; conn_idx2 < LAND_TO_LAND_COUNT[intermediate_land]; conn_idx2++) {
    uint dst_land2 = LAND_TO_LAND_CONN[intermediate_land][conn_idx2];
    if (LAND_DIST[src_land][dst_land2] == 2) {
      land_path[src_land][dst_land2] = intermediate_land;
    }
  }
}
void set_land_path_for_l2s(uint src_land, uint intermediate_land, LandAirArray land_path) {
  for (uint conn_idx2 = 0; conn_idx2 < LAND_TO_SEA_COUNT[intermediate_land]; conn_idx2++) {
    uint dst_air = LAND_TO_SEA_CONN[intermediate_land][conn_idx2] + LANDS_COUNT;
    if (LAND_DIST[src_land][dst_air] == 2) {
      land_path[src_land][dst_air] = intermediate_land;
    }
  }
}
void initialize_sea_path() {
  for (uint canal_state_idx = 0; canal_state_idx < CANAL_STATES; canal_state_idx++) {
    for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      for (uint conn_idx = 0; conn_idx < SEA_TO_SEA_COUNT[src_sea]; conn_idx++) {
        initialize_sea_path_2(conn_idx, src_sea, canal_state_idx, &SEA_PATH);
        initialize_sea_path_2(SEA_TO_SEA_COUNT[src_sea] - 1 - conn_idx, src_sea, canal_state_idx,
                              &SEA_PATH_ALT);
      }
    }
  }
}

void initialize_sea_path_2(uint mid_sea_idx, uint src_sea, ulong canal_state_idx,
                           CanalSeaSeaArray* sea_path_ptr) {
  uint mid_sea = SEA_TO_SEA_CONN[src_sea][mid_sea_idx];
  for (uint conn_idx2 = 0; conn_idx2 < SEA_TO_SEA_COUNT[mid_sea]; conn_idx2++) {
    uint dst_sea = SEA_TO_SEA_CONN[mid_sea][conn_idx2];
    if (SEA_DIST[canal_state_idx][src_sea][dst_sea] == 2) {
      (*sea_path_ptr)[canal_state_idx][src_sea][dst_sea] = mid_sea;
    }
  }
}

void initialize_within_x_moves() {
  for (uint src_land = 0; src_land < LANDS_COUNT; src_land++) {
    initialize_land_within_2_moves(src_land);
    initialize_load_within_2_moves(src_land);
  }
  initialize_sea_within_x_moves();
  initialize_air_within_x_moves();
  initialize_air_to_land_within_x_moves();
}
void initialize_land_within_2_moves(uint src_land) {
  for (uint dst_land = 0; dst_land < LANDS_COUNT; dst_land++) {
    if (src_land == dst_land) {
      continue;
    }
    if (LAND_DIST[src_land][dst_land] <= 2) {
      LANDS_WITHIN_2_MOVES[src_land][LANDS_WITHIN_2_MOVES_COUNT[src_land]++] = dst_land;
    }
  }
}
void initialize_load_within_2_moves(uint src_land) {
  for (uint dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
    if (LAND_DIST[src_land][dst_sea + LANDS_COUNT] <= 2) {
      LOAD_WITHIN_2_MOVES[src_land][LOAD_WITHIN_2_MOVES_COUNT[src_land]++] = dst_sea;
    }
  }
}
void initialize_sea_within_x_moves() {
  for (uint canal_state_idx = 0; canal_state_idx < CANAL_STATES; canal_state_idx++) {
    for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      for (uint dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
        if (src_sea == dst_sea) {
          continue;
        }
        if (SEA_DIST[canal_state_idx][src_sea][dst_sea] <= 2) {
          SEAS_WITHIN_2_MOVES[canal_state_idx][src_sea]
                             [SEAS_WITHIN_2_MOVES_COUNT[canal_state_idx][src_sea]++] = dst_sea;
        } else {
          continue;
        }
        if (SEA_DIST[canal_state_idx][src_sea][dst_sea] <= 1) {
          SEAS_WITHIN_1_MOVE[canal_state_idx][src_sea]
                            [SEAS_WITHIN_1_MOVE_COUNT[canal_state_idx][src_sea]++] = dst_sea;
        }
      }
    }
  }
}
void initialize_air_within_x_moves() {
  for (uint src_air = 0; src_air < AIRS_COUNT; src_air++) {
    for (uint dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
      if (src_air == dst_air) {
        continue;
      }
      for (uint i = 0; i < 5; i++) {
        if (AIR_DIST[src_air][dst_air] <= i + 1) {
          AIR_WITHIN_X_MOVES[i][src_air][AIR_WITHIN_X_MOVES_COUNT[i][src_air]++] = dst_air;
        }
      }
      if (dst_air >= LANDS_COUNT) { // bombers can only end on land
        continue;
      }
      if (AIR_DIST[src_air][dst_air] <= 6) {
        AIR_WITHIN_X_MOVES[5][src_air][AIR_WITHIN_X_MOVES_COUNT[5][src_air]] = dst_air;
        AIR_WITHIN_X_MOVES_COUNT[5][src_air]++;
      }
    }
  }
}
void initialize_air_to_land_within_x_moves() {
  for (uint src_air = 0; src_air < AIRS_COUNT; src_air++) {
    for (uint dst_land = 0; dst_land < LANDS_COUNT; dst_land++) {
      if (src_air == dst_land) {
        continue;
      }
      for (uint i = 0; i < 6; i++) {
        if (AIR_DIST[src_air][dst_land] <= i + 1) {
          uint count = AIR_TO_LAND_WITHIN_X_MOVES_COUNT[i][src_air];
          AIR_TO_LAND_WITHIN_X_MOVES[i][src_air][count] = dst_land;
          (AIR_TO_LAND_WITHIN_X_MOVES_COUNT[i][src_air])++;
        }
      }
    }
  }
}
#define RANDOM_MAX 65536
#define ACTION_COUNT 256
void initialize_random_numbers() {
  for (uint i = 0; i < RANDOM_MAX; i++) {
    RANDOM_NUMBERS[i] = (rand() % ACTION_COUNT);
  }
}
void initialize_land_pointers() {
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    air_units_state[land_idx][FIGHTERS] = state.land_state[land_idx].fighters.data();
    air_units_state[land_idx][BOMBERS_LAND_AIR] = state.land_state[land_idx].bombers.data();
    land_units_state[land_idx][FIGHTERS] = state.land_state[land_idx].fighters.data();
    land_units_state[land_idx][BOMBERS_LAND_AIR] = state.land_state[land_idx].bombers.data();
    land_units_state[land_idx][INFANTRY] = state.land_state[land_idx].infantry.data();
    land_units_state[land_idx][ARTILLERY] = state.land_state[land_idx].artillery.data();
    land_units_state[land_idx][TANKS] = state.land_state[land_idx].tanks.data();
    land_units_state[land_idx][AA_GUNS] = state.land_state[land_idx].aa_guns.data();
    total_player_land_unit_types[0][land_idx] = current_player_land_unit_types[land_idx];
    for (uint player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      total_player_land_unit_types[player_idx][land_idx] =
          state.other_land_units[player_idx - 1][land_idx];
    }
    owner_idx[land_idx] = &state.land_state[land_idx].owner_idx;
    bombard_max[land_idx] = &state.land_state[land_idx].bombard_max;
    factory_hp[land_idx] = &state.land_state[land_idx].factory_hp;
    factory_max[land_idx] = &state.land_state[land_idx].factory_max;
  }
}
void initialize_sea_pointers() {
  for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    uint air_idx = sea_idx + LANDS_COUNT;
    air_units_state[air_idx][FIGHTERS] = state.units_sea[sea_idx].fighters.data();
    air_units_state[air_idx][BOMBERS_LAND_AIR] = state.units_sea[sea_idx].bombers.data();
    sea_units_state[sea_idx][FIGHTERS] = state.units_sea[sea_idx].fighters.data();
    sea_units_state[sea_idx][TRANS_EMPTY] = state.units_sea[sea_idx].trans_empty.data();
    sea_units_state[sea_idx][TRANS_1I] = state.units_sea[sea_idx].trans_1i.data();
    sea_units_state[sea_idx][TRANS_1A] = state.units_sea[sea_idx].trans_1a.data();
    sea_units_state[sea_idx][TRANS_1T] = state.units_sea[sea_idx].trans_1t.data();
    sea_units_state[sea_idx][TRANS_2I] = state.units_sea[sea_idx].trans_2i.data();
    sea_units_state[sea_idx][TRANS_1I_1A] = state.units_sea[sea_idx].trans_1i_1a.data();
    sea_units_state[sea_idx][TRANS_1I_1T] = state.units_sea[sea_idx].trans_1i_1t.data();
    sea_units_state[sea_idx][SUBMARINES] = state.units_sea[sea_idx].submarines.data();
    sea_units_state[sea_idx][DESTROYERS] = state.units_sea[sea_idx].destroyers.data();
    sea_units_state[sea_idx][CARRIERS] = state.units_sea[sea_idx].carriers.data();
    sea_units_state[sea_idx][CRUISERS] = state.units_sea[sea_idx].cruisers.data();
    sea_units_state[sea_idx][BATTLESHIPS] = state.units_sea[sea_idx].battleships.data();
    sea_units_state[sea_idx][BS_DAMAGED] = state.units_sea[sea_idx].bs_damaged.data();
    sea_units_state[sea_idx][BOMBERS_SEA] = state.units_sea[sea_idx].bombers.data();
    total_player_sea_unit_types[0][sea_idx] = current_player_sea_unit_types[sea_idx];
    for (uint player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      total_player_sea_unit_types[player_idx][sea_idx] =
          state.other_sea_units[player_idx - 1][sea_idx];
    }
  }
}

// bool state.skipped_moves[AIRS_COUNT][AIRS_COUNT] = {0};
//  skip_precals in format: src_air, dst_air: array of src_air's to skip
uint skip_4air_precals[AIRS_COUNT][AIRS_COUNT][AIRS_COUNT] = {0};
uint skip_4air_precals_count[AIRS_COUNT][AIRS_COUNT] = {0};
uint AIRS_X_TO_4_MOVES_AWAY[4][AIRS_COUNT][AIRS_COUNT] = {0};
uint AIRS_X_TO_4_MOVES_AWAY_COUNT[4][AIRS_COUNT] = {0};

void intialize_airs_x_to_4_moves_away() {
  for (uint moves = 1; moves <= 4; moves++) {
    for (uint src_air = 0; src_air < AIRS_COUNT; src_air++) {
      AirArray air_within_x_moves = AIR_WITHIN_X_MOVES[4 - 1][src_air];
      uint air_within_x_moves_count = AIR_WITHIN_X_MOVES_COUNT[4 - 1][src_air];
      for (uint dst_air_idx = 0; dst_air_idx < air_within_x_moves_count; dst_air_idx++) {
        uint dst_air = air_within_x_moves[dst_air_idx];
        uint dist = AIR_DIST[src_air][dst_air];
        if (dist >= moves) {
          AIRS_X_TO_4_MOVES_AWAY[moves - 1][dst_air]
                                [AIRS_X_TO_4_MOVES_AWAY_COUNT[moves - 1][dst_air_idx]++] = src_air;
        }
      }
    }
  }
}

// Notes on how allowed moves from history are calculated:
// Basically when Terr A with moves remaining X is skipped (from Terr C to Terr B)
//   in favor of Terr B with moves remaining Y,
//   we need to make sure that Terr A is always skipped if moves remaining is <= X and
//   Terr B moves remain are >= Y
// Said another way, if Distance[C][B] == 4, then get all terrs with distance 4,5,6 from B
//   for each terr D, loop through newly skipped terrs in ArrayX (ArrayX = Distance[C][A...])
//   if Distance[D][A] >= Distance[C][A] then SkippedMove[iterated_Terr][skipped_Terr] = true
void initialize_skip_4air_precals() {
  for (uint src_air = 0; src_air < AIRS_COUNT; src_air++) {
    AirArray dist_src_air = AIR_DIST[src_air];
    // get a list of all valid actions (possible destinations)
    AirArray air_within_4_moves = AIR_WITHIN_X_MOVES[4 - 1][src_air];
    uint air_within_4_moves_count = AIR_WITHIN_X_MOVES_COUNT[4 - 1][src_air];
    for (uint selected_idx = 0; selected_idx < air_within_4_moves_count; selected_idx++) {
      uint dst_air = air_within_4_moves[selected_idx];
      uint dist = dist_src_air[dst_air];
      uint* airs_x_to_4_moves_away = AIRS_X_TO_4_MOVES_AWAY[dist - 1][dst_air];
      uint airs_x_to_4_moves_away_count = AIRS_X_TO_4_MOVES_AWAY_COUNT[dist - 1][dst_air];
      for (uint air_idx = 0; air_idx < airs_x_to_4_moves_away_count; air_idx++) {
        uint other_src_air = airs_x_to_4_moves_away[air_idx];
        // moves start with low number indexes
        if (other_src_air <= src_air) {
          continue;
        }
        AirArray other_src_air_dist = AIR_DIST[other_src_air];
        for (uint skipped_idx = selected_idx + 1; skipped_idx < air_within_4_moves_count;
             skipped_idx++) {
          uint skipped_air = air_within_4_moves[skipped_idx];
          if (other_src_air_dist[skipped_air] >= dist_src_air[skipped_air]) {
            skip_4air_precals[src_air][dst_air][skip_4air_precals_count[src_air][dst_air]++] =
                other_src_air;
          }
        }
      }
    }
  }
}

void load_game_data(char const* filename) {
  memset(&state, 0, sizeof(state));
  memset(&current_player_land_unit_types, 0, sizeof(current_player_land_unit_types));
  memset(&current_player_sea_unit_types, 0, sizeof(current_player_sea_unit_types));
  memset(&total_player_land_units, 0, sizeof(total_player_land_units));
  memset(&total_player_sea_units, 0, sizeof(total_player_sea_units));
  load_game_data_from_json(filename, &state);
  refresh_full_cache();
}
void refresh_full_cache() {
  refresh_economy();
  refresh_land_armies();
  refresh_sea_navies();
  refresh_allies();
  refresh_canals();
  refresh_enemy_armies();
  refresh_fleets();
  refresh_land_path_blocked();
  refresh_sea_path_blocked();
}
void refresh_eot_cache() {
  refresh_allies();
  refresh_canals();
  refresh_enemy_armies();
  refresh_fleets();
  refresh_land_path_blocked();
  refresh_sea_path_blocked();
}
void refresh_economy() {
  for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    income_per_turn[player_idx] = 0;
    total_factory_count[player_idx] = 0;
  }
}
void refresh_land_armies() {
  FILL_2D_ARRAY(current_player_land_unit_types, 0);
  FILL_2D_ARRAY(total_player_land_units, 0);
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    uint land_owner = *owner_idx[land_idx];
    if (*factory_max[land_idx] > 0) {
      factory_locations[land_owner][total_factory_count[land_owner]++] = land_idx;
    }
    income_per_turn[land_owner] += LANDS[land_idx].land_value;
    auto cp_land_unit_types_land = current_player_land_unit_types[land_idx];
    auto land_units_state_land = land_units_state[land_idx];
    for (uint unit_idx = 0; unit_idx < LAND_UNIT_TYPES_COUNT; unit_idx++) {
      uint* cp_land_unit_types_land_unit = &cp_land_unit_types_land[unit_idx];
      uint* land_units_state_land_unit = land_units_state_land[unit_idx];
      uint total_states = STATES_MOVE_LAND[unit_idx];
      for (uint unit_state_idx = 0; unit_state_idx < total_states; unit_state_idx++) {
        *cp_land_unit_types_land_unit += land_units_state_land_unit[unit_state_idx];
      }
    }
    for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
      uint* cur_units_land_player_total = &total_player_land_units[player_idx][land_idx];
      for (uint unit_idx = 0; unit_idx < LAND_UNIT_TYPES_COUNT; unit_idx++) {
        *cur_units_land_player_total +=
            total_player_land_unit_types[player_idx][land_idx][unit_idx];
      }
    }
  }
}
void refresh_sea_navies() {
  FILL_2D_ARRAY(current_player_sea_unit_types, 0);
  FILL_2D_ARRAY(total_player_sea_units, 0);
  for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    for (uint unit_idx = 0; unit_idx < SEA_UNIT_TYPES_COUNT; unit_idx++) {
      uint total_states = STATES_MOVE_SEA[unit_idx];
      for (uint unit_state_idx = 0; unit_state_idx < total_states; unit_state_idx++) {
        current_player_sea_unit_types[sea_idx][unit_idx] +=
            sea_units_state[sea_idx][unit_idx][unit_state_idx];
      }
    }
    for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
      uint* cur_units_sea_player_total = &total_player_sea_units[player_idx][sea_idx];
      auto total_player_sea_unit_types_player = total_player_sea_unit_types[player_idx][sea_idx];
      for (uint unit_idx = 0; unit_idx < SEA_UNIT_TYPES_COUNT - 1; unit_idx++) {
        *cur_units_sea_player_total += total_player_sea_unit_types_player[unit_idx];
      }
    }
  }
}
void refresh_allies() {
  enemies_count_0 = 0;
  for (uint player_idx1 = 0; player_idx1 < PLAYERS_COUNT; player_idx1++) {
    is_allied_0[player_idx1] =
        PLAYERS[state.player_index].is_allied[(state.player_index + player_idx1) % PLAYERS_COUNT];
    if (!is_allied_0[player_idx1]) {
      enemies_0[enemies_count_0++] = player_idx1;
    }
  }
}

void refresh_canals() {
  canal_state = 0;
  for (uint canal_idx = 0; canal_idx < CANALS_COUNT; canal_idx++) {
    if (is_allied_0[*owner_idx[CANALS[canal_idx].lands[0]]] &&
        is_allied_0[*owner_idx[CANALS[canal_idx].lands[1]]]) {
      canal_state += 1U << canal_idx;
    }
  }
  using std::begin;
  using std::copy;
  using std::end;
  COPY_FULL_ARRAY(SEA_DIST[canal_state], sea_dist);
  COPY_FULL_ARRAY(SEA_PATH[canal_state], sea_path);
  COPY_FULL_ARRAY(SEA_PATH_ALT[canal_state], sea_path_alt);
  COPY_FULL_ARRAY(SEAS_WITHIN_1_MOVE[canal_state], seas_within_1_move_canal);
  COPY_FULL_ARRAY(SEAS_WITHIN_2_MOVES[canal_state], seas_within_2_moves_canal);
  COPY_FULL_ARRAY(SEAS_WITHIN_1_MOVE_COUNT[canal_state], seas_within_1_move_count_canal);
  COPY_FULL_ARRAY(SEAS_WITHIN_2_MOVES_COUNT[canal_state], seas_within_2_moves_count_canal);
}
void refresh_enemy_armies() {
  FILL_ARRAY(enemy_units_count, 0);
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    for (uint enemy_idx1 = 0; enemy_idx1 < enemies_count_0; enemy_idx1++) {
      uint ememy_player_idx = enemies_0[enemy_idx1];
      enemy_units_count[land_idx] += total_player_land_units[ememy_player_idx][land_idx];
    }
  }
}
void refresh_fleets() {
  FILL_ARRAY(enemy_destroyers_total, 0);
  FILL_ARRAY(enemy_blockade_total, 0);
  for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    uint air_idx = sea_idx + LANDS_COUNT;
    auto sea_units_0 = current_player_sea_unit_types[sea_idx];
    allied_carriers[sea_idx] = sea_units_0[CARRIERS];
    for (uint player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      auto sea_units = total_player_sea_unit_types[player_idx][sea_idx];
      if (is_allied_0[player_idx]) {
        allied_carriers[sea_idx] += sea_units[CARRIERS];
      } else {
        enemy_units_count[air_idx] += total_player_sea_units[player_idx][sea_idx];
        enemy_destroyers_total[sea_idx] += sea_units[DESTROYERS];
        enemy_blockade_total[sea_idx] += sea_units[DESTROYERS] + sea_units[CARRIERS] +
                                         sea_units[CRUISERS] + sea_units[BATTLESHIPS] +
                                         sea_units[BS_DAMAGED];
      }
    }
    transports_with_large_cargo_space[sea_idx] = sea_units_0[TRANS_EMPTY] + sea_units_0[TRANS_1I];
    transports_with_small_cargo_space[sea_idx] = sea_units_0[TRANS_EMPTY] + sea_units_0[TRANS_1I] +
                                                 sea_units_0[TRANS_1A] + sea_units_0[TRANS_1T];
  }
}
void refresh_land_path_blocked() {
  for (uint src_land = 0; src_land < LANDS_COUNT; src_land++) {
    uint lands_within_2_moves_count = LANDS_WITHIN_2_MOVES_COUNT[src_land];
    auto& lands_within_2_moves = LANDS_WITHIN_2_MOVES[src_land];
    for (uint conn_idx = 0; conn_idx < lands_within_2_moves_count; conn_idx++) {
      uint dst_land = lands_within_2_moves[conn_idx];
      uint nextLandMovement = LAND_PATH[src_land][dst_land];
      uint nextLandMovementAlt = LAND_PATH_ALT[src_land][dst_land];
      is_land_path_blocked[src_land][dst_land] =
          (enemy_units_count[nextLandMovement] > 0 || *factory_max[nextLandMovement] > 0) &&
          (enemy_units_count[nextLandMovementAlt] > 0 || *factory_max[nextLandMovementAlt] > 0);
    }
  }
}
void refresh_sea_path_blocked() {
  for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    //    for (uint dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) { // todo this seems excessive
    uint seas_within_2_moves_count = SEAS_WITHIN_2_MOVES_COUNT[canal_state][src_sea];
    auto& seas_within_2_moves = SEAS_WITHIN_2_MOVES[canal_state][src_sea];
    for (uint conn_idx = 0; conn_idx < seas_within_2_moves_count; conn_idx++) {
      uint dst_sea = seas_within_2_moves[conn_idx];
      uint nextSeaMovement = sea_path[src_sea][dst_sea];
      uint nextSeaMovementAlt = sea_path_alt[src_sea][dst_sea];
      is_sea_path_blocked[src_sea][dst_sea] =
          enemy_blockade_total[nextSeaMovement] > 0 && enemy_blockade_total[nextSeaMovementAlt] > 0;
      is_sub_path_blocked[src_sea][dst_sea] = enemy_destroyers_total[nextSeaMovement] > 0 &&
                                              enemy_destroyers_total[nextSeaMovementAlt] > 0;
    }
  }
}
void set_seed(uint new_seed) {
  seed = new_seed;
  random_number_index = new_seed;
}
#ifdef DEBUG
void play_full_turn() {
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
  // end_turn();
  // debug_checks();
  crash_air_units();
  debug_checks();
  reset_units_fully();
  debug_checks();
  buy_factory();
  debug_checks();
  collect_money();
  rotate_turns();
}
#else
void play_full_turn() {
  move_fighter_units();
  setPrintableStatus();
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
}
#endif
void cause_breakpoint() { printf("\nbreakpoint\n"); }
#ifdef DEBUG
void debug_checks() {
  step_id++;
  if (step_id == 1999998819) {
    actually_print = true;
  }
  if (actually_print) {
    printf("iter: %d  loops: %d  step_id: %d  seed: %d  answers_remaining: %d  select_action: %d\n",
           MCTS_ITERATIONS, max_loops, step_id, seed, answers_remaining, use_selected_action);
  }
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    for (uint unit_idx = 0; unit_idx < LAND_UNIT_TYPES_COUNT; unit_idx++) {
      uint temp_unit_type_total = 0;
      for (uint cur_unit_state = 0; cur_unit_state < STATES_MOVE_LAND[unit_idx]; cur_unit_state++) {
        temp_unit_type_total += land_units_state[land_idx][unit_idx][cur_unit_state];
      }
      if (temp_unit_type_total != current_player_land_unit_types[land_idx][unit_idx]) {
        printf("temp_unit_type_total %d != current_player_land_unit_types[land_idx][unit_idx] %d",
               temp_unit_type_total, current_player_land_unit_types[land_idx][unit_idx]);
        cause_breakpoint();
      }
    }
  }
  for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    for (uint unit_idx = 0; unit_idx < SEA_UNIT_TYPES_COUNT; unit_idx++) {
      uint temp_unit_type_total = 0;
      for (uint cur_unit_state = 0; cur_unit_state < STATES_MOVE_SEA[unit_idx]; cur_unit_state++) {
        temp_unit_type_total += sea_units_state[sea_idx][unit_idx][cur_unit_state];
      }
      if (temp_unit_type_total != current_player_sea_unit_types[sea_idx][unit_idx]) {
        printf("temp_unit_type_total %d != current_player_sea_unit_types[%d][%d] %d",
               temp_unit_type_total, sea_idx, unit_idx,
               current_player_sea_unit_types[sea_idx][unit_idx]);
        cause_breakpoint();
      }
    }
  }
  for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
      uint total = 0;
      uint total_data = 0;
      for (uint unit_idx = 0; unit_idx < LAND_UNIT_TYPES_COUNT; unit_idx++) {
        if (player_idx == 0 || unit_idx < LAND_UNIT_TYPES_COUNT)
          total += total_player_land_unit_types[player_idx][land_idx][unit_idx];
        if (player_idx == 0) {
          total_data += current_player_land_unit_types[land_idx][unit_idx];
        } else {
          if (unit_idx < LAND_UNIT_TYPES_COUNT)
            total_data += state.other_land_units[player_idx - 1][land_idx][unit_idx];
        }
      }
      if (total != total_player_land_units[player_idx][land_idx] || total != total_data) {
        printf("total %d != total_player_land_units[player_idx][land_idx] %d != total_data %d",
               total, total_player_land_units[player_idx][land_idx], total_data);
        cause_breakpoint();
      }

      if (total_player_land_units[player_idx][land_idx] < 0 ||
          total_player_land_units[player_idx][land_idx] > 900) {
        printf("units_land_player_total[player_idx][land_idx] < 0");
        cause_breakpoint();
      }
      if (enemy_units_count[land_idx] < 0 || enemy_units_count[land_idx] > 500) {
        printf("enemy_units_count[land_idx] < 0");
        cause_breakpoint();
      }
      total = 0;
      for (uint unit_idx = 0; unit_idx < LAND_UNIT_TYPES_COUNT; unit_idx++) {
        if (total_player_land_unit_types[player_idx][land_idx][unit_idx] < 0 ||
            total_player_land_unit_types[player_idx][land_idx][unit_idx] > 240) {
          printf("total_player_land_unit_types[player_idx][land_idx][unit_idx] < 0");
          cause_breakpoint();
        }
        total += total_player_land_unit_types[player_idx][land_idx][unit_idx];
      }
      if (total != total_player_land_units[player_idx][land_idx]) {
        printf("total %d != total_player_land_units[player_idx][land_idx] %d", total,
               total_player_land_units[player_idx][land_idx]);
        cause_breakpoint();
      }
      uint enemy_total = 0;
      for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
        uint enemy_player_idx = enemies_0[enemy_idx];
        enemy_total += total_player_land_units[enemy_player_idx][land_idx];
      }
      if (enemy_total != enemy_units_count[land_idx]) {
        printf("enemy_total %d != enemy_units_count[land_idx] %d", enemy_total,
               enemy_units_count[land_idx]);
        cause_breakpoint();
      }
    }
    for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
      if (total_player_sea_units[player_idx][sea_idx] < 0 ||
          total_player_sea_units[player_idx][sea_idx] > 240) {
        printf("units_sea_player_total[player_idx][sea_idx] < 0");
        cause_breakpoint();
      }
      for (uint unit_idx = 0; unit_idx < SEA_UNIT_TYPES_COUNT; unit_idx++) {
        if (total_player_sea_unit_types[player_idx][sea_idx][unit_idx] < 0 ||
            total_player_sea_unit_types[player_idx][sea_idx][unit_idx] > 240) {
          printf("total_player_sea_unit_types[player_idx][sea_idx][unit_idx] < 0");
          cause_breakpoint();
        }
      }
      if (enemy_units_count[sea_idx + LANDS_COUNT] < 0 ||
          enemy_units_count[sea_idx + LANDS_COUNT] > 240) {
        printf("enemy_units_count[land_idx] < 0");
        cause_breakpoint();
      }
      uint total = 0;
      uint total_data = 0;
      for (uint unit_idx = 0; unit_idx < SEA_UNIT_TYPES_COUNT; unit_idx++) {
        if (player_idx == 0 || unit_idx < SEA_UNIT_TYPES_COUNT - 1)
          total += total_player_sea_unit_types[player_idx][sea_idx][unit_idx];
        if (player_idx == 0) {
          total_data += current_player_sea_unit_types[sea_idx][unit_idx];
        } else {
          if (unit_idx < SEA_UNIT_TYPES_COUNT - 1)
            total_data += state.other_sea_units[player_idx - 1][sea_idx][unit_idx];
        }
      }
      if (total != total_player_sea_units[player_idx][sea_idx] || total != total_data) {
        printf("total %d != total_player_sea_units[player_idx %d][sea_idx %d] %d != total_data %d",
               total, player_idx, sea_idx, total_player_sea_units[player_idx][sea_idx], total_data);
        cause_breakpoint();
      }
      uint enemy_total = 0;
      for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
        uint enemy_player_idx = enemies_0[enemy_idx];
        enemy_total += total_player_sea_units[enemy_player_idx][sea_idx];
      }
      if (enemy_total != enemy_units_count[sea_idx + LANDS_COUNT]) {
        printf("enemy_total %d != enemy_units_count[sea_idx + LANDS_COUNT] %d", enemy_total,
               enemy_units_count[sea_idx + LANDS_COUNT]);
        cause_breakpoint();
      }
    }
  }
}
#else
void debug_checks() {}
#endif
void setPrintableStatus() {
#ifdef DEBUG
  debug_checks();
  char threeCharStr[4];
  // set printableGameStatus to ""
  //-exec watch total_player_sea_units[0][0] if total_player_sea_units[0][0] > 240
  //-exec watch total_player_sea_units[0][0] if total_player_sea_units[0][0] < 0
  printableGameStatus[0] = '\0';
  if (actually_print) {
    strcat(printableGameStatus, "---\n");
    setPrintableStatusLands();
    setPrintableStatusSeas();
    strcat(printableGameStatus, "\n");
    strcat(printableGameStatus, PLAYERS[state.player_index].color);
    strcat(printableGameStatus, PLAYERS[state.player_index].name);
    strcat(printableGameStatus, "\033[0m");
    strcat(printableGameStatus, ": ");
    sprintf(threeCharStr, "%d", state.money[0]);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, " IPC\n");
  }
#endif
}
#ifdef DEBUG
void setPrintableStatusLands() {
  char threeCharStr[6];
  char paddedStr[32];
  char* my_color = PLAYERS[state.player_index].color;
  char* my_name = PLAYERS[state.player_index].name;
  uint player_index = state.player_index;
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    //    LandState land_state = gameData.land_state[i];
    uint land_owner = (*owner_idx[land_idx] + player_index) % PLAYERS_COUNT;
    strcat(printableGameStatus, PLAYERS[land_owner].color);
    sprintf(threeCharStr, "%d ", land_idx);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, LANDS[land_idx].name);
    strcat(printableGameStatus, ": ");
    strcat(printableGameStatus, PLAYERS[land_owner].name);
    strcat(printableGameStatus, " ");
    sprintf(threeCharStr, "%d", state.builds_left[land_idx]);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, "/");
    sprintf(threeCharStr, "%d", *factory_hp[land_idx]);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, "/");
    sprintf(threeCharStr, "%d", *factory_max[land_idx]);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, "/");
    sprintf(threeCharStr, "%d", LAND_VALUE[land_idx]);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, " Combat:");
    if (state.flagged_for_combat[land_idx] > 0) {
      strcat(printableGameStatus, "true\n");
    } else {
      strcat(printableGameStatus, "false\n");
    }
    uint grand_total = 0;
    for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
      grand_total += total_player_land_units[player_idx][land_idx];
    }
    if (grand_total == 0) {
      strcat(printableGameStatus, "\033[0m");
      continue;
    }
    strcat(printableGameStatus, "                 |Tot| 0| 1| 2| 3| 4| 5| 6|\n");
    if (total_player_land_units[0][land_idx] > 0) {
      strcat(printableGameStatus, my_color);
      auto other_land_units_here = current_player_land_unit_types[land_idx];
      for (uint land_unit_idx = 0; land_unit_idx < LAND_UNIT_TYPES_COUNT; land_unit_idx++) {
        uint unit_count = other_land_units_here[land_unit_idx];
        if (unit_count > 0) {
          strcat(printableGameStatus, my_name);
          strcat(printableGameStatus, " ");
          sprintf(paddedStr, "%-14s", NAMES_UNIT_LAND[land_unit_idx]);
          strcat(printableGameStatus, paddedStr);
          sprintf(threeCharStr, "%3d", unit_count);
          strcat(printableGameStatus, threeCharStr);
          uint* units_here = land_units_state[land_idx][land_unit_idx];
          for (uint cur_state = 0; cur_state < STATES_MOVE_LAND[land_unit_idx]; cur_state++) {
            sprintf(threeCharStr, "%3d", units_here[cur_state]);
            strcat(printableGameStatus, threeCharStr);
          }
          strcat(printableGameStatus, "\n");
        }
      }
    }
    strcat(printableGameStatus, "\033[0m");
    for (uint player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      if (total_player_land_units[player_idx][land_idx] == 0)
        continue;
      uint current_player_idx = (player_index + player_idx) % PLAYERS_COUNT;
      strcat(printableGameStatus, PLAYERS[current_player_idx].color);
      auto other_land_units_here = total_player_land_unit_types[player_idx][land_idx];
      for (uint land_unit_idx = 0; land_unit_idx < LAND_UNIT_TYPES_COUNT; land_unit_idx++) {
        uint unit_count = other_land_units_here[land_unit_idx];
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
#else
void setPrintableStatusLands() {}
#endif
void setPrintableStatusSeas() {
  char threeCharStr[4];
  char paddedStr[32];
  for (uint sea_index = 0; sea_index < SEAS_COUNT; sea_index++) {
    uint grand_total = 0;
    for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
      grand_total += total_player_sea_units[player_idx][sea_index];
    }
    if (grand_total == 0) {
      continue;
    }
    UnitsSea units_sea = state.units_sea[sea_index];
    sprintf(threeCharStr, "%d ", LANDS_COUNT + sea_index);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, SEAS[sea_index].name);
    strcat(printableGameStatus, " Combat:");
    if (state.flagged_for_combat[sea_index + LANDS_COUNT] > 0) {
      strcat(printableGameStatus, "true\n");
    } else {
      strcat(printableGameStatus, "false\n");
    }
    strcat(printableGameStatus, "                 |Tot| 0| 1| 2| 3| 4| 5| 6|\n");
    strcat(printableGameStatus, PLAYERS[state.player_index].color);
    if (total_player_sea_units[0][sea_index] > 0) {
      for (uint sea_unit_idx = 0; sea_unit_idx < SEA_UNIT_TYPES_COUNT; sea_unit_idx++) {
        uint unit_count = current_player_sea_unit_types[sea_index][sea_unit_idx];
        if (unit_count > 0) {
          strcat(printableGameStatus, PLAYERS[state.player_index].name);
          strcat(printableGameStatus, " ");
          sprintf(paddedStr, "%-14s", NAMES_UNIT_SEA[sea_unit_idx]);
          strcat(printableGameStatus, paddedStr);
          sprintf(threeCharStr, "%3d", unit_count);
          strcat(printableGameStatus, threeCharStr);
          for (uint cur_state = 0; cur_state < STATES_MOVE_SEA[sea_unit_idx]; cur_state++) {
            sprintf(threeCharStr, "%3d", sea_units_state[sea_index][sea_unit_idx][cur_state]);
            strcat(printableGameStatus, threeCharStr);
          }
          strcat(printableGameStatus, "\n");
        }
      }
    }
    strcat(printableGameStatus, "\033[0m");
    for (uint player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      if (total_player_sea_units[player_idx][sea_index] == 0)
        continue;
      strcat(printableGameStatus, PLAYERS[(state.player_index + player_idx) % PLAYERS_COUNT].color);
      for (uint sea_unit_idx = 0; sea_unit_idx < SEA_UNIT_TYPES_COUNT - 1; sea_unit_idx++) {
        uint unit_count = total_player_sea_unit_types[player_idx][sea_index][sea_unit_idx];
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
uint getUserInput() {
  char buffer[4]; // Buffer to hold input string (3 digits + null terminator)
  uint user_input = 0;
  while (true) {
    // 0-valid_moves_count
    // printf("Enter a number between 0 and 255: ");
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
      if (sscanf(buffer, "%d", &user_input) == 1 && user_input >= 0) {
        for (uint i = 0; i < valid_moves_count; i++) {
          if (user_input == valid_moves[i]) {
            return (uint)user_input;
          }
        }
        return valid_moves[0];
      }
    }
  }
}
#ifdef DEBUG
uint getAIInput() {
  answers_remaining--;
  if (use_selected_action) {
    // printf("selecting action %d\n", selected_action);
    bool found = false;
    for (uint i = 0; i < valid_moves_count; i++) {
      if (valid_moves[i] == selected_action) {
        found = true;
        break;
      }
    }
    if (!found) {
      printf("selected action %d not found in valid_moves\n", selected_action);
      cause_breakpoint();
    }
    return selected_action;
  }
  // printf("selecting random action %d\n", RANDOM_NUMBERS[random_number_index]);
  return valid_moves[RANDOM_NUMBERS[random_number_index++] % valid_moves_count];
}
#else
uint getAIInput() {
  answers_remaining--;
  if (use_selected_action) {
    return selected_action;
  }
  return valid_moves[RANDOM_NUMBERS[random_number_index++] % valid_moves_count];
}
#endif

void update_move_history_4air(uint src_air, uint dst_air) {
  // get a list of newly skipped valid_actions
#ifdef DEBUG
  uint valid_moves_idx = valid_moves_count;
#endif
  while (true) {
    uint valid_action = valid_moves[valid_moves_count];
    if (valid_action == dst_air) {
      break;
    }
#ifdef DEBUG
    if (valid_moves_count == 0) {
      printf("valid_moves_count < 0\n");
      cause_breakpoint();
    }
#endif
    state.skipped_moves[src_air][valid_action].bit = true;
    apply_skip(src_air, valid_action);
    valid_moves_count--;
  }
}

void apply_skip(uint src_air, uint dst_air) {
  for (uint i = 0; i < AIRS_COUNT; i++) {
    if (state.skipped_moves[dst_air][i].bit) {
      state.skipped_moves[src_air][i].bit = true;
    }
  }
}

void clear_move_history() {
  for (auto& move : state.skipped_moves) {
      for(auto& bit : move) {
          bit.bit = false;
      }
  }  // FILL_2D_ARRAY(state.skipped_moves, 0);
}

uint get_user_purchase_input(uint src_air) {
  if (PLAYERS[state.player_index].is_human) {
    char stringBuffer[32];
    setPrintableStatus();
    strcat(printableGameStatus, "Purchasing at ");
    if (src_air < LANDS_COUNT) {
      strcat(printableGameStatus, LANDS[src_air].name);
      sprintf(stringBuffer, " %d=Finished ", valid_moves[0]);
      strcat(printableGameStatus, stringBuffer);
      for (uint i = 1; i < valid_moves_count; i++) {
        sprintf(stringBuffer, "%d=%s ", valid_moves[i], NAMES_UNIT_LAND[valid_moves[i]]);
        strcat(printableGameStatus, stringBuffer);
      }
    } else {
      strcat(printableGameStatus, SEAS[src_air - LANDS_COUNT].name);
      sprintf(stringBuffer, " %d=Finished ", valid_moves[0]);
      strcat(printableGameStatus, stringBuffer);
      for (uint i = 1; i < valid_moves_count; i++) {
        sprintf(stringBuffer, "%d=%s ", valid_moves[i], NAMES_UNIT_SEA[valid_moves[i]]);
        strcat(printableGameStatus, stringBuffer);
      }
    }
    printf("%s\n", printableGameStatus);
    return getUserInput();
  }
  return getAIInput();
}
uint get_user_move_input(uint unit_type, uint src_air) {
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
    for (uint i = 0; i < valid_moves_count; i++) {
      char threeCharStr[6];
      sprintf(threeCharStr, "%d ", valid_moves[i]);
      strcat(printableGameStatus, threeCharStr);
    }
    printf("%s\n", printableGameStatus);
    return getUserInput();
  }
  return getAIInput();
}
bool load_transport(uint unit_type, uint src_land, uint dst_sea, uint land_unit_state) {
#ifdef DEBUG
  if (actually_print) {
    printf("load_transport: unit_type=%d src_land=%d dst_sea=%d land_unit_state=%d\n", unit_type,
           src_land, dst_sea, land_unit_state);
  }
#endif
  auto load_unit_type = LOAD_UNIT_TYPE[unit_type];
  auto units_sea_ptr_dst_sea = sea_units_state[dst_sea];
  for (uint trans_type = (UNIT_WEIGHTS[unit_type] > 2) ? TRANS_1I : TRANS_1T;
       trans_type >= TRANS_EMPTY; trans_type--) {
    uint* units_sea_ptr_dst_sea_trans_type = units_sea_ptr_dst_sea[trans_type];
    uint states_unloading = STATES_UNLOADING[trans_type];
    for (uint trans_state = STATES_MOVE_SEA[trans_type] - STATES_STAGING[trans_type];
         trans_state >= states_unloading && trans_state != 255;
         trans_state--) { // TODO - fix more underflows
      if (units_sea_ptr_dst_sea_trans_type[trans_state] > 0) {
        uint new_trans_type = load_unit_type[trans_type];
        units_sea_ptr_dst_sea[trans_type][trans_state]--;
        if (trans_type == TRANS_EMPTY && trans_state == TRANS_EMPTY_UNLOADING_STATES) {
          trans_state = STATES_UNLOADING[new_trans_type]; // empty transports doesn't have an
                                                          // "unloading" state
        }
        units_sea_ptr_dst_sea[new_trans_type][trans_state]++;
        current_player_sea_unit_types[dst_sea][trans_type]--;
        current_player_sea_unit_types[dst_sea][new_trans_type]++;
        total_player_land_units[0][src_land]--;
        current_player_land_unit_types[src_land][unit_type]--;
        land_units_state[src_land][unit_type][land_unit_state]--;
        transports_with_large_cargo_space[dst_sea] =
            current_player_sea_unit_types[dst_sea][TRANS_EMPTY] +
            current_player_sea_unit_types[dst_sea][TRANS_1I];
        transports_with_small_cargo_space[dst_sea] =
            transports_with_large_cargo_space[dst_sea] +
            current_player_sea_unit_types[dst_sea][TRANS_1A] +
            current_player_sea_unit_types[dst_sea][TRANS_1T];
#ifdef DEBUG
        debug_checks();
#endif
        return true;
      }
    }
  }
  printf("Error: Failed to load transport\n");
  cause_breakpoint();
  return false;
}

void add_valid_land_moves(uint src_land, uint moves_remaining, uint unit_type) {
  if (moves_remaining == 2) {
    // check for moving from land to land (two moves away)
    uint lands_within_2_moves_count = LANDS_WITHIN_2_MOVES_COUNT[src_land];
    LandArray lands_within_2_moves = LANDS_WITHIN_2_MOVES[src_land];
    AirArray land_dist = LAND_DIST[src_land];
    auto is_land_path_blocked_src_land = is_land_path_blocked[src_land];
    for (uint land_idx = 0; land_idx < lands_within_2_moves_count; land_idx++) {
      uint dst_land = lands_within_2_moves[land_idx];
      if (state.skipped_moves[src_land][dst_land].bit) {
        continue;
      }
      if (land_dist[dst_land] == 2 && is_land_path_blocked_src_land[dst_land]) {
        continue;
      }
      valid_moves[valid_moves_count++] = dst_land;
    }
    // check for moving from land to sea (two moves away)
    uint load_within_2_moves_count = LOAD_WITHIN_2_MOVES_COUNT[src_land];
    SeaArray load_within_2_moves = LOAD_WITHIN_2_MOVES[src_land];
    for (uint sea_idx = 0; sea_idx < load_within_2_moves_count; sea_idx++) {
      uint dst_sea = load_within_2_moves[sea_idx];
      if (transports_with_large_cargo_space[dst_sea] == 0) { // assume large, only tanks move 2
        continue;
      }
      uint dst_air = dst_sea + LANDS_COUNT;
      if (state.skipped_moves[src_land][dst_air].bit) {
        continue;
      }
      if (land_dist[dst_air] == 1 && is_land_path_blocked_src_land[dst_air]) {
        continue;
      }
      valid_moves[valid_moves_count++] = dst_air;
    }
  } else {
    // check for moving from land to land (one move away)
    uint lands_within_1_move_count = LAND_TO_LAND_COUNT[src_land];
    L2LConn lands_within_1_move = LAND_TO_LAND_CONN[src_land];
    bool is_non_combat_unit = ATTACK_UNIT_LAND[unit_type] == 0;
    bool is_unloadable_unit = UNIT_WEIGHTS[unit_type] > 5;
    bool is_heavy_unit = UNIT_WEIGHTS[unit_type] > 2;
    for (uint land_idx = 0; land_idx < lands_within_1_move_count; land_idx++) {
      uint dst_land = lands_within_1_move[land_idx];
      if (state.skipped_moves[src_land][dst_land].bit) {
        continue;
      }
      if (is_non_combat_unit && !is_allied_0[*owner_idx[dst_land]]) {
        continue;
      }
      valid_moves[valid_moves_count++] = dst_land;
    }
    // check for moving from land to sea (one move away)
    if (is_unloadable_unit)
      return;
    uint land_to_sea_count = LAND_TO_SEA_COUNT[src_land];
    L2SConn land_to_sea_conn = LAND_TO_SEA_CONN[src_land];
    for (uint sea_idx = 0; sea_idx < land_to_sea_count; sea_idx++) {
      uint dst_sea = land_to_sea_conn[sea_idx];
      if (transports_with_small_cargo_space[dst_sea] == 0) {
        continue;
      }
      if (is_heavy_unit && transports_with_large_cargo_space[dst_sea] == 0) {
        continue;
      }
      uint dst_air = dst_sea + LANDS_COUNT;
      if (state.skipped_moves[src_land][dst_air].bit) {
        continue;
      }
      valid_moves[valid_moves_count++] = dst_air;
    }
  }
}

void add_valid_sea_moves(uint src_sea, uint moves_remaining) {
  if (moves_remaining == 2) {
    // check for moving from sea to sea (two moves away)
    uint seas_within_2_moves_count = SEAS_WITHIN_2_MOVES_COUNT[canal_state][src_sea];
    SeaArray seas_within_2_moves =
        SEAS_WITHIN_2_MOVES[canal_state][src_sea]; // TODO optimize canal_state
    auto is_sea_path_blocked_src_sea = is_sea_path_blocked[src_sea];
    for (uint sea_idx = 0; sea_idx < seas_within_2_moves_count; sea_idx++) {
      uint dst_sea = seas_within_2_moves[sea_idx];
      if (is_sea_path_blocked_src_sea[dst_sea]) {
        continue;
      }
      uint src_air = src_sea + LANDS_COUNT;
      uint dst_air = dst_sea + LANDS_COUNT;
      if (state.skipped_moves[src_air][dst_air].bit) {
        continue;
      }
      valid_moves[valid_moves_count++] = dst_air;
    }
  } else {
    // check for moving from sea to sea (one move away)
    uint seas_within_1_move_count = SEAS_WITHIN_1_MOVE_COUNT[canal_state][src_sea];
    SeaArray seas_within_1_move =
        SEAS_WITHIN_1_MOVE[canal_state][src_sea]; // TODO optimize canal_state
    for (uint sea_idx = 0; sea_idx < seas_within_1_move_count; sea_idx++) {
      uint dst_sea = seas_within_1_move[sea_idx];
      uint src_air = src_sea + LANDS_COUNT;
      uint dst_air = dst_sea + LANDS_COUNT;
      if (state.skipped_moves[src_air][dst_air].bit) {
        continue;
      }
      valid_moves[valid_moves_count++] = dst_air;
    }
  }
}

void add_valid_sub_moves(uint src_sea, uint moves_remaining) {
  if (moves_remaining == 2) {
    // check for moving from sea to sea (two moves away)
    uint seas_within_2_moves_count = SEAS_WITHIN_2_MOVES_COUNT[canal_state][src_sea];
    SeaArray seas_within_2_moves =
        SEAS_WITHIN_2_MOVES[canal_state][src_sea]; // TODO optimize canal_state
    auto is_sub_path_blocked_src_sea = is_sub_path_blocked[src_sea];
    for (uint sea_idx = 0; sea_idx < seas_within_2_moves_count; sea_idx++) {
      uint dst_sea = seas_within_2_moves[sea_idx];
      if (is_sub_path_blocked_src_sea[dst_sea]) {
        continue;
      }
      uint src_air = src_sea + LANDS_COUNT;
      uint dst_air = dst_sea + LANDS_COUNT;
      if (state.skipped_moves[src_air][dst_air].bit) {
        continue;
      }
      valid_moves[valid_moves_count++] = dst_air;
    }
  } else {
    // check for moving from sea to sea (one move away)
    uint seas_within_1_move_count = SEAS_WITHIN_1_MOVE_COUNT[canal_state][src_sea];
    SeaArray seas_within_1_move =
        SEAS_WITHIN_1_MOVE[canal_state][src_sea]; // TODO optimize canal_state
    for (uint sea_idx = 0; sea_idx < seas_within_1_move_count; sea_idx++) {
      uint dst_sea = seas_within_1_move[sea_idx];
      uint src_air = src_sea + LANDS_COUNT;
      uint dst_air = dst_sea + LANDS_COUNT;
      if (state.skipped_moves[src_air][dst_air].bit) {
        continue;
      }
      valid_moves[valid_moves_count++] = dst_air;
    }
  }
}

bool stage_transport_units() {
  bool units_to_process = false;
  // loop through transports with "3" moves remaining (that aren't full),
  // start at sea 0 to n
  // TODO: optimize with cache - only loop through regions with transports
  for (uint unit_type = TRANS_EMPTY; unit_type <= TRANS_1T; unit_type++) {
    uint staging_state = STATES_MOVE_SEA[unit_type] - 1;
    uint done_staging = staging_state - 1;
    // clear_move_history();
    //  TODO CHECKPOINT
    for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      uint* total_ships = &sea_units_state[src_sea][unit_type][staging_state];
      if (*total_ships == 0)
        continue;
      uint src_air = src_sea + LANDS_COUNT;
      valid_moves[0] = src_air;
      valid_moves_count = 1;
      add_valid_sea_moves(src_sea, 2);
      uint* units_sea_ptr_src_sea_unit_type = sea_units_state[src_sea][unit_type];
      while (*total_ships > 0) {
        units_to_process = true;
        uint dst_air = valid_moves[0];
        if (valid_moves_count > 1) {
          if (answers_remaining == 0)
            return true;
          dst_air = get_user_move_input(unit_type, src_air);
        }
#ifdef DEBUG
        if (actually_print) {
          printf("stage_transport_units: unit_type=%d src_air=%d dst_air=%d\n", unit_type, src_air,
                 dst_air);
          setPrintableStatus();
          printf("%s\n", printableGameStatus);
        }
#endif
        // update_move_history(dst_air, src_sea);
        if (src_air == dst_air) {
          units_sea_ptr_src_sea_unit_type[done_staging] += *total_ships;
          *total_ships = 0;
          continue;
        }
        uint dst_sea = dst_air - LANDS_COUNT;
        uint sea_distance = sea_dist[src_sea][dst_air];
        if (enemy_blockade_total[dst_sea] > 0) {
          state.flagged_for_combat[dst_air] = 2;
          sea_distance = MAX_MOVE_SEA[unit_type];
          continue;
        }
        sea_units_state[dst_sea][unit_type][done_staging - sea_distance]++;
        current_player_sea_unit_types[dst_sea][unit_type]++;
        total_player_sea_units[0][dst_sea]++;
        transports_with_small_cargo_space[dst_sea]++;
        sea_units_state[src_sea][unit_type][staging_state]--;
        current_player_sea_unit_types[src_sea][unit_type]--;
        total_player_sea_units[0][src_sea]--;
        transports_with_small_cargo_space[src_sea]--;
        if (unit_type <= TRANS_1I) {
          transports_with_large_cargo_space[src_sea]--;
          transports_with_large_cargo_space[dst_sea]++;
        }
      }
    }
  }
  if (units_to_process) {
    clear_move_history();
  }
  return false;
}

void pre_move_fighter_units() {
  FILL_ARRAY(canFighterLandHere, 0);
#ifdef DEBUG
  if (actually_print) {
    setPrintableStatus();
    printf("%s\n", printableGameStatus);
    if (enemy_units_count[2] == -1) {
      printf("DEBUG: enemy_units_count[2] == -1\n");
    }
  }
#endif
  //  clear_move_history();
  // refresh_canFighterLandHere
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    uint land_owner = *owner_idx[land_idx];
    // is allied owned and not recently conquered?
    canFighterLandHere[land_idx] =
        (is_allied_0[land_owner] && state.flagged_for_combat[land_idx] == 0);
    // check for possiblity to build carrier under fighter
    if (land_owner == state.player_index && *factory_max[land_idx] > 0) {
      uint land_to_sea_count = LAND_TO_SEA_COUNT[land_idx];
      L2SConn land_to_sea_conn = LAND_TO_SEA_CONN[land_idx];
      for (uint conn_idx = 0; conn_idx < land_to_sea_count; conn_idx++) {
        canFighterLandHere[LANDS_COUNT + land_to_sea_conn[conn_idx]] = true;
      }
    }
  }
  for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    if (allied_carriers[sea_idx] > 0) {
      canFighterLandHere[sea_idx + LANDS_COUNT] = true;
      // if player owns these carriers, then landing area is 2 spaces away
      if (sea_units_state[sea_idx][CARRIERS][CARRIER_MOVES_MAX] > 0) {
        S2SConn sea_to_sea_conn = SEA_TO_SEA_CONN[sea_idx];
        uint sea_to_sea_count = SEA_TO_SEA_COUNT[sea_idx];
        for (uint conn_idx = 0; conn_idx < sea_to_sea_count; conn_idx++) {
          uint connected_sea1 = sea_to_sea_conn[conn_idx];
          canFighterLandHere[LANDS_COUNT + connected_sea1] = true;
          uint sea_to_sea_count2 = SEA_TO_SEA_COUNT[connected_sea1];
          S2SConn sea_to_sea_conn2 = SEA_TO_SEA_CONN[connected_sea1];
          for (uint conn2_idx = 0; conn2_idx < sea_to_sea_count2; conn2_idx++) {
            canFighterLandHere[LANDS_COUNT + sea_to_sea_conn2[conn2_idx]] = true;
          }
        }
      }
    }
  }
  // refresh_canFighterLandIn1Move
  for (uint air_idx = 0; air_idx < AIRS_COUNT; air_idx++) {
    canFighterLandIn1Move[air_idx] = false;
    uint air_conn_count = AIR_CONN_COUNT[air_idx];
    for (uint conn_idx = 0; conn_idx < air_conn_count; conn_idx++) {
      if (canFighterLandHere[AIR_CONNECTIONS[air_idx][conn_idx]]) {
        canFighterLandIn1Move[air_idx] = true;
        break;
      }
    }
  }
}
bool move_fighter_units() {
#ifdef DEBUG
  if (actually_print) {
    printf("move_fighter_units\n");
  }
#endif
  bool units_to_process = false;
  for (uint src_air = 0; src_air < AIRS_COUNT; src_air++) {
    uint* total_fighters = &air_units_state[src_air][FIGHTERS][FIGHTER_MOVES_MAX];
    if (*total_fighters == 0) {
      continue;
    }
    if (!units_to_process) {
      units_to_process = true;
      pre_move_fighter_units();
    }
    valid_moves[0] = src_air;
    valid_moves_count = 1;
    add_valid_fighter_moves(src_air, FIGHTER_MOVES_MAX);
    while (*total_fighters > 0) {
      units_to_process = true;
      uint dst_air = valid_moves[0];
      if (valid_moves_count > 1) {
        if (answers_remaining == 0)
          return true;
        dst_air = get_user_move_input(FIGHTERS, src_air);
      }
#ifdef DEBUG
      if (actually_print) {
        setPrintableStatus();
        printf("%s\n", printableGameStatus);
        printf("DEBUG: player: %s moving fighters %d, src_air: %d, dst_air: %d\n",
               PLAYERS[state.player_index].name, FIGHTERS, src_air, dst_air);
      }
#endif
      // update_move_history(dst_air, src_air);
      update_move_history_4air(src_air, dst_air);
      if (src_air == dst_air) {
        air_units_state[src_air][FIGHTERS][0] += *total_fighters;
        *total_fighters = 0;
        continue;
      }
      uint airDistance = AIR_DIST[src_air][dst_air];
      if (dst_air < LANDS_COUNT) {
        if (!is_allied_0[*owner_idx[dst_air]]) {
#ifdef DEBUG
          if (actually_print) {
            printf("Fighter moving to enemy territory. Automatically flagging for combat\n");
          }
#endif
          state.flagged_for_combat[dst_air] = 2;
          // assuming enemy units are present based on valid moves
        } else {
          airDistance = 4; // use up all moves if this is a friendly rebase
        }
      } else {
#ifdef DEBUG
        if (actually_print) {
          printf("Fighter moving to sea. Possibly flagging for combat\n");
        }
#endif
        state.flagged_for_combat[dst_air] = enemy_units_count[dst_air] > 0;
      }
      if (dst_air < LANDS_COUNT) {
        land_units_state[dst_air][FIGHTERS][FIGHTER_MOVES_MAX - airDistance]++;
        current_player_land_unit_types[dst_air][FIGHTERS]++;
        total_player_land_units[0][dst_air]++;
      } else {
        uint dst_sea = dst_air - LANDS_COUNT;
        sea_units_state[dst_sea][FIGHTERS][FIGHTER_MOVES_MAX - airDistance]++;
        current_player_sea_unit_types[dst_sea][FIGHTERS]++;
        total_player_sea_units[0][dst_sea]++;
      }
      if (src_air < LANDS_COUNT) {
        current_player_land_unit_types[src_air][FIGHTERS]--;
        total_player_land_units[0][src_air]--;
      } else {
        uint src_sea = src_air - LANDS_COUNT;
        current_player_sea_unit_types[src_sea][FIGHTERS]--;
        total_player_sea_units[0][src_sea]--;
      }
      *total_fighters -= 1;
    }
  }
  if (units_to_process) {
    clear_move_history();
  }
  return false;
}

bool move_bomber_units() {
  // check if any bombers have full moves remaining
  bool units_to_process = false;
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    // is allied owned and not recently conquered?
    canBomberLandHere[land_idx] =
        (is_allied_0[*owner_idx[land_idx]] && state.flagged_for_combat[land_idx] == 0);
  }
  //  refresh_canBomberLandIn1Move
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    canBomberLandIn1Move[land_idx] = false;
    uint land_conn_count = LANDS[land_idx].land_conn_count;
    auto connected_land_index = LANDS[land_idx].land_conns;
    for (uint conn_idx = 0; conn_idx < land_conn_count; conn_idx++) {
      if (canBomberLandHere[connected_land_index[conn_idx]]) {
        canBomberLandIn1Move[land_idx] = true;
        break;
      }
    }
  }
  for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    canBomberLandIn1Move[LANDS_COUNT + sea_idx] = false;
    uint land_conn_count = SEAS[sea_idx].land_conn_count;
    auto connected_land_index = SEAS[sea_idx].land_conns;
    for (uint conn_idx = 0; conn_idx < land_conn_count; conn_idx++) {
      if (canBomberLandHere[connected_land_index[conn_idx]]) {
        canBomberLandIn1Move[LANDS_COUNT + sea_idx] = true;
        break;
      }
    }
  }
  // refresh_canBomberLandIn2Moves
  for (uint air_idx = 0; air_idx < AIRS_COUNT; air_idx++) {
    canBomberLandIn2Moves[air_idx] = false;
    uint air_conn_count = AIR_CONN_COUNT[air_idx];
    A2AConn air_conn = AIR_CONNECTIONS[air_idx];
    for (uint conn_idx = 0; conn_idx < air_conn_count; conn_idx++) {
      if (canBomberLandIn1Move[air_conn[conn_idx]]) {
        canBomberLandIn2Moves[air_idx] = true;
        break;
      }
    }
  }
  for (uint src_land = 0; src_land < LANDS_COUNT; src_land++) {
    uint* total_bombers = &land_units_state[src_land][BOMBERS_LAND_AIR][BOMBER_MOVES_MAX];
    if (*total_bombers == 0) {
      continue;
    }
    units_to_process = true;
    valid_moves[0] = src_land;
    valid_moves_count = 1;
    add_valid_bomber_moves(src_land, BOMBER_MOVES_MAX);
    while (*total_bombers > 0) {
      units_to_process = true;
      uint dst_air = valid_moves[0];
      if (valid_moves_count == 1) {
        if (answers_remaining == 0)
          return true;
        dst_air = get_user_move_input(BOMBERS_LAND_AIR, src_land);
      }
#ifdef DEBUG
      if (actually_print) {
        printf("DEBUG: player: %s bombers fighters %d, src_air: %d, dst_air: %d\n",
               PLAYERS[state.player_index].name, BOMBERS_LAND_AIR, src_land, dst_air);
      }
#endif
      // update_move_history(dst_air, src_land);
      if (src_land == dst_air) {
        land_units_state[src_land][BOMBERS_LAND_AIR][0] += *total_bombers;
        *total_bombers = 0;
        continue;
      }
      if (dst_air < LANDS_COUNT) {
        if (!is_allied_0[*owner_idx[dst_air]]) {
#ifdef DEBUG
          if (actually_print) {
            printf("Bomber moving to enemy territory. Automatically flagging for combat\n");
          }
#endif
          state.flagged_for_combat[dst_air] = 2;
          // assuming enemy units are present based on valid moves
        }
      } else {
#ifdef DEBUG
        if (actually_print) {
          printf("Bomber moving to sea. Possibly flagging for combat\n");
        }
#endif
        uint dst_sea = dst_air - LANDS_COUNT;
        state.flagged_for_combat[dst_air] = enemy_units_count[dst_air] > 0;
      }
      uint airDistance = AIR_DIST[src_land][dst_air];
      if (dst_air < LANDS_COUNT) {
        land_units_state[dst_air][BOMBERS_LAND_AIR][BOMBER_MOVES_MAX - airDistance]++;
        total_player_land_unit_types[0][dst_air][BOMBERS_LAND_AIR]++;
        total_player_land_units[0][dst_air]++;
      } else {
        uint dst_sea = dst_air - LANDS_COUNT;
        sea_units_state[dst_sea][BOMBERS_SEA][BOMBER_MOVES_MAX - 1 - airDistance]++;
        current_player_sea_unit_types[dst_sea][BOMBERS_SEA]++;
        total_player_sea_units[0][dst_sea]++;
      }
      current_player_land_unit_types[src_land][BOMBERS_LAND_AIR]--;
      total_player_land_units[0][src_land]--;
      *total_bombers -= 1;
    }
  }
  if (units_to_process) {
    clear_move_history();
  }
  return false;
}

void conquer_land(uint dst_land) {
#ifdef DEBUG
  if (actually_print) {
    setPrintableStatus();
    printf("%s\n", printableGameStatus);
    printf("conquer_land: dst_land=%d\n", dst_land);
  }
#endif
  uint old_owner_id = *owner_idx[dst_land];
  if (PLAYERS[(state.player_index + old_owner_id) % PLAYERS_COUNT].capital_territory_index ==
      dst_land) {
    state.money[0] += state.money[old_owner_id];
    state.money[old_owner_id] = 0;
  }
  income_per_turn[old_owner_id] -= LAND_VALUE[dst_land];
  uint new_owner_id = 0;
  uint orig_owner_id =
      (LANDS[dst_land].original_owner_index + PLAYERS_COUNT - state.player_index) % PLAYERS_COUNT;
  if (is_allied_0[orig_owner_id]) {
    new_owner_id = orig_owner_id;
  }
#ifdef DEBUG
  if (actually_print) {
    printf("conquer_land: old_owner_id=%d new_owner_id=%d orig_owner_id=%d\n", old_owner_id,
           new_owner_id, orig_owner_id);
  }
#endif
  *owner_idx[dst_land] = new_owner_id;
  income_per_turn[new_owner_id] += LAND_VALUE[dst_land];
  factory_locations[new_owner_id][total_factory_count[new_owner_id]++] = dst_land;
  total_factory_count[old_owner_id]--;
  for (uint i = 0; i < total_factory_count[old_owner_id]; i++) {
    if (factory_locations[old_owner_id][i] == dst_land) {
#ifdef DEBUG
      if (actually_print) {
        printf("DEBUG: Found factory at %s\n", LANDS[dst_land].name);
      }
#endif
      for (uint j = i; j < total_factory_count[old_owner_id]; j++) {
#ifdef DEBUG
        if (actually_print) {
          printf("DEBUG: Moving factory at %s\n", LANDS[factory_locations[old_owner_id][j]].name);
        }
#endif
        factory_locations[old_owner_id][j] = factory_locations[old_owner_id][j + 1];
      }
      break;
    }
  }
#ifdef DEBUG
  if (actually_print) {
    setPrintableStatus();
    printf("%s\n", printableGameStatus);
    for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
      for (uint factory_index = 0; factory_index < total_factory_count[player_idx];
           factory_index++) {
        uint factory_location = factory_locations[player_idx][factory_index];
        if (*owner_idx[factory_location] != player_idx) {
          printf("DEBUG: Player %s has a unowned factory at %s\n",
                 PLAYERS[(state.player_index + player_idx) % PLAYERS_COUNT].name,
                 LANDS[factory_location].name);
        }
      }
    }
  }
#endif
}

bool move_land_unit_type(uint unit_type) {
  bool units_to_process = false;
  for (uint src_land = 0; src_land < LANDS_COUNT; src_land++) {
    for (uint moves_remaining = MAX_MOVE_LAND[unit_type]; moves_remaining > 0; moves_remaining--) {
      uint* total_units = &land_units_state[src_land][unit_type][moves_remaining];
      if (*total_units == 0) {
        continue;
      }
      units_to_process = true;
      valid_moves[0] = src_land;
      valid_moves_count = 1;
      add_valid_land_moves(src_land, moves_remaining, unit_type);
      while (*total_units > 0) {
        units_to_process = true;
        uint dst_air = valid_moves[0];
        if (valid_moves_count > 1) {
          if (answers_remaining == 0)
            return true;
          dst_air = get_user_move_input(unit_type, src_land);
        }
#ifdef DEBUG
        if (actually_print) {
          setPrintableStatus();
          printf("%s\n", printableGameStatus);
          printf(
              "DEBUG: player: %s moving land unit %d, src_air: %d, dst_air: %d enemy count: %d\n",
              PLAYERS[state.player_index].name, unit_type, src_land, dst_air,
              enemy_units_count[dst_air]);
        }
#endif
        // update_move_history(dst_air, src_land);
        if (src_land == dst_air) {
          land_units_state[src_land][unit_type][0] += *total_units;
          *total_units = 0;
          continue;
        }
        if (dst_air >= LANDS_COUNT) {
          load_transport(unit_type, src_land, dst_air - LANDS_COUNT, moves_remaining);
          // recalculate valid moves since transport cargo has changed
          valid_moves_count = 1;
          add_valid_land_moves(src_land, moves_remaining, unit_type);
          continue;
        }
        state.flagged_for_combat[dst_air] = enemy_units_count[dst_air] > 0;
        // if the destination is not blitzable, then end unit turn
        uint landDistance = LAND_DIST[src_land][dst_air];
        if (is_allied_0[*owner_idx[dst_air]] || enemy_units_count[dst_air] > 0) {
          landDistance = moves_remaining;
        }
        land_units_state[dst_air][unit_type][moves_remaining - landDistance]++;
        current_player_land_unit_types[dst_air][unit_type]++;
        total_player_land_units[0][dst_air]++;
        current_player_land_unit_types[src_land][unit_type]--;
        total_player_land_units[0][src_land]--;
        *total_units -= 1;
        if (!is_allied_0[*owner_idx[dst_air]] && enemy_units_count[dst_air] == 0) {
#ifdef DEBUG
          if (actually_print) {
            printf("Conquering land");
          }
#endif
          conquer_land(dst_air);
          state.flagged_for_combat[dst_air] = 2;
        }
#ifdef DEBUG
        debug_checks();
#endif
      }
    }
  }
  if (units_to_process) {
    clear_move_history();
  }
  return false;
}

void skip_empty_transports() {
  for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
#pragma unroll
    for (uint cur_state = TRANS_EMPTY_STATES - TRANS_EMPTY_STAGING_STATES;
         cur_state >= TRANS_EMPTY_UNLOADING_STATES + 1; cur_state--) {
      uint* total_ships = &sea_units_state[src_sea][TRANS_EMPTY][cur_state];
      if (*total_ships == 0) {
        continue;
      }
      sea_units_state[src_sea][TRANS_EMPTY][0] += *total_ships;
      *total_ships = 0;
    }
  }
}
typedef uint unit_states[5];
typedef unit_states* unit_states_ptr;

bool move_transport_units() {
  bool units_to_process = false;
  skip_empty_transports();
#ifdef DEBUG
  if (actually_print) {
    printf("DEBUG: move_transport_units\n");
  }
#endif
  for (uint unit_type = TRANS_1I; unit_type <= TRANS_1I_1T;
       unit_type++) { // there should be no TRANS_EMPTY
    uint max_state = STATES_MOVE_SEA[unit_type] - STATES_STAGING[unit_type];
    // uint done_moving = 1;//STATES_UNLOADING[unit_type];
    // uint min_state = 2;//STATES_UNLOADING[unit_type] + 1;
    // clear_move_history();
    for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      // for (uint cur_state = max_state; cur_state >= min_state; cur_state--) {
      for (uint i = 1; i < 3; i++) {
        uint cur_state = max_state - i;
        uint* total_ships = &sea_units_state[src_sea][unit_type][cur_state];
        // unit_states_ptr unit_states = total_ships;
        if (*total_ships == 0) {
          continue;
        }
        units_to_process = true;
        uint moves_remaining = cur_state - 1; // STATES_UNLOADING[unit_type];
        uint src_air = src_sea + LANDS_COUNT;
        valid_moves[0] = src_air;
        valid_moves_count = 1;
        add_valid_sea_moves(src_sea, moves_remaining);
        while (*total_ships > 0) {
          units_to_process = true;
          uint dst_air = valid_moves[0];
          if (valid_moves_count > 1) {
            if (answers_remaining == 0)
              return true;
            dst_air = get_user_move_input(unit_type, src_air);
          }
#ifdef DEBUG
          if (actually_print) {
            printf("DEBUG: moving transport units unit_type: %d, src_air: %d, dst_air: %d\n",
                   unit_type, src_air, dst_air);
          }
#endif
          // update_move_history(dst_air, src_air);
          uint dst_sea = dst_air - LANDS_COUNT;
          if (enemy_blockade_total[dst_sea] > 0) {
#ifdef DEBUG
            if (actually_print) {
              printf("Enemy units detected, flagging for combat\n");
            }
#endif
            state.flagged_for_combat[dst_air] = 2;
          }
          if (src_air == dst_air) {
            sea_units_state[src_sea][unit_type][1] += *total_ships;
            *total_ships = 0;
            continue;
          }
          sea_units_state[dst_sea][unit_type][1]++;
          current_player_sea_unit_types[dst_sea][unit_type]++;
          total_player_sea_units[0][dst_sea]++;
          (*total_ships)--;
          current_player_sea_unit_types[src_sea][unit_type]--;
          total_player_sea_units[0][src_sea]--;
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
  if (units_to_process) {
    clear_move_history();
  }
  return false;
}
bool move_subs() {
  bool units_to_process = false;
  for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    uint* total_subs = &sea_units_state[src_sea][SUBMARINES][SUB_UNMOVED];
    if (*total_subs == 0)
      continue;
    uint src_air = src_sea + LANDS_COUNT;
    valid_moves[0] = src_air;
    valid_moves_count = 1;
    add_valid_sub_moves(src_sea, SUB_MOVES_MAX);
    while (*total_subs > 0) {
      units_to_process = true;
      uint dst_air = valid_moves[0];
      if (valid_moves_count > 1) {
        if (answers_remaining == 0)
          return true;
        dst_air = get_user_move_input(SUBMARINES, src_air);
      }
#ifdef DEBUG
      if (actually_print) {
        setPrintableStatus();
        printf("%s\n", printableGameStatus);
        printf("DEBUG: moving sub units unit_type: %d, src_air: %d, dst_air: %d\n", SUBMARINES,
               src_air, dst_air);
      }
#endif
      // update_move_history(dst_air, src_air);
      uint dst_sea = dst_air - LANDS_COUNT;
      if (enemy_units_count[dst_sea] > 0) {
#ifdef DEBUG
        if (actually_print) {
          printf("Submarine moving to where enemy units are present, flagging for combat\n");
        }
#endif
        state.flagged_for_combat[dst_sea] = 2;
        // break;
      }
      if (src_air == dst_air) {
        sea_units_state[src_sea][SUBMARINES][SUB_DONE_MOVING] += *total_subs;
        *total_subs = 0;
        continue;
      }
      sea_units_state[dst_sea][SUBMARINES][SUB_DONE_MOVING]++;
      current_player_sea_unit_types[dst_sea][SUBMARINES]++;
      total_player_sea_units[0][dst_sea]++;
      *total_subs -= 1;
      current_player_sea_unit_types[src_sea][SUBMARINES]--;
      total_player_sea_units[0][src_sea]--;
    }
  }
  if (units_to_process) {
    clear_move_history();
  }
  return false;
}

bool move_destroyers_battleships() {
  bool units_to_process = false;
  for (uint unit_type = DESTROYERS; unit_type <= BS_DAMAGED; unit_type++) {
    uint unmoved = UNMOVED_SEA[unit_type];
    uint done_moving = DONE_MOVING_SEA[unit_type];
    uint moves_remaining = MAX_MOVE_SEA[unit_type];
    // TODO CHECKPOINT
    // clear_move_history();
    for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      uint* total_ships = &sea_units_state[src_sea][unit_type][unmoved];
      if (*total_ships == 0) {
        continue;
      }
      units_to_process = true;
      uint src_air = src_sea + LANDS_COUNT;
      valid_moves[0] = src_air;
      valid_moves_count = 1;
      add_valid_sea_moves(src_sea, moves_remaining);
      while (*total_ships > 0) {
        units_to_process = true;
        uint dst_air = valid_moves[0];
        if (valid_moves_count > 1) {
          if (answers_remaining == 0)
            return true;
          dst_air = get_user_move_input(unit_type, src_air);
        }
#ifdef DEBUG
        if (actually_print) {
          setPrintableStatus();
          printf("%s\n", printableGameStatus);
          printf("DEBUG: moving ships units unit_type: %d, src_air: %d, dst_air: %d\n", unit_type,
                 src_air, dst_air);
        }
#endif
        // update_move_history(dst_air, src_air);
        if (enemy_units_count[dst_air] > 0) {
#ifdef DEBUG
          if (actually_print) {
            printf("Moving large ships. Enemy units detected, flagging for combat\n");
          }
#endif
          state.flagged_for_combat[dst_air] = 2;
          // break;
        }
        if (src_air == dst_air) {
          sea_units_state[src_sea][unit_type][done_moving] += *total_ships;
          *total_ships = 0;
          continue;
        }
        uint dst_sea = dst_air - LANDS_COUNT;
        sea_units_state[dst_sea][unit_type][done_moving]++;
        current_player_sea_unit_types[dst_sea][unit_type]++;
        total_player_sea_units[0][dst_sea]++;
        *total_ships -= 1;
        current_player_sea_unit_types[src_sea][unit_type]--;
        total_player_sea_units[0][src_sea]--;
        if (unit_type == CARRIERS)
          carry_allied_fighters(src_sea, dst_sea);
#ifdef DEBUG
        debug_checks();
#endif
      }
    }
  }
  if (units_to_process) {
    clear_move_history();
  }
  return false;
}

void carry_allied_fighters(uint src_sea, uint dst_sea) {
#ifdef DEBUG
  if (actually_print) {
    printf("DEBUG: carry_allied_fighters: src_sea: %d, dst_sea: %d\n", src_sea, dst_sea);
    setPrintableStatus();
    printf("%s\n", printableGameStatus);
  }
  debug_checks();
#endif
  uint allied_fighters_moved = 0;
  for (uint other_player_idx = 1; other_player_idx < PLAYERS_COUNT; other_player_idx++) {
    if (!is_allied_0[other_player_idx]) {
      continue;
    }
    while (total_player_sea_unit_types[other_player_idx][src_sea][FIGHTERS] > 0) {
      total_player_sea_unit_types[other_player_idx][src_sea][FIGHTERS]--;
      total_player_sea_units[other_player_idx][src_sea]--;
      total_player_sea_unit_types[other_player_idx][dst_sea][FIGHTERS]++;
      total_player_sea_units[other_player_idx][dst_sea]++;
      if (allied_fighters_moved == 1)
        return;
      allied_fighters_moved++;
    }
  }
}

bool resolve_sea_battles() {
  for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    // check if battle is over (e.g. untargetable subs/air/trans or zero units)
    // target options:
    // 1. attacker has air, defender has std ships or air
    // 2. attacker has any ships, defender has any non-transports
    // 3. defender has air, attacker has std ships or air
    // 4. defender has any ships, attacker has any non-transports
    uint src_air = src_sea + LANDS_COUNT;
    // if not flagged for combat, continue
    if (state.flagged_for_combat[src_air] == 0) {
      continue;
    }
    if (total_player_sea_units[0][src_sea] == 0) {
      continue;
    }
#ifdef DEBUG
    if (actually_print) {
      setPrintableStatus();
      printf("%s\n", printableGameStatus);
      printf("DEBUG: resolve_sea_battles: src_sea: %d\n", src_sea);
    }
#endif
    bool defender_submerged = current_player_sea_unit_types[src_sea][DESTROYERS] == 0;
    if (total_player_sea_units[0][src_sea] == 2) {
      // does enemy only have submerged subs?
      if (defender_submerged) {
        uint total_enemy_subs = 0;
        for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
          total_enemy_subs +=
              total_player_sea_unit_types[enemies_0[enemy_idx]][src_sea][SUBMARINES];
        }
        if (total_enemy_subs == enemy_units_count[src_sea]) {
          continue;
        }
      }
      // combat is proceeding, so disable bombardment capabilities of ships
      for (uint unit_type = CRUISERS; unit_type <= BS_DAMAGED; unit_type++) {
        sea_units_state[src_sea][unit_type][0] += sea_units_state[src_sea][unit_type][1];
        sea_units_state[src_sea][unit_type][1] = 0;
      }
    }
    while (true) {
#ifdef DEBUG
      if (actually_print) {
        setPrintableStatus();
        printf("%s\n", printableGameStatus);
        if (current_player_land_unit_types[2][FIGHTERS] == 255) {
          printf("DEBUG: units_land_ptr[0][FIGHTERS][0] == 255\n");
        }
        if (enemy_units_count[2] == -1) {
          printf("DEBUG: enemy_units_count[2] == -1\n");
        }
      }
#endif
      // check for retreat option
      if (state.flagged_for_combat[src_air] == 1) {
        if (current_player_sea_unit_types[src_sea][FIGHTERS] +
                    current_player_sea_unit_types[src_sea][BOMBERS_SEA] +
                    current_player_sea_unit_types[src_sea][SUBMARINES] +
                    current_player_sea_unit_types[src_sea][DESTROYERS] +
                    current_player_sea_unit_types[src_sea][CARRIERS] +
                    current_player_sea_unit_types[src_sea][CRUISERS] +
                    current_player_sea_unit_types[src_sea][BATTLESHIPS] +
                    current_player_sea_unit_types[src_sea][BS_DAMAGED] >
                0 ||
            enemy_blockade_total[src_sea] == 0) {
          valid_moves[0] = src_air;
          valid_moves_count = 1;
        } else {
          valid_moves_count = 0;
        }
        S2SConn sea_to_sea_conn = SEA_TO_SEA_CONN[src_sea];
        for (uint sea_conn_idx = 0; sea_conn_idx < SEA_TO_SEA_COUNT[src_sea]; sea_conn_idx++) {
          uint sea_dst = sea_to_sea_conn[sea_conn_idx];
          if (enemy_blockade_total[sea_dst] == 0)
            valid_moves[valid_moves_count++] = sea_dst + LANDS_COUNT;
        }
        if (valid_moves_count > 0) {
          uint dst_air;
          if (valid_moves_count == 1) {
            dst_air = valid_moves[0];
          } else {
            if (answers_remaining == 0)
              return true;
            dst_air = ask_to_retreat();
          }
          // if retreat, move units to retreat zone immediately and end battle
          uint dst_sea = dst_air - LANDS_COUNT;
          if (sea_dist[src_sea][dst_sea] == 1 && state.flagged_for_combat[dst_air] == 0) {
            sea_retreat(src_sea, dst_sea);
            break;
          }
        }
      }
      state.flagged_for_combat[src_air] = 1;
      // uint* units11 = &sea_units_state[src_sea][DESTROYERS][0];
      bool targets_exist = false;
      if (current_player_sea_unit_types[src_sea][DESTROYERS]) {
        if (enemy_units_count[src_air] > 0) {
          targets_exist = true;
        }
      } else if (current_player_sea_unit_types[src_sea][CARRIERS] +
                     current_player_sea_unit_types[src_sea][CRUISERS] +
                     current_player_sea_unit_types[src_sea][BATTLESHIPS] +
                     current_player_sea_unit_types[src_sea][BS_DAMAGED] >
                 0) {
        if (enemy_units_count[src_air] > 0) {
          for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
            uint enemy_player_idx = enemies_0[enemy_idx];
            if (total_player_sea_units[enemy_player_idx][src_sea] -
                    total_player_sea_unit_types[enemy_player_idx][src_sea][SUBMARINES] >
                0) {
              targets_exist = true;
              break;
            }
          }
        }
      } else if (current_player_sea_unit_types[src_sea][SUBMARINES] >
                 0) { // no blockade ships, only subs
        for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
          uint enemy_player_idx = enemies_0[enemy_idx];
          if (total_player_sea_units[enemy_player_idx][src_sea] -
                  (total_player_sea_unit_types[enemy_player_idx][src_sea][FIGHTERS] +
                   total_player_sea_unit_types[enemy_player_idx][src_sea][SUBMARINES]) >
              0) {
            targets_exist = true;
            break;
          }
        }
      } else if (current_player_sea_unit_types[src_sea][FIGHTERS] +
                     current_player_sea_unit_types[src_sea][BOMBERS_SEA] >
                 0) { // no ships, only air
        for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
          uint enemy_player_idx = enemies_0[enemy_idx];
          if (total_player_sea_units[enemy_player_idx][src_sea] -
                  total_player_sea_unit_types[enemy_player_idx][src_sea][SUBMARINES] >
              0) {
            targets_exist = true;
            break;
          }
        }
      }
#ifdef DEBUG
      debug_checks();
#endif
      // TODO only allow hits to hit valid targets
      // untargetable battle
      if (!targets_exist) {
        if (enemy_units_count[src_air] > 0) {
          total_player_sea_units[0][src_sea] -=
              total_player_sea_unit_types[0][src_sea][TRANS_EMPTY];
          total_player_sea_unit_types[0][src_sea][TRANS_EMPTY] = 0;
          sea_units_state[src_sea][TRANS_EMPTY][0] = 0;
          for (uint trans_unit_type = TRANS_1I; trans_unit_type <= TRANS_1I_1T; trans_unit_type++) {
            total_player_sea_units[0][src_sea] -=
                total_player_sea_unit_types[0][src_sea][trans_unit_type];
            total_player_sea_unit_types[0][src_sea][trans_unit_type] = 0;
            sea_units_state[src_sea][trans_unit_type][1] = 0;
          }
        }
        // allied_carriers[src_sea] =
        //     data.units_sea[src_sea].carriers[0] + data.units_sea[src_sea].carriers[1];
        // for (uint player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
        //   allied_carriers[src_sea] +=
        //       other_sea_units_ptr[player_idx][src_sea][CARRIERS] * is_allied_0[player_idx];
        // }
        state.flagged_for_combat[src_air] = 0;
        break;
      }
      // fire subs (defender always submerges if possible)
      uint attacker_damage = sea_units_state[src_sea][SUBMARINES][0] * SUB_ATTACK; // TODO FIX
      // uint attacker_damage = current_player_sea_unit_types[src_sea][SUBMARINES] * SUB_ATTACK;
      // //TODO FIX
      uint attacker_hits = get_attacker_hits(attacker_damage);
      uint defender_damage = 0;
      uint defender_hits = 0;
      if (!defender_submerged) {
        defender_damage = 0; // TODO skip if no subs
        for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
          defender_damage += total_player_sea_unit_types[enemies_0[enemy_idx]][src_sea][SUBMARINES];
        }
        defender_hits = get_defender_hits(defender_damage);
#ifdef DEBUG
        debug_checks();
#endif
        if (defender_hits > 0) {
          remove_sea_attackers(src_sea, defender_hits);
        }
#ifdef DEBUG
        debug_checks();
#endif
      }
// remove casualties
#ifdef DEBUG
      debug_checks();
#endif
      if (attacker_hits > 0)
        remove_sea_defenders(src_sea, attacker_hits, defender_submerged);
      // fire all ships and air for both sides
      attacker_damage = 0;
      for (uint unit_type_idx = 0; unit_type_idx < BLOCKADE_UNIT_TYPES_COUNT; unit_type_idx++) {
        uint unit_type = BLOCKADE_UNIT_TYPES[unit_type_idx];
        attacker_damage +=
            current_player_sea_unit_types[src_sea][unit_type] * ATTACK_UNIT_SEA[unit_type];
      }
      attacker_damage +=
          current_player_sea_unit_types[src_sea][FIGHTERS] * ATTACK_UNIT_SEA[FIGHTERS];
      attacker_damage +=
          current_player_sea_unit_types[src_sea][BOMBERS_SEA] * ATTACK_UNIT_SEA[BOMBERS_SEA];
      attacker_hits = get_attacker_hits(attacker_damage);
      defender_damage = 0;
      for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
        auto enemy_units = total_player_sea_unit_types[enemies_0[enemy_idx]][src_sea];
        for (uint unit_type = 0; unit_type < BLOCKADE_UNIT_TYPES_COUNT; unit_type++) {
          defender_damage += enemy_units[unit_type] * DEFENSE_UNIT_SEA[unit_type];
        }
        defender_damage += enemy_units[FIGHTERS] * DEFENSE_UNIT_SEA[FIGHTERS];
      }
      defender_hits = get_defender_hits(defender_damage);
// remove casualties
#ifdef DEBUG
      debug_checks();
#endif
      if (defender_hits > 0) {
        remove_sea_attackers(src_sea, defender_hits);
      }
      if (attacker_hits > 0) {
        remove_sea_defenders(src_sea, attacker_hits, defender_submerged);
      }
      if (enemy_units_count[src_air] == 0 || total_player_sea_units[0][src_sea] == 0) {
        state.flagged_for_combat[src_air] = 0;
        break;
      }
    }
  }
  return false;
}

uint get_attacker_hits(uint attacker_damage) {
  uint attacker_hits = 0;
  if (answers_remaining < 2) {
    if (PLAYERS[state.player_index].is_allied[unlucky_player_idx]) {
      attacker_hits = (attacker_damage / 6);
    } else {
      attacker_hits = (attacker_damage / 6) + (1 < attacker_damage % 6 ? 1 : 0);
    }
  } else {
    attacker_hits = (attacker_damage / 6) +
                    (RANDOM_NUMBERS[random_number_index++] % 6 < attacker_damage % 6 ? 1 : 0);
  }
  return attacker_hits;
}

uint get_defender_hits(uint defender_damage) {
  uint defender_hits = 0;
  if (answers_remaining < 2) {
    if (PLAYERS[state.player_index].is_allied[unlucky_player_idx]) {
      defender_hits = (defender_damage / 6) + (1 < defender_damage % 6 ? 1 : 0);
    } else {
      defender_hits = (defender_damage / 6);
    }
  } else {
    defender_hits = (defender_damage / 6) +
                    (RANDOM_NUMBERS[random_number_index++] % 6 < defender_damage % 6 ? 1 : 0);
  }
  return defender_hits;
}

void sea_retreat(uint src_sea, uint dst_sea) {
#ifdef DEBUG
  if (actually_print) {
    debug_checks();
    printf("DEBUG: retreating to sea: %d\n", dst_sea);
  }
#endif
  for (uint unit_type = TRANS_EMPTY; unit_type <= BS_DAMAGED; unit_type++) {
    uint unit_state_count = current_player_sea_unit_types[src_sea][unit_type];
    sea_units_state[dst_sea][unit_type][0] += unit_state_count;
    current_player_sea_unit_types[dst_sea][unit_type] += unit_state_count;
    total_player_sea_units[0][dst_sea] += unit_state_count;
    total_player_sea_units[0][src_sea] -= unit_state_count;
    sea_units_state[src_sea][unit_type][0] = 0;
    sea_units_state[src_sea][unit_type][1] = 0;
    current_player_sea_unit_types[src_sea][unit_type] = 0;
  }
  state.flagged_for_combat[src_sea + LANDS_COUNT] = 0;
#ifdef DEBUG
  debug_checks();
#endif
}

uint ask_to_retreat() {
  if (PLAYERS[state.player_index].is_human) {
    setPrintableStatus();
    strcat(printableGameStatus, "To where do you want to retreat (255 for no)? ");
    printf("%s\n", printableGameStatus);
    return getUserInput();
  }
  return getAIInput();
}
void remove_land_defenders(uint src_land, uint hits) {
  for (uint unit_idx = 0; unit_idx < DEFENDER_LAND_UNIT_TYPES_COUNT; unit_idx++) {
    for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
      uint enemy_player_idx = enemies_0[enemy_idx];
      uint* total_units = &total_player_land_unit_types[enemy_player_idx][src_land]
                                                       [ORDER_OF_LAND_DEFENDERS[unit_idx]];
      if (*total_units > 0) {
#ifdef DEBUG
        if (actually_print) {
          printf(
              "DEBUG: remove_land_defenders: src_land: %d, hits: %d, unit_idx: %d, enemy_idx: %d\n",
              src_land, hits, ORDER_OF_LAND_DEFENDERS[unit_idx], enemy_idx);
        }
#endif
        if (*total_units < hits) {
          hits -= *total_units;
          total_player_land_units[enemy_player_idx][src_land] -= *total_units;
          enemy_units_count[src_land] -= *total_units;
          *total_units = 0;
        } else {
          *total_units -= hits;
          total_player_land_units[enemy_player_idx][src_land] -= hits;
          enemy_units_count[src_land] -= hits;
          return;
        }
      }
    }
  }
}
void remove_land_attackers(uint src_land, uint hits) {
  uint* total_units;
  for (uint unit_idx = 0; unit_idx < ATTACKER_LAND_UNIT_TYPES_COUNT_1; unit_idx++) {
    uint unit_type = ORDER_OF_LAND_ATTACKERS_1[unit_idx];
    // TODO fix - why are AA guns with 1 move remaining here?
    total_units = &land_units_state[src_land][unit_type][0];
    if (*total_units > 0) {
#ifdef DEBUG
      if (actually_print) {
        printf("DEBUG: remove_land_attackers: unit_type: %d src_land: %d, hits: %d\n", unit_type,
               src_land, hits);
      }
#endif
      if (*total_units < hits) {
        hits -= *total_units;
        total_player_land_units[0][src_land] -= *total_units;
        *total_units = 0;
        total_player_land_unit_types[0][src_land][unit_type] = 0;
      } else {
        *total_units -= hits;
        total_player_land_unit_types[0][src_land][unit_type] -= hits;
        total_player_land_units[0][src_land] -= hits;
        hits = 0;
        return;
      }
    }
  }
  for (uint unit_idx = 0; unit_idx < ATTACKER_LAND_UNIT_TYPES_COUNT_2; unit_idx++) {
    uint unit_type = ORDER_OF_LAND_ATTACKERS_2[unit_idx];
    if (total_player_land_unit_types[0][src_land][unit_type] == 0)
      continue;
    for (uint cur_state = 1; cur_state < STATES_MOVE_LAND[unit_type] - 1; cur_state++) {
      total_units = &land_units_state[src_land][unit_type][cur_state];
      if (*total_units > 0) {
#ifdef DEBUG
        if (actually_print) {
          printf("DEBUG: remove_land_attackers: unit_type: %d src_land: %d, hits: %d\n", unit_type,
                 src_land, hits);
        }
#endif

        if (*total_units < hits) {
          hits -= *total_units;
          total_player_land_unit_types[0][src_land][unit_type] -= *total_units;
          total_player_land_units[0][src_land] -= *total_units;
          *total_units = 0;
        } else {
          *total_units -= hits;
          total_player_land_unit_types[0][src_land][unit_type] -= hits;
          total_player_land_units[0][src_land] -= hits;
          hits = 0;
          return;
        }
      }
    }
  }
}
void remove_sea_defenders(uint src_sea, uint hits, bool defender_submerged) {
#ifdef DEBUG
  if (actually_print) {
    printf("DEBUG: remove_sea_defenders: src_sea: %d, hits: %d\n", src_sea, hits);
  }
  debug_checks();
#endif
  uint src_air = src_sea + LANDS_COUNT;
  for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
    uint enemy_player_idx = enemies_0[enemy_idx];
    uint* total_battleships = &total_player_sea_unit_types[enemy_player_idx][src_sea][BATTLESHIPS];
    uint* total_bs_damaged = &total_player_sea_unit_types[enemy_player_idx][src_sea][BS_DAMAGED];
    if (*total_battleships > 0) {
#ifdef DEBUG
      if (actually_print) {
        printf("DEBUG: remove_sea_defenders: src_sea: %d, hits: %d, total_battleships: %d\n",
               src_sea, hits, *total_battleships);
      }
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
    for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
      uint enemy_player_idx = enemies_0[enemy_idx];
      uint* total_units = &total_player_sea_unit_types[enemy_player_idx][src_sea][SUBMARINES];
      if (*total_units > 0) {
#ifdef DEBUG
        if (actually_print) {
          printf("DEBUG: remove_sea_defenders subs: src_sea: %d, hits: %d, total_units: %d\n",
                 src_sea, hits, *total_units);
        }
#endif
        if (*total_units < hits) {
          hits -= *total_units;
          total_player_sea_units[enemy_player_idx][src_sea] -= *total_units;
          enemy_units_count[src_air] -= *total_units;
          *total_units = 0;
        } else {
          *total_units -= hits;
          total_player_sea_units[enemy_player_idx][src_sea] -= hits;
          enemy_units_count[src_air] -= hits;
          hits = 0;
          return;
        }
      }
    }
  }

  // skipping submarines
  for (uint unit_idx = 1; unit_idx < DEFENDER_SEA_UNIT_TYPES_COUNT; unit_idx++) {
    for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
      uint enemy_player_idx = enemies_0[enemy_idx];
      uint* total_units =
          &total_player_sea_unit_types[enemy_player_idx][src_sea][ORDER_OF_SEA_DEFENDERS[unit_idx]];
      if (*total_units > 0) {
#ifdef DEBUG
        if (actually_print) {
          printf(
              "DEBUG: remove_sea_defenders: unit_type: %d src_sea: %d, hits: %d, total_units: %d\n",
              ORDER_OF_SEA_DEFENDERS[unit_idx], src_sea, hits, *total_units);
        }
#endif
        if (*total_units < hits) {
          hits -= *total_units;
          total_player_sea_units[enemy_player_idx][src_sea] -= *total_units;
          enemy_units_count[src_air] -= *total_units;
          if (unit_idx >= DESTROYERS && unit_idx <= BS_DAMAGED) {
            enemy_blockade_total[src_sea] -= *total_units;
          }
          *total_units = 0;
        } else {
          *total_units -= hits;
          total_player_sea_units[enemy_player_idx][src_sea] -= hits;
          enemy_units_count[src_air] -= hits;
          if (unit_idx >= DESTROYERS && unit_idx <= BS_DAMAGED) {
            enemy_blockade_total[src_sea] -= hits;
          }
          hits = 0;
          return;
        }
      }
    }
  }
#ifdef DEBUG
  if (actually_print) {
    setPrintableStatus();
    printf("%s\n", printableGameStatus);
  }
#endif
}

void remove_sea_attackers(uint src_sea, uint hits) {
#ifdef DEBUG
  debug_checks();
#endif
#ifdef DEBUG
  if (actually_print) {
    printf("DEBUG: remove_sea_attackers: src_sea: %d, hits: %d\n", src_sea, hits);
  }
#endif
  uint* total_battleships = &sea_units_state[src_sea][BATTLESHIPS][0];
  uint* total_bs_damaged = &sea_units_state[src_sea][BS_DAMAGED][0];
  if (*total_battleships > 0) {
    if (*total_battleships < hits) {
#ifdef DEBUG
      if (actually_print) {
        printf("DEBUG: remove_sea_attackers: src_sea: %d, hits: %d, total_battleships: %d\n",
               src_sea, hits, *total_battleships);
      }
#endif
      hits -= *total_battleships;
      *total_bs_damaged += *total_battleships;
      current_player_sea_unit_types[src_sea][BS_DAMAGED] += *total_battleships;
      *total_battleships = 0;
      current_player_sea_unit_types[src_sea][BATTLESHIPS] = 0;
    } else {
      *total_bs_damaged += hits;
      current_player_sea_unit_types[src_sea][BS_DAMAGED] += hits;
      *total_battleships -= hits;
      current_player_sea_unit_types[src_sea][BATTLESHIPS] -= hits;
      hits = 0;
      return;
    }
  }
  uint* total_units;
  for (uint unit_idx = 0; unit_idx < ATTACKER_SEA_UNIT_TYPES_COUNT_1; unit_idx++) {
    uint unit_type = ORDER_OF_SEA_ATTACKERS_1[unit_idx];
    uint* total_units = &sea_units_state[src_sea][unit_type][0];
    if (*total_units > 0) {
#ifdef DEBUG
      if (actually_print) {
        printf("DEBUG: remove_sea_attackers: unit_type: %d src_sea: %d, hits: %d\n", unit_type,
               src_sea, hits);
      }
#endif
      if (*total_units < hits) {
        hits -= *total_units;
        total_player_sea_units[0][src_sea] -= *total_units;
        *total_units = 0;
        current_player_sea_unit_types[src_sea][unit_type] = 0;
      } else {
        *total_units -= hits;
        total_player_sea_units[0][src_sea] -= hits;
        current_player_sea_unit_types[src_sea][unit_type] -= hits;
        hits = 0;
        return;
      }
    }
  }
  total_units = &sea_units_state[src_sea][CARRIERS][0];
  if (*total_units > 0) {
    if (*total_units < hits) {
#ifdef DEBUG
      if (actually_print) {
        printf("DEBUG: remove_sea_attackers carriers: unit_type: %d src_sea: %d, hits: %d\n",
               CARRIERS, src_sea, hits);
      }
#endif
      hits -= *total_units;
      total_player_sea_units[0][src_sea] -= *total_units;
      *total_units = 0;
      current_player_sea_unit_types[src_sea][CARRIERS] = 0;
      allied_carriers[src_sea] = 0;
    } else {
      *total_units -= hits;
      current_player_sea_unit_types[src_sea][CARRIERS] -= hits;
      allied_carriers[src_sea] -= hits;
      total_player_sea_units[0][src_sea] -= hits;
      hits = 0;
      return; // TODO: do allied fighters have a chance to land?
    }
  }
  total_units = &sea_units_state[src_sea][CRUISERS][0];
  if (*total_units > 0) {
    if (*total_units < hits) {
#ifdef DEBUG
      if (actually_print) {
        printf("DEBUG: remove_sea_attackers cruisers: unit_type: %d src_sea: %d, hits: %d\n",
               CRUISERS, src_sea, hits);
      }
#endif
      hits -= *total_units;
      total_player_sea_units[0][src_sea] -= *total_units;
      *total_units = 0;
      current_player_sea_unit_types[src_sea][CRUISERS] = 0;
    } else {
      *total_units -= hits;
      current_player_sea_unit_types[src_sea][CRUISERS] -= hits;
      total_player_sea_units[0][src_sea] -= hits;
      hits = 0;
      return;
    }
  }
  // #ifdef DEBUG
  //         if (MCTS_ITERATIONS == 14500 && answers_remaining == 90659) {
  //           actually_print = true;
  //         }
  // #endif
  for (uint unit_idx = 0; unit_idx < ATTACKER_SEA_UNIT_TYPES_COUNT_2; unit_idx++) {
    uint unit_type = ORDER_OF_SEA_ATTACKERS_2[unit_idx];
    if (total_player_sea_unit_types[0][src_sea][unit_type] == 0)
      continue;
    for (uint cur_state = 0; cur_state < STATES_MOVE_SEA[unit_type]; cur_state++) {
      total_units = &sea_units_state[src_sea][unit_type][cur_state];
      if (*total_units > 0) {
#ifdef DEBUG
        if (actually_print) {
          printf("DEBUG: remove_sea_attackers: unit_type: %d src_sea: %d, hits: %d\n", unit_type,
                 src_sea, hits);
        }
#endif
        if (*total_units < hits) {
          hits -= *total_units;
          total_player_sea_units[0][src_sea] -= *total_units;
          current_player_sea_unit_types[src_sea][unit_type] -= *total_units;
          *total_units = 0;
        } else {
          *total_units -= hits;
          current_player_sea_unit_types[src_sea][unit_type] -= hits;
          total_player_sea_units[0][src_sea] -= hits;
          hits = 0;
          return;
        }
      }
    }
  }
  for (uint unit_idx = 0; unit_idx < ATTACKER_SEA_UNIT_TYPES_COUNT_3; unit_idx++) {
    uint unit_type = ORDER_OF_SEA_ATTACKERS_3[unit_idx];
    uint* total_units = &sea_units_state[src_sea][unit_type][0];
    if (*total_units > 0) {
#ifdef DEBUG
      if (actually_print) {
        printf("DEBUG: remove_sea_attackers: unit_type: %d src_sea: %d, hits: %d\n", unit_type,
               src_sea, hits);
      }
#endif
      if (*total_units < hits) {
        hits -= *total_units;
        total_player_sea_units[0][src_sea] -= *total_units;
        if (unit_type <= TRANS_1T) {
          transports_with_small_cargo_space[src_sea] -= *total_units;
          if (unit_type <= TRANS_1I) {
            transports_with_large_cargo_space[src_sea] -= *total_units;
          }
        }
        *total_units = 0;
        current_player_sea_unit_types[src_sea][unit_type] = 0;
      } else {
        *total_units -= hits;
        current_player_sea_unit_types[src_sea][unit_type] -= hits;
        total_player_sea_units[0][src_sea] -= hits;
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
#ifdef DEBUG
  debug_checks();
#endif
}

bool unload_transports() {
  bool units_to_process = false;
  for (uint unit_type = TRANS_1I; unit_type <= TRANS_1I_1T; unit_type++) {
    uint unloading_state = STATES_UNLOADING[unit_type];
    uint unload_cargo1 = UNLOAD_CARGO1[unit_type];
    uint unload_cargo2 = UNLOAD_CARGO2[unit_type];
    //    clear_move_history();
    for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      uint* total_units = &sea_units_state[src_sea][unit_type][unloading_state];
      if (*total_units == 0) {
        continue;
      }
      units_to_process = true;
      uint src_air = src_sea + LANDS_COUNT;
      valid_moves[0] = src_air;
      valid_moves_count = 1;
      add_valid_unload_moves(src_sea);
      while (*total_units > 0) {
        uint dst_air = valid_moves[0];
        if (valid_moves_count > 1) {
          if (answers_remaining == 0)
            return true;
          dst_air = get_user_move_input(unit_type, src_air);
        }
#ifdef DEBUG
        if (actually_print) {
          printf("DEBUG: unload_transports unit_type: %d, src_sea: %d, dst_air: %d, unload_cargo: "
                 "%d, %d\n",
                 unit_type, src_sea, dst_air, unload_cargo1, unload_cargo2);
          setPrintableStatus();
          printf("%s\n", printableGameStatus);
        }
#endif
        // update_move_history(dst_air, src_sea);
        if (src_air == dst_air) {
          sea_units_state[src_sea][unit_type][0] += *total_units;
          *total_units = 0;
          continue;
        }
        (*bombard_max[dst_air])++;
        land_units_state[dst_air][unload_cargo1][0]++;
        current_player_land_unit_types[dst_air][unload_cargo1]++;
        total_player_land_units[0][dst_air]++;
        sea_units_state[src_sea][TRANS_EMPTY][0]++;
        current_player_sea_unit_types[src_sea][TRANS_EMPTY]++;
        current_player_sea_unit_types[src_sea][unit_type]--;
        *total_units -= 1;
        if (unit_type > TRANS_1T) {
          (*bombard_max[dst_air])++;
          land_units_state[dst_air][unload_cargo2][0]++;
          current_player_land_unit_types[dst_air][unload_cargo2]++;
          total_player_land_units[0][dst_air]++;
        }
        if (!is_allied_0[*owner_idx[dst_air]]) {
          state.flagged_for_combat[dst_air] = 2;
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
  if (units_to_process) {
    clear_move_history();
  }
  return false;
}
#define MAX_COMBAT_ROUNDS 100
bool resolve_land_battles() {
  for (uint src_land = 0; src_land < LANDS_COUNT; src_land++) {
    // check if battle is over
    if (state.flagged_for_combat[src_land] == 0) {
      continue;
    }
#ifdef DEBUG
    // if (MCTS_ITERATIONS == 5201) {
    //   actually_print = true;
    //   cause_breakpoint();
    // }
    // debug print the current src_land and its name
    if (actually_print) {
      printf("Resolve land combat in: %d, Name: %s\n", src_land, LANDS[src_land].name);
      setPrintableStatus();
      printf("%s\n", printableGameStatus);
    }
#endif
    auto other_land_units_0_src_land = current_player_land_unit_types[src_land];
    auto units_land_player_total_0_src_land = &total_player_land_units[0][src_land];
    uint attacker_damage;
    uint attacker_hits;
    // check if no friendlies remain
    if (*units_land_player_total_0_src_land == 0) {
#ifdef DEBUG
      if (actually_print) {
        printf("No friendlies remain");
      }
#endif
      continue;
    }
    if (state.flagged_for_combat[src_land] == 2) {

      // only bombers exist
      auto other_land_units_ptr_0_src_land = total_player_land_unit_types[0][src_land];
      uint* bombers_count = &other_land_units_ptr_0_src_land[BOMBERS_LAND_AIR];
      if (*bombers_count > 0 && total_player_land_units[0][src_land] == *bombers_count) {
        if (*factory_hp[src_land] > -*factory_max[src_land]) {
#ifdef DEBUG
          if (actually_print) {
            printf("Strategic Bombing");
          }
#endif
          // fire_strat_aa_guns();
          uint defender_damage = *bombers_count;
          uint defender_hits = get_defender_hits(defender_damage);
          if (defender_hits > 0) {
            uint* units_land_ptr_src_land_bombers = land_units_state[src_land][BOMBERS_LAND_AIR];
            for (uint cur_state = 1; cur_state < BOMBER_LAND_STATES - 1; cur_state++) {
              uint* total_units = &units_land_ptr_src_land_bombers[cur_state];
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
          attacker_hits = get_attacker_hits(attacker_damage);
        }
        *factory_hp[src_land] =
            fmax(*factory_hp[src_land] - attacker_hits, -*factory_max[src_land]);
        continue;
      }

#ifdef DEBUG
      if (actually_print) {
        printf("Normal Land Combat\n");
      }
#endif

      // bombard_shores
      if (*bombard_max[src_land] > 0) {
        attacker_damage = 0;
#ifdef DEBUG
        if (actually_print) {
          printf("Sea Bombardment\n");
        }
#endif
        for (uint unit_type = BS_DAMAGED; unit_type >= CRUISERS; unit_type--) {
          for (uint sea_idx = 0; sea_idx < LAND_TO_SEA_COUNT[src_land]; sea_idx++) {
            uint src_sea = LAND_TO_SEA_CONN[src_land][sea_idx];
            uint* total_bombard_ships = sea_units_state[src_sea][unit_type];
            while (total_bombard_ships[1] > 0 && *bombard_max[src_land] > 0) {
              attacker_damage += ATTACK_UNIT_SEA[unit_type];
              total_bombard_ships[0]++;
              total_bombard_ships[1]--;
              (*bombard_max[src_land])--;
            }
          }
        }
        *bombard_max[src_land] = 0;
        attacker_hits = get_attacker_hits(attacker_damage);
        if (attacker_hits > 0) {
          remove_land_defenders(src_land, attacker_hits);
        }
      }
      // check if can fire tactical aa_guns
      uint total_air_units = other_land_units_ptr_0_src_land[FIGHTERS] +
                             other_land_units_ptr_0_src_land[BOMBERS_LAND_AIR];
      uint defender_damage;
      uint defender_hits;
      if (total_air_units > 0) {
        uint total_aa_guns = 0;
        for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
          total_aa_guns += total_player_land_unit_types[enemies_0[enemy_idx]][src_land][AA_GUNS];
        }
        if (total_aa_guns > 0) {
#ifdef DEBUG
          if (actually_print) {
            printf("Firing AA");
          }
#endif
          // fire_tact_aa_guns();
          defender_damage = total_air_units * 3;
          defender_hits = get_defender_hits(defender_damage);
          if (defender_hits > 0) {
            for (uint cur_state = 0; cur_state < FIGHTER_STATES; cur_state++) {
              uint* total_units = &land_units_state[src_land][FIGHTERS][cur_state];
              if (*total_units < defender_hits) {
                defender_hits -= *total_units;
                other_land_units_ptr_0_src_land[FIGHTERS] -= *total_units;
                *units_land_player_total_0_src_land -= *total_units;
                *total_units = 0;
              } else {
                *total_units -= defender_hits;
                other_land_units_ptr_0_src_land[FIGHTERS] -= defender_hits;
                *units_land_player_total_0_src_land -= defender_hits;
                defender_hits = 0;
                break;
              }
            }
          }
          if (defender_hits > 0) {
            for (uint cur_state = 0; cur_state < BOMBER_LAND_STATES; cur_state++) {
              uint* total_units = &land_units_state[src_land][BOMBERS_LAND_AIR][cur_state];
              if (*total_units < defender_hits) {
                defender_hits -= *total_units;
                other_land_units_ptr_0_src_land[BOMBERS_LAND_AIR] -= *total_units;
                *units_land_player_total_0_src_land -= *total_units;
                *total_units = 0;
              } else {
                *total_units -= defender_hits;
                other_land_units_ptr_0_src_land[BOMBERS_LAND_AIR] -= defender_hits;
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
        if (actually_print) {
          printf("No friendlies remain");
        }
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
    }
    uint combat_rounds = 0;
    while (true) {
      combat_rounds++;
      // print land location name
#ifdef DEBUG
      if (actually_print) {
        printf("Current land battle start src_land: %d, Name: %s\n", src_land,
               LANDS[src_land].name);
        setPrintableStatus();
        printf("%s\n", printableGameStatus);
      }
#endif
      if (*units_land_player_total_0_src_land == 0) {
#ifdef DEBUG
        if (actually_print) {
          printf("No friendlies remain");
        }
#endif
        break;
      }

      if (state.flagged_for_combat[src_land] == 1) {
        // ask to retreat (0-255, any non valid retreat zone is considered a no)
        valid_moves[0] = src_land;
        valid_moves_count = 1;
        L2LConn land_to_land_conn = LAND_TO_LAND_CONN[src_land];
        uint land_to_land_count = LAND_TO_LAND_COUNT[src_land];
        for (uint land_conn_idx = 0; land_conn_idx < land_to_land_count; land_conn_idx++) {
          uint land_dst = land_to_land_conn[land_conn_idx];
          if (enemy_units_count[land_dst] == 0 && state.flagged_for_combat[land_dst] == 0 &&
              is_allied_0[*owner_idx[land_dst]])
            valid_moves[valid_moves_count++] = land_dst;
        }
        uint dst_air;
        if (valid_moves_count == 1) {
          dst_air = valid_moves[0];
        } else {
          if (answers_remaining == 0)
            return true;
          dst_air = ask_to_retreat();
        }
        // if retreat, move units to retreat zone immediately and end battle
        // max 100 combat rounds for fighters facing aa guns with unlucky dice
        if (src_land != dst_air || combat_rounds > MAX_COMBAT_ROUNDS) {
#ifdef DEBUG
          if (actually_print) {
            printf("Retreating land_battle from: %d to: %d\n", src_land, dst_air);
          }
#endif
          for (uint unit_type = INFANTRY; unit_type <= TANKS; unit_type++) {
            uint total_units = land_units_state[src_land][unit_type][0];
            land_units_state[dst_air][unit_type][0] += total_units;
            current_player_land_unit_types[dst_air][unit_type] += total_units;
            total_player_land_units[0][dst_air] += total_units;
            current_player_land_unit_types[src_land][unit_type] -= total_units;
            *units_land_player_total_0_src_land -= total_units;
            land_units_state[src_land][unit_type][0] = 0;
          }
          state.flagged_for_combat[src_land] = 0;
#ifdef DEBUG
          debug_checks();
#endif
          break;
        }
      }
      state.flagged_for_combat[src_land] = 1;
      // land_battle
      uint infantry_count = current_player_land_unit_types[src_land][INFANTRY];
      uint artillery_count = current_player_land_unit_types[src_land][ARTILLERY];
      attacker_damage =
          (current_player_land_unit_types[src_land][FIGHTERS] * FIGHTER_ATTACK) +
          (current_player_land_unit_types[src_land][BOMBERS_LAND_AIR] * BOMBER_ATTACK) +
          (infantry_count * INFANTRY_ATTACK) + (artillery_count * ARTILLERY_ATTACK) +
          (current_player_land_unit_types[src_land][TANKS] * TANK_ATTACK);
      // add damage for the minimum of count of infantry/artillery
      attacker_damage += infantry_count < artillery_count ? infantry_count : artillery_count;
      attacker_hits = get_attacker_hits(attacker_damage);
      uint defender_damage = 0;
      uint defender_hits = 0;
#ifdef DEBUG
      if (actually_print) {
        printf("Enemy Count: %d\n", enemy_units_count[src_land]);
      }
#endif
      for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
        auto land_units = total_player_land_unit_types[enemies_0[enemy_idx]][src_land];
        defender_damage += (land_units[INFANTRY] * INFANTRY_DEFENSE) +
                           (land_units[ARTILLERY] * ARTILLERY_DEFENSE) +
                           (land_units[TANKS] * TANK_DEFENSE) +
                           (land_units[FIGHTERS] * FIGHTER_DEFENSE) +
                           (land_units[BOMBERS_LAND_AIR] * BOMBER_DEFENSE);
        defender_hits = get_defender_hits(defender_damage);
      }
      if (defender_hits > 0) {
#ifdef DEBUG
        if (actually_print) {
          printf("Defender Hits: %d", defender_hits);
        }
#endif
        remove_land_attackers(src_land, defender_hits);
#ifdef DEBUG
        debug_checks();
#endif
      }
      if (attacker_hits > 0) {
#ifdef DEBUG
        if (actually_print) {
          printf("Attacker Hits: %d", attacker_hits);
        }
#endif
        remove_land_defenders(src_land, attacker_hits);
#ifdef DEBUG
        debug_checks();
#endif
      }

      if (*units_land_player_total_0_src_land == 0) {
#ifdef DEBUG
        if (actually_print) {
          printf("No friendlies remain");
        }
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
    }
  }
  return false;
}

void add_valid_unload_moves(uint src_sea) {
  S2LConn near_land = SEA_TO_LAND_CONN[src_sea];
  uint near_land_count = SEA_TO_LAND_COUNT[src_sea];
  for (uint land_idx = 0; land_idx < near_land_count; land_idx++) {
    uint dst_land = near_land[land_idx];
    // add_valid_air_move_if_history_allows_X(dst_land, src_sea + LANDS_COUNT, 1);
    if (!state.skipped_moves[src_sea + LANDS_COUNT][dst_land].bit) {
      valid_moves[valid_moves_count++] = dst_land;
    }
  }
}

void add_valid_fighter_moves(uint src_air, uint remaining_moves) {
#ifdef DEBUG
  if (actually_print) {
    printf("DEBUG: add_valid_fighter_moves: src_air: %d, remaining_moves: %d\n", src_air,
           remaining_moves);
  }
#endif
  AirArray near_air = AIR_WITHIN_X_MOVES[remaining_moves - 1][src_air];
  uint near_air_count = AIR_WITHIN_X_MOVES_COUNT[remaining_moves - 1][src_air];
  for (uint i = 0; i < near_air_count; i++) {
    uint dst_air = near_air[i];
    uint air_dist = AIR_DIST[src_air][dst_air];
    if (air_dist <= 2 || canFighterLandHere[dst_air] ||
        (air_dist == 3 && canFighterLandIn1Move[dst_air])) {
      if (!canFighterLandHere[dst_air] && enemy_units_count[dst_air] == 0) // waste of a move
        continue;
      // add_valid_air_move_if_history_allows_X(dst_air, src_air, air_dist);
      if (!state.skipped_moves[src_air][dst_air].bit) {
        valid_moves[valid_moves_count++] = dst_air;
      }
    }
  }
}

void add_valid_fighter_landing(uint src_air, uint remaining_moves) {
  AirArray near_air = AIR_WITHIN_X_MOVES[remaining_moves - 1][src_air];
  uint near_air_count = AIR_WITHIN_X_MOVES_COUNT[remaining_moves - 1][src_air];
  for (uint i = 0; i < near_air_count; i++) {
    uint dst_air = near_air[i];
    if (canFighterLandHere[dst_air]) {
      // add_valid_air_move_if_history_allows_X(dst_air, src_air, remaining_moves);
      if (!state.skipped_moves[src_air][dst_air].bit) {
        valid_moves[valid_moves_count++] = dst_air;
      }
    }
  }
}

void add_valid_bomber_moves(uint src_air, uint remaining_moves) {
  AirArray near_air = AIR_WITHIN_X_MOVES[remaining_moves - 1][src_air];
  uint near_air_count = AIR_WITHIN_X_MOVES_COUNT[remaining_moves - 1][src_air];
  for (uint i = 0; i < near_air_count; i++) {
    uint dst_air = near_air[i];
    uint air_dist = AIR_DIST[src_air][dst_air];
    if (air_dist <= 3 || canBomberLandHere[dst_air] ||
        (air_dist == 4 && canBomberLandIn2Moves[dst_air]) ||
        (air_dist == 5 && canBomberLandIn1Move[dst_air])) {
      if (!canBomberLandHere[dst_air] && enemy_units_count[dst_air] == 0) {
        if (dst_air >= LANDS_COUNT || *factory_max[dst_air] == 0 ||
            *factory_hp[dst_air] == -*factory_max[dst_air]) // waste of a move
          continue;
      }
      // add_valid_air_move_if_history_allows_X(dst_air, src_air, air_dist);
      if (!state.skipped_moves[src_air][dst_air].bit) {
        valid_moves[valid_moves_count++] = dst_air;
      }
    }
  }
}

void refresh_can_fighter_land_here() {
  //  refresh_canFighterLandHere_final
  FILL_ARRAY(canFighterLandHere, 0);
  for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    // if (allied_carriers[sea_idx] > 0) {
    canFighterLandHere[sea_idx + LANDS_COUNT] = allied_carriers[sea_idx] > 0;
    //}
  }
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    uint land_owner = *owner_idx[land_idx];
    // is allied owned and not recently conquered?
    canFighterLandHere[land_idx] =
        is_allied_0[land_owner] && state.flagged_for_combat[land_idx] == 0;
    // check for possiblity to build carrier under fighter
    if (*factory_max[land_idx] > 0 && land_owner == state.player_index) {
      uint land_to_sea_count = LAND_TO_SEA_COUNT[land_idx];
      for (uint conn_idx = 0; conn_idx < land_to_sea_count; conn_idx++) {
        canFighterLandHere[LANDS_COUNT + LAND_TO_SEA_CONN[land_idx][conn_idx]] = true;
      }
    }
  }
}

bool land_fighter_units() {
  bool units_to_process = false;
  //  check if any fighters have moves remaining
  for (uint cur_state = 1; cur_state < FIGHTER_STATES - 1;
       cur_state++) { // TODO optimize to find next fighter faster
    for (uint src_air = 0; src_air < AIRS_COUNT; src_air++) {
      uint* total_fighter_count = &air_units_state[src_air][FIGHTERS][cur_state];
      if (*total_fighter_count == 0)
        continue;
      if (units_to_process == false) {
        units_to_process = true;
        refresh_can_fighter_land_here();
      }
      // valid_moves[0] = src_air;
      valid_moves_count = 0;
      add_valid_fighter_landing(src_air, cur_state);
      if (valid_moves_count == 0 || canFighterLandHere[src_air]) {
        valid_moves[valid_moves_count++] = src_air;
      }
      while (*total_fighter_count > 0) {
        uint dst_air = valid_moves[0];
        if (valid_moves_count > 1) {
          if (answers_remaining == 0)
            return true;
          dst_air = get_user_move_input(FIGHTERS, src_air);
        }
#ifdef DEBUG
        if (actually_print) {
          setPrintableStatus();
          printf("%s\n", printableGameStatus);
          printf("DEBUG: player: %s landing fighters %d, src_air: %d, dst_air: %d\n",
                 PLAYERS[state.player_index].name, FIGHTERS, src_air, dst_air);
        }
#endif
        // update_move_history(dst_air, src_air);
        if (src_air == dst_air) {
          air_units_state[src_air][FIGHTERS][0]++;
          *total_fighter_count -= 1;
          continue;
        }
        air_units_state[dst_air][FIGHTERS][0]++;
        if (dst_air < LANDS_COUNT) {
          total_player_land_units[0][dst_air]++;
          current_player_land_unit_types[dst_air][FIGHTERS]++;
        } else {
          uint dst_sea = dst_air - LANDS_COUNT;
          total_player_sea_units[0][dst_sea]++;
          current_player_sea_unit_types[dst_sea][FIGHTERS]++;
        }
        if (src_air < LANDS_COUNT) {
          total_player_land_units[0][src_air]--;
          current_player_land_unit_types[src_air][FIGHTERS]--;
        } else {
          uint src_sea = src_air - LANDS_COUNT;
          total_player_sea_units[0][src_sea]--;
          current_player_sea_unit_types[src_sea][FIGHTERS]--;
        }
        *total_fighter_count -= 1;
#ifdef DEBUG
        debug_checks();
#endif
      }
    }
  }
  if (units_to_process) {
    clear_move_history();
  }
  return false;
}

void add_valid_bomber_landing(uint src_air, uint movement_remaining) {
#ifdef DEBUG
  if (actually_print) {
    printf("movement_remaining: %d\n", movement_remaining);
  }
#endif
  LandArray near_land = AIR_TO_LAND_WITHIN_X_MOVES[movement_remaining - 1][src_air];
  for (uint i = 0; i < AIR_TO_LAND_WITHIN_X_MOVES_COUNT[movement_remaining - 1][src_air]; i++) {
    uint dst_air = near_land[i];
    if (canBomberLandHere[dst_air]) {
#ifdef DEBUG
      if (actually_print) {
        printf("Adding valid move: %d\n", dst_air);
      }
#endif
      valid_moves[valid_moves_count++] = dst_air;
    }
  }
}
void refresh_can_bomber_land_here() {
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    // is allied owned and not recently conquered?
    canBomberLandHere[land_idx] =
        is_allied_0[*owner_idx[land_idx]] && state.flagged_for_combat[land_idx] == 0;
  }
}

bool land_bomber_units() {
  bool units_to_process = false;
#ifdef DEBUG
  if (actually_print) {
    printf("Landing Bombers\n");
  }
#endif
  // check if any bombers have moves remaining
  for (uint cur_state1 = 0; cur_state1 < BOMBER_LAND_STATES - 2;
       cur_state1++) { // TODO optimize to find next bomber faster
                       //    clear_move_history();
    for (uint src_air = 0; src_air < AIRS_COUNT; src_air++) {
      uint cur_state = (src_air < LANDS_COUNT ? cur_state1 + 1 : cur_state1);
      uint* total_bomber_count = &air_units_state[src_air][BOMBERS_LAND_AIR][cur_state];
      if (*total_bomber_count == 0)
        continue;
      if (units_to_process == false) {
        units_to_process = true;
        refresh_can_bomber_land_here();
      }
#ifdef DEBUG
      if (actually_print) {
        printf("Bomber Count: %d with state %d, in location %d\n", *total_bomber_count, cur_state,
               src_air);
      }
#endif
      // valid_moves[0] = src_air;
      valid_moves_count = 0;
      uint movement_remaining = cur_state + (src_air < LANDS_COUNT ? 0 : 1);
      add_valid_bomber_landing(src_air, movement_remaining);
      while (*total_bomber_count > 0) {
        units_to_process = true;
        if (valid_moves_count == 0) {
          valid_moves[valid_moves_count++] = src_air;
        }
        uint dst_air = valid_moves[0];
        if (valid_moves_count > 1) {
          if (answers_remaining == 0)
            return true;
          dst_air = get_user_move_input(BOMBERS_LAND_AIR, src_air);
        }
#ifdef DEBUG
        if (actually_print) {
          setPrintableStatus();
          printf("%s\n", printableGameStatus);
          printf("valid_moves_count: %d\n", valid_moves_count);
          printf("valid_moves: ");
          for (uint i = 0; i < valid_moves_count; i++) {
            printf("%d ", valid_moves[i]);
          }
          printf("\n");
          printf("DEBUG: player: %s landing bombers %d, src_air: %d, dst_air: %d\n",
                 PLAYERS[state.player_index].name, BOMBERS_LAND_AIR, src_air, dst_air);
        }
#endif
        // update_move_history(dst_air, src_air);
        if (src_air == dst_air) {
          air_units_state[src_air][BOMBERS_LAND_AIR][0]++;
          *total_bomber_count -= 1;
#ifdef DEBUG
          debug_checks();
#endif
          continue;
        }
        air_units_state[dst_air][BOMBERS_LAND_AIR][0]++;
        total_player_land_units[0][dst_air]++;
        current_player_land_unit_types[dst_air][BOMBERS_LAND_AIR]++;
        if (src_air < LANDS_COUNT) {
          total_player_land_units[0][src_air]--;
          current_player_land_unit_types[src_air][BOMBERS_LAND_AIR]--;
        } else {
          uint src_sea = src_air - LANDS_COUNT;
          total_player_sea_units[0][src_sea]--;
          current_player_sea_unit_types[src_sea][BOMBERS_LAND_AIR]--;
        }
        *total_bomber_count -= 1;
#ifdef DEBUG
        debug_checks();
#endif
      }
#ifdef DEBUG
      debug_checks();
#endif
    }
  }
  if (units_to_process) {
    clear_move_history();
  }
  return false;
}
bool end_turn() {
  valid_moves_count = 1;
  valid_moves[0] = MAX_INT;
  if (answers_remaining == 0)
    return true;
  (void)getAIInput();
  return false;
}
bool buy_units() {
  bool units_to_process = false;
#ifdef DEBUG
  if (actually_print) {
    setPrintableStatus();
    printf("%s\nBuying Units\n", printableGameStatus);
  }
#endif
  for (uint factory_idx = 0; factory_idx < total_factory_count[0]; factory_idx++) {
    uint dst_land = factory_locations[0][factory_idx];
#ifdef DEBUG
    if (*owner_idx[dst_land] != 0) {
      if (actually_print) {
        printf("DEBUG: player: %s cannot buy units at %s\n", PLAYERS[state.player_index].name,
               LANDS[dst_land].name);
      }
      cause_breakpoint();
    }
#endif
    if (state.builds_left[dst_land] == 0) {
      continue;
    }
    uint repair_cost = 0;
    // buy sea units
    for (uint sea_idx = 0; sea_idx < LAND_TO_SEA_COUNT[dst_land]; sea_idx++) {
      units_to_process = false;
      uint dst_sea = LAND_TO_SEA_CONN[dst_land][sea_idx];
      uint dst_air = dst_sea + LANDS_COUNT;
      valid_moves[0] = SEA_UNIT_TYPES_COUNT; // pass all units
      uint last_purchased = 0;
      while (state.builds_left[dst_air] > 0) {
        if (state.money[0] < TRANSPORT_COST) {
          state.builds_left[dst_air] = 0;
          break;
        }
        uint units_built = *factory_max[dst_land] - state.builds_left[dst_land];
        if (*factory_hp[dst_land] <= units_built)
          repair_cost = 1 + units_built - *factory_hp[dst_land]; // subtracting a negative
        // add all units that can be bought
        valid_moves_count = 1;
        for (uint unit_type_idx = COST_UNIT_SEA_COUNT - 1; unit_type_idx >= 0; unit_type_idx--) {
          uint unit_type = BUY_UNIT_SEA[unit_type_idx];
          // if (unit_type < last_purchased) //state.skipped_moves
          //   break;
          if (state.skipped_moves[0][unit_type].bit) {
            last_purchased = unit_type;
            break;
          }
          if (state.money[0] < COST_UNIT_SEA[unit_type] + repair_cost) {
            continue;
          }
          if (unit_type == FIGHTERS) {
            uint total_fighters = 0;
            for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
              total_fighters += total_player_sea_unit_types[player_idx][dst_sea][FIGHTERS];
            }
            if (allied_carriers[dst_sea] * 2 <= total_fighters) {
              continue;
            }
          }
          valid_moves[valid_moves_count++] = unit_type;
        }
        if (valid_moves_count == 1) {
          state.builds_left[dst_air] = 0;
          break;
        }
        if (answers_remaining == 0) {
          return true;
        }
        units_to_process = true;
        uint purchase = get_user_purchase_input(dst_air);
        if (purchase == SEA_UNIT_TYPES_COUNT) { // pass all units
          state.builds_left[dst_air] = 0;
          break;
        }
#ifdef DEBUG
        if (actually_print) {
          // print which player is buying which unit at which location
          printf("Player %d buying %s at %s\n", state.player_index, NAMES_UNIT_SEA[purchase],
                 SEAS[dst_sea].name);
        }
#endif
        for (uint sea_idx2 = sea_idx; sea_idx2 < LAND_TO_SEA_COUNT[dst_land]; sea_idx2++) {
          state.builds_left[LAND_TO_SEA_CONN[dst_land][sea_idx2] + LANDS_COUNT]--;
        }
        state.builds_left[dst_land]--;
        *factory_hp[dst_land] += repair_cost;
        state.money[0] -= COST_UNIT_SEA[purchase] + repair_cost;
        sea_units_state[dst_sea][purchase][0]++;
        total_player_sea_units[0][dst_sea]++;
        current_player_sea_unit_types[dst_sea][purchase]++;
        if (purchase > last_purchased) {
          for (uint unit_type2 = last_purchased; unit_type2 < purchase; unit_type2++) {
            state.skipped_moves[0][unit_type2].bit = true;
          }

          last_purchased = purchase;
        }
      }
      if (units_to_process) {
        clear_move_history();
      }
    }
    // buy land units
    valid_moves[0] = LAND_UNIT_TYPES_COUNT; // pass all units
    uint last_purchased = 0;
    units_to_process = false;
    while (state.builds_left[dst_land] > 0) {
      if (state.money[0] < INFANTRY_COST) {
        state.builds_left[dst_land] = 0;
        break;
      }
      uint units_built = *factory_max[dst_land] - state.builds_left[dst_land];
      if (*factory_hp[dst_land] <= units_built)
        repair_cost = 1 + units_built - *factory_hp[dst_land]; // subtracting a negative
      // add all units that can be bought
      valid_moves_count = 1;
      for (uint unit_type = LAND_UNIT_TYPES_COUNT - 1; unit_type >= 0; unit_type--) {
        // if (unit_type < last_purchased)
        //   break;
        if (state.skipped_moves[0][unit_type].bit) {
          last_purchased = unit_type;
          break;
        }
        if (state.money[0] < COST_UNIT_LAND[unit_type] + repair_cost)
          continue;
        valid_moves[valid_moves_count++] = unit_type;
      }
      if (valid_moves_count == 1) {
        state.builds_left[dst_land] = 0;
        break;
      }
      if (answers_remaining == 0)
        return true;
      units_to_process = true;
      uint purchase = get_user_purchase_input(dst_land);
      if (purchase == LAND_UNIT_TYPES_COUNT) { // pass all units
        state.builds_left[dst_land] = 0;
        break;
      }
#ifdef DEBUG
      if (actually_print) {
        // print which player is buying which unit at which location
        printf("Player %d buying %s at %s\n", state.player_index, NAMES_UNIT_LAND[purchase],
               LANDS[dst_land].name);
      }
#endif
      state.builds_left[dst_land]--;
      *factory_hp[dst_land] += repair_cost;
      state.money[0] -= COST_UNIT_LAND[purchase] + repair_cost;
      land_units_state[dst_land][purchase][0]++;
      total_player_land_units[0][dst_land]++;
      total_player_land_unit_types[0][dst_land][purchase]++;
      if (purchase > last_purchased) {
        for (uint unit_type2 = last_purchased; unit_type2 < purchase; unit_type2++) {
          state.skipped_moves[0][unit_type2].bit = true;
        }
      }
      last_purchased = purchase;
    }
    if (units_to_process) {
      clear_move_history();
    }
  }
#ifdef DEBUG
  if (actually_print) {
    setPrintableStatus();
    printf("%s\n", printableGameStatus);
  }
#endif
  return false;
}

void crash_air_units() {
  // crash planes not on friendly land
  pre_move_fighter_units();
  for (uint air_idx = 0; air_idx < LANDS_COUNT; air_idx++) {
    if (canFighterLandHere[air_idx]) {
      continue;
    }
    if (current_player_land_unit_types[air_idx][FIGHTERS] == 0) {
      continue;
    }
#ifdef DEBUG
    if (actually_print) {
      printf("DEBUG: Crashing fighters at %d\n", air_idx);
    }
#endif
    total_player_land_units[0][air_idx] -= current_player_land_unit_types[air_idx][FIGHTERS];
    current_player_land_unit_types[air_idx][FIGHTERS] = 0;
    air_units_state[air_idx][FIGHTERS][0] = 0;
  }
  // crash planes not on allied carriers
  for (uint air_idx = LANDS_COUNT; air_idx < AIRS_COUNT; air_idx++) {
    // units_air_ptr[air_idx][BOMBERS_LAND_AIR][0] = 0;
    uint sea_idx = air_idx - LANDS_COUNT;
    uint free_space = allied_carriers[sea_idx] * 2;
    for (uint player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      free_space -= total_player_sea_unit_types[player_idx][sea_idx][FIGHTERS];
    }
    uint* total_fighter_count = &air_units_state[air_idx][FIGHTERS][0];
    if (free_space < *total_fighter_count) {
      uint fighters_lost = *total_fighter_count - free_space;
#ifdef DEBUG
      if (actually_print) {
        printf("DEBUG: Crashing %d fighters at %d\n", fighters_lost, air_idx);
      }
#endif
      total_player_sea_units[0][sea_idx] -= fighters_lost;
      current_player_sea_unit_types[sea_idx][FIGHTERS] -= fighters_lost;
      *total_fighter_count = free_space;
    }
  }
}
void reset_units_fully() {
  // reset battleship health
  for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    sea_units_state[sea_idx][BATTLESHIPS][0] += current_player_sea_unit_types[sea_idx][BS_DAMAGED];
    current_player_sea_unit_types[sea_idx][BATTLESHIPS] +=
        current_player_sea_unit_types[sea_idx][BS_DAMAGED];
    sea_units_state[sea_idx][BS_DAMAGED][0] = 0;
    sea_units_state[sea_idx][BS_DAMAGED][1] = 0;
    current_player_sea_unit_types[sea_idx][BS_DAMAGED] = 0;
  }
}
// todo BUY FACTORY
void buy_factory() {}
void collect_money() {
  // if player still owns their capital, collect income
  state.money[0] +=
      (income_per_turn[0] * (*owner_idx[PLAYERS[state.player_index].capital_territory_index] == 0));
}
void rotate_turns() {
  // rotate units
#ifdef DEBUG
  if (actually_print) {
    setPrintableStatus();
    printf("%s\n", printableGameStatus);
    printf("DEBUG: Rotating turns\n");
  }
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    if (total_player_land_units[0][land_idx] > 0 && enemy_units_count[land_idx] > 0) {
      printf("DEBUG: Player %s has %d units at %s\n", PLAYERS[state.player_index].name,
             total_player_land_units[0][land_idx], LANDS[land_idx].name);
      cause_breakpoint();
    }
  }
  for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    if (total_player_sea_units[0][sea_idx] > 0 && enemy_units_count[sea_idx + LANDS_COUNT] > 0) {
      if (actually_print) {
        printf("DEBUG: Player %s has %d units at %s\n", PLAYERS[state.player_index].name,
               total_player_sea_units[0][sea_idx], SEAS[sea_idx].name);
      }
    }
  }
#endif
  COPY_FULL_ARRAY(current_player_land_unit_types, total_land_unit_types_temp);
  COPY_FULL_ARRAY(state.other_land_units[0], current_player_land_unit_types);
  memmove(&state.other_land_units[0], &state.other_land_units[1],
          sizeof(state.other_land_units[0]) * (PLAYERS_COUNT - 2));
  memcpy(&state.other_land_units[PLAYERS_COUNT - 2], &total_land_unit_types_temp,
         sizeof(state.other_land_units[0]));
  for (uint dst_land = 0; dst_land < LANDS_COUNT; dst_land++) {
    LandState* land0 = &state.land_state[dst_land];
    auto land1 = current_player_land_unit_types[dst_land];
    // todo optimize restructuring
    memset(&state.land_state[dst_land].fighters, 0, sizeof(state.land_state[0].fighters));
    land0->fighters[FIGHTER_STATES - 1] = land1[FIGHTERS];
    memset(&state.land_state[dst_land].bombers, 0, sizeof(state.land_state[0].bombers));
    land0->bombers[BOMBER_LAND_STATES - 1] = land1[BOMBERS_LAND_AIR];
    memset(&state.land_state[dst_land].infantry, 0, sizeof(state.land_state[0].infantry));
    land0->infantry[INFANTRY_STATES - 1] = land1[INFANTRY];
    memset(&state.land_state[dst_land].artillery, 0, sizeof(state.land_state[0].artillery));
    land0->artillery[ARTILLERY_STATES - 1] = land1[ARTILLERY];
    memset(&state.land_state[dst_land].tanks, 0, sizeof(state.land_state[0].tanks));
    land0->tanks[TANK_STATES - 1] = land1[TANKS];
    memset(&state.land_state[dst_land].aa_guns, 0, sizeof(state.land_state[0].aa_guns));
    land0->aa_guns[AA_GUN_STATES - 1] = land1[AA_GUNS];
  }
  memcpy(&total_sea_units_temp, &current_player_sea_unit_types, SEA_UNIT_TYPES_COUNT * SEAS_COUNT);
  //  memcpy(&other_sea_units_0, &data.other_sea_units[0], OTHER_SEA_UNITS_SIZE);
  for (uint dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
    memcpy(&current_player_sea_unit_types[dst_sea], &state.other_sea_units[0][dst_sea],
           SEA_UNIT_TYPES_COUNT - 1);
  }
  memmove(&state.other_sea_units[0], &state.other_sea_units[1],
          sizeof(state.other_sea_units[0]) * (PLAYERS_COUNT - 2));
  //  memcpy(&data.other_sea_units[PLAYERS_COUNT - 2], &other_sea_units_temp, OTHER_SEA_UNITS_SIZE);
  memset(&state.units_sea, 0, sizeof(state.units_sea));
  for (uint dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
    memcpy(&state.other_sea_units[PLAYERS_COUNT - 2][dst_sea], &total_sea_units_temp[dst_sea],
           SEA_UNIT_TYPES_COUNT - 1);
    for (uint unit_type = 0; unit_type < LAND_UNIT_TYPES_COUNT; unit_type++) {
      UnitsSea* sea0 = &state.units_sea[dst_sea];
      auto sea1 = current_player_sea_unit_types[dst_sea];
      sea0->fighters[FIGHTER_STATES - 1] = sea1[FIGHTERS];
      sea0->trans_empty[TRANS_EMPTY_STATES - 1] = sea1[TRANS_EMPTY];
      sea0->trans_1i[TRANS_1I_STATES - 1] = sea1[TRANS_1I];
      sea0->trans_1a[TRANS_1A_STATES - 1] = sea1[TRANS_1A];
      sea0->trans_1t[TRANS_1T_STATES - 1] = sea1[TRANS_1T];
      sea0->trans_2i[TRANS_2I_STATES - 1] = sea1[TRANS_2I];
      sea0->trans_1i_1a[TRANS_1I_1A_STATES - 1] = sea1[TRANS_1I_1A];
      sea0->trans_1i_1t[TRANS_1I_1T_STATES - 1] = sea1[TRANS_1I_1T];
      sea0->submarines[SUB_STATES - 1] = sea1[SUBMARINES];
      sea0->destroyers[DESTROYER_STATES - 1] = sea1[DESTROYERS];
      sea0->carriers[CARRIER_STATES - 1] = sea1[CARRIERS];
      sea0->cruisers[CRUISER_STATES - 1] = sea1[CRUISERS];
      sea0->battleships[BATTLESHIP_STATES - 1] = sea1[BATTLESHIPS];
      sea0->bs_damaged[BATTLESHIP_STATES - 1] = sea1[BS_DAMAGED];
      sea0->bombers[BOMBER_SEA_STATES - 1] = sea1[BOMBERS_SEA];
    }
  }
  uint temp_money = state.money[0];
  memmove(&state.money[0], &state.money[1], sizeof(state.money[0]) * (PLAYERS_COUNT - 1));
  state.money[PLAYERS_COUNT - 1] = temp_money;
  income_per_turn[PLAYERS_COUNT] = income_per_turn[0];
  memmove(&income_per_turn[0], &income_per_turn[1], sizeof(income_per_turn[0]) * PLAYERS_COUNT);
  total_factory_count[PLAYERS_COUNT] = total_factory_count[0];
  memmove(&total_factory_count[0], &total_factory_count[1],
          sizeof(total_factory_count[0]) * PLAYERS_COUNT);
  memcpy(&factory_locations[PLAYERS_COUNT], &factory_locations[0], sizeof(factory_locations[0]));
  memmove(&factory_locations[0], &factory_locations[1],
          sizeof(factory_locations[0]) * PLAYERS_COUNT);
  memcpy(&total_player_land_units[PLAYERS_COUNT], &total_player_land_units[0],
         sizeof(total_player_land_units[0]));
  memmove(&total_player_land_units[0], &total_player_land_units[1],
          sizeof(total_player_land_units[0]) * PLAYERS_COUNT);
  memcpy(&total_player_sea_units[PLAYERS_COUNT], &total_player_sea_units[0],
         sizeof(total_player_sea_units[0]));
  memmove(&total_player_sea_units[0], &total_player_sea_units[1],
          sizeof(total_player_sea_units[0]) * PLAYERS_COUNT);
  // reset combat flags
  memset(&state.flagged_for_combat, 0, sizeof(state.flagged_for_combat));
  state.player_index = (state.player_index + 1) % PLAYERS_COUNT;

  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    state.land_state[land_idx].owner_idx =
        (state.land_state[land_idx].owner_idx + PLAYERS_COUNT - 1) % PLAYERS_COUNT;
  }

  for (uint factory_idx = 0; factory_idx < total_factory_count[0]; factory_idx++) {
    uint dst_land = factory_locations[0][factory_idx];
    state.builds_left[dst_land] = *factory_max[dst_land];
    for (uint sea_idx = 0; sea_idx < LAND_TO_SEA_COUNT[dst_land]; sea_idx++) {
      state.builds_left[LAND_TO_SEA_CONN[dst_land][sea_idx] + LANDS_COUNT] +=
          *factory_max[dst_land];
    }
  }

  refresh_eot_cache();
#ifdef DEBUG
  if (actually_print) {
    printf("DEBUG: Cache refreshed. Player %s's turn\n", PLAYERS[state.player_index].name);
    setPrintableStatus();
    printf("%s\n", printableGameStatus);
  }
  for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    for (uint factory_index = 0; factory_index < total_factory_count[player_idx]; factory_index++) {
      uint factory_location = factory_locations[player_idx][factory_index];
      if (*owner_idx[factory_location] != player_idx) {
        printf("DEBUG: Player %s has a unowned factory at %s\n", PLAYERS[state.player_index].name,
               LANDS[factory_location].name);
        cause_breakpoint();
      }
    }
  }
#endif
}

double get_score() {

  // Evaluate the game state and return a score
  uint allied_score = 1; // one helps prevent division by zero
  uint enemy_score = 1;
  allied_score += state.money[0];
  for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    uint score = state.money[player_idx];
    for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
      for (uint unit_type = 0; unit_type < LAND_UNIT_TYPES_COUNT; unit_type++) {
        score += total_player_land_unit_types[player_idx][land_idx][unit_type] *
                 COST_UNIT_LAND[unit_type];
      }
    }
    for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
      for (uint unit_type = 0; unit_type < SEA_UNIT_TYPES_COUNT; unit_type++) {
        score +=
            total_player_sea_unit_types[player_idx][sea_idx][unit_type] * COST_UNIT_SEA[unit_type];
      }
    }
    if (PLAYERS[state.player_index].is_allied[(state.player_index + player_idx) % PLAYERS_COUNT]) {
      allied_score += score;
    } else {
      enemy_score += score;
    }
  }
  double score = ((double)allied_score / (double)(enemy_score + allied_score));

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

void get_possible_actions(GameState* game_state, uint* num_actions, ActionsPtr actions) {
  // Return the list of possible actions from the given state
  // if (MCTS_ITERATIONS == 6767) {
  //   actually_print = true;
  // }
  // uint starting_player = game_state->player_index;
  // if (starting_player >= PLAYERS_COUNT) {
  //   *num_actions = 1;
  //   (*actions)[0] = MAX_int;
  //   return;
  // }
  //  unlucky_player_idx = state.player_index;
  unlucky_player_idx = 0;
  memcpy(&state, game_state, sizeof(GameState));
  refresh_full_cache();
  answers_remaining = 0;
  random_number_index = 0;
  // clear_move_history();
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
    // if (end_turn())
    //   break;
    crash_air_units();
    reset_units_fully();
    buy_factory();
    collect_money();
    rotate_turns();
  }
  *num_actions = valid_moves_count;
  // memcpy(actions, valid_moves, valid_moves_count * sizeof(uint));
  std::memcpy(actions, &valid_moves, valid_moves_count * sizeof(uint));
  // std::copy(&valid_moves,&valid_moves + valid_moves_count, actions);
}
void apply_action(GameState* game_state, uint action) {
  // Apply the action to the game state

#ifdef DEBUG
  if (actually_print) {
    printf("DEBUG: copying state and Applying action %d\n", action);
  }
#endif
  // uint starting_player = game_state->player_index;
  // if (starting_player >= PLAYERS_COUNT) {
  //   game_state->player_index -= PLAYERS_COUNT;
  //   return;
  // }
  memcpy(&state, game_state, sizeof(GameState));
  refresh_full_cache();
  //  unlucky_player_idx = state.player_index;
  unlucky_player_idx = 0;
  answers_remaining = 1;
  selected_action = action;
  use_selected_action = true;
  random_number_index = 0;
  // clear_move_history();
  //  RANDOM_NUMBERS[random_number_index] = action;
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
    // if (end_turn())
    //   break;
    crash_air_units();
    reset_units_fully();
    buy_factory();
    collect_money();
    rotate_turns();
  }
  // if (PLAYERS[starting_player % PLAYERS_COUNT].is_allied[(state.player_index)]) {
  //   // printf("(adding to player_index) Player %s is allied with player %s\n",
  //   // PLAYERS[starting_player % PLAYERS_COUNT].name,
  //   //       PLAYERS[state.player_index].name);
  //   state.player_index += PLAYERS_COUNT;
  // }
  memcpy(game_state, &state, sizeof(GameState));
}
double random_play_until_terminal(GameState* game_state) {
  memcpy(&state, game_state, sizeof(GameState));
  // uint starting_player = state.player_index;
  //  if (starting_player >= PLAYERS_COUNT) {
  //    state.player_index -= PLAYERS_COUNT;
  //  }
  //  if (MCTS_ITERATIONS == 7549) {
  //    printf("%d, %d\n", seed, random_number_index);
  //    json = serialize_game_data_to_json(game_state);
  //    write_json_to_file("debug_data.json", json);
  //    cJSON_Delete(json);
  //    actually_print = true;
  //  }
  refresh_full_cache();
  answers_remaining = 100000;
  use_selected_action = false;
  double score = get_score();
  max_loops = 1000;
  random_number_index = (uint)(rand() % RANDOM_NUMBERS_SIZE);
  // clear_move_history();
  while (score > 0.01 && score < 0.99 && max_loops-- > 0) {
    // printf("max_loops: %d\n", max_loops);
    //  if(max_loops == 2) {
    //    setPrintableStatus();
    //    printf("%s\n", printableGameStatus);
    //    printf("DEBUG: max_loops reached\n");
    //  }
    // if (max_loops % 100 == 0) {
    //   printf("max_loops: %d\n", max_loops);
    // }
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
    //    end_turn();
    crash_air_units();
    reset_units_fully();
    buy_factory();
    collect_money();
    rotate_turns();
    score = get_score();
  }
  if (state.player_index % 2 == 1) {
    score = 1 - score;
  }
  return score;
}

bool is_terminal_state(GameState* game_state) {
  // Return true if the game is over
  double score = evaluate_state(game_state);
  return (score > 0.99 || score < 0.01);
}

double evaluate_state(GameState* game_state) {
  uint starting_player = game_state->player_index;
  if (starting_player >= PLAYERS_COUNT) {
    game_state->player_index -= PLAYERS_COUNT;
  }
  // Evaluate the game state and return a score
  uint allied_score = 1; // one helps prevent division by zero
  uint enemy_score = 1;
  allied_score += game_state->money[0];
  for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    uint score = game_state->money[player_idx];
    for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
      for (uint unit_type = 0; unit_type < LAND_UNIT_TYPES_COUNT; unit_type++) {
        score += total_player_land_unit_types[player_idx][land_idx][unit_type] *
                 COST_UNIT_LAND[unit_type];
      }
    }
    for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
      for (uint unit_type = 0; unit_type < SEA_UNIT_TYPES_COUNT; unit_type++) {
        score +=
            total_player_sea_unit_types[player_idx][sea_idx][unit_type] * COST_UNIT_SEA[unit_type];
      }
    }
    if (PLAYERS[game_state->player_index]
            .is_allied[(game_state->player_index + player_idx) % PLAYERS_COUNT]) {
      allied_score += score;
    } else {
      enemy_score += score;
    }
  }
  double score = ((double)allied_score / (double)(enemy_score + allied_score));
  game_state->player_index = starting_player;
  if (starting_player >= PLAYERS_COUNT) {
    return 1 - score;
  }
  return score;
}
void load_single_game() {
  random_number_index = 34282;
  answers_remaining = 100000;
  use_selected_action = false;
  double score = get_score();
  actually_print = true;
  max_loops = 1000;
  uint player_idx = 4;
  uint sea_idx1 = 2;
  printf("\nINITIAL: %d, %d, %d\n", total_player_sea_units[player_idx][sea_idx1],
         current_player_sea_unit_types[sea_idx1][0],
         total_player_sea_unit_types[player_idx][sea_idx1][0]);
  setPrintableStatus();
  printf("%s\n", printableGameStatus);
  while (score > 0.01 && score < 0.99 && max_loops-- > 0) {
    // printf("max_loops: %d\n", max_loops);
    //  if(max_loops == 2) {
    //    setPrintableStatus();
    //    printf("%s\n", printableGameStatus);
    //    printf("DEBUG: max_loops reached\n");
    //  }
    // if (max_loops % 100 == 0) {
    //   printf("max_loops: %d\n", max_loops);
    // }
    debug_checks();
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
    //    end_turn();
    crash_air_units();
    reset_units_fully();
    buy_factory();
    collect_money();
    rotate_turns();
    score = get_score();
  }
}

PYBIND11_MODULE(engine, handle) {
  handle.doc() = "caaa engine doc";
  handle.def("random_play_until_terminal", &random_play_until_terminal);
  handle.def("clone_state", &clone_state);
  //handle.def("get_possible_actions", &get_possible_actions);
  handle.def("apply_action", &apply_action);
  handle.def("is_terminal_state", &is_terminal_state);
  handle.def("initialize_constants", &initialize_constants);
  handle.def("get_game_state_copy", &get_game_state_copy);
  handle.def("evaluate_state", &evaluate_state);
  handle.def("load_game_data", &load_game_data);
}
