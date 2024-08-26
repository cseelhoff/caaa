#include "engine.h"
#include "game_data.h"
#include "land.h"
#include "player.h"
#include "sea.h"
#include "serialize_data.h"
#include "units/fighter.h"
#include "units/transport.h"
#include "units/units.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

uint8_t BLOCKADE_UNIT_TYPES[BLOCKADE_UNIT_TYPES_COUNT] = {DESTROYERS, CARRIERS, CRUISERS,
                                                          BATTLESHIPS, BS_DAMAGED};
// char* LAND_NAMES[LANDS_COUNT] = {0};
// char* SEA_NAMES[SEAS_COUNT] = {0};
// char* AIR_NAMES[AIRS_COUNT] = {0};
uint8_t LAND_VALUE[LANDS_COUNT] = {0};
uint8_t LAND_DIST[LANDS_COUNT][AIRS_COUNT] = {0};
uint8_t AIR_CONN_COUNT[AIRS_COUNT] = {0};
uint8_t AIR_CONNECTIONS[AIRS_COUNT][MAX_AIR_TO_AIR_CONNECTIONS] = {0};
uint8_t AIR_DIST[AIRS_COUNT][AIRS_COUNT] = {0};
uint8_t LAND_PATH[LAND_MOVE_SIZE][LANDS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
uint8_t land_path1[LANDS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
// uint8_t land_path2[LANDS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
uint8_t LAND_PATH_ALT[LANDS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
uint8_t LANDS_WITHIN_1_MOVE[LANDS_COUNT][LANDS_COUNT - 1] = {0};
uint8_t LANDS_WITHIN_1_MOVE_COUNT[LANDS_COUNT] = {0};
uint8_t LANDS_WITHIN_2_MOVES[LANDS_COUNT][LANDS_COUNT - 1] = {0};
uint8_t LANDS_WITHIN_2_MOVES_COUNT[LANDS_COUNT] = {0};
uint8_t LOAD_WITHIN_2_MOVES[LANDS_COUNT][SEAS_COUNT] = {0};
uint8_t LOAD_WITHIN_2_MOVES_COUNT[LANDS_COUNT] = {0};
uint8_t UNLOAD_WITHIN_1_MOVE[SEAS_COUNT][LANDS_COUNT] = {0};
uint8_t UNLOAD_WITHIN_1_MOVE_COUNT[SEAS_COUNT] = {0};
uint8_t SEAS_WITHIN_1_MOVE[CANAL_STATES][SEAS_COUNT][SEAS_COUNT - 1] = {0};
uint8_t SEAS_WITHIN_1_MOVE_COUNT[CANAL_STATES][SEAS_COUNT] = {0};
uint8_t SEAS_WITHIN_2_MOVES[CANAL_STATES][SEAS_COUNT][SEAS_COUNT - 1] = {0};
uint8_t SEAS_WITHIN_2_MOVES_COUNT[CANAL_STATES][SEAS_COUNT] = {0};
uint8_t AIR_WITHIN_X_MOVES[6][AIRS_COUNT][AIRS_COUNT - 1] = {0};
uint8_t AIR_WITHIN_X_MOVES_COUNT[6][AIRS_COUNT] = {0};
uint8_t AIR_TO_LAND_WITHIN_X_MOVES[5][AIRS_COUNT][LANDS_COUNT - 1] = {0};
uint8_t AIR_TO_LAND_WITHIN_X_MOVES_COUNT[5][AIRS_COUNT] = {0};
uint8_t SEA_DIST[CANAL_STATES][SEAS_COUNT][SEAS_COUNT] = {0};
uint8_t sea_dist[SEAS_COUNT][AIRS_COUNT] = {0}; // TODO optimize sea_count rather than air_count
uint8_t SEA_PATH[SEA_MOVE_SIZE][CANAL_STATES][SEAS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
uint8_t SEA_PATH_ALT[CANAL_STATES][SEAS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
uint8_t sea_path1[SEAS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
uint8_t sea_path2[SEAS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
uint8_t sea_path_alt1[SEAS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
uint8_t AIR_PATH[AIR_MOVE_SIZE][AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
uint8_t air_path2[AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
uint8_t air_path3[AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
uint8_t air_path4[AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
uint8_t air_path5[AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
uint8_t air_path6[AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
uint8_t LAND_TO_LAND_COUNT[LANDS_COUNT] = {0};
uint8_t LAND_TO_LAND_CONN[LANDS_COUNT][MAX_LAND_TO_LAND_CONNECTIONS] = {0};
uint8_t LAND_TO_SEA_COUNT[LANDS_COUNT] = {0};
uint8_t LAND_TO_SEA_CONN[LANDS_COUNT][MAX_LAND_TO_SEA_CONNECTIONS] = {0};
uint8_t SEA_TO_SEA_COUNT[SEAS_COUNT] = {0};
uint8_t SEA_TO_SEA_CONN[SEAS_COUNT][MAX_SEA_TO_SEA_CONNECTIONS] = {0};
uint8_t SEA_TO_LAND_COUNT[SEAS_COUNT] = {0};
uint8_t SEA_TO_LAND_CONN[SEAS_COUNT][MAX_SEA_TO_LAND_CONNECTIONS] = {0};

uint8_t ORDER_OF_LAND_DEFENDERS[DEFENDER_LAND_UNIT_TYPES_COUNT] = {
    AA_GUNS, BOMBERS_LAND_AIR, INFANTRY, ARTILLERY, TANKS, FIGHTERS};

uint8_t ORDER_OF_LAND_ATTACKERS_1[ATTACKER_LAND_UNIT_TYPES_COUNT_1] = {INFANTRY, ARTILLERY, TANKS};
uint8_t ORDER_OF_LAND_ATTACKERS_2[ATTACKER_LAND_UNIT_TYPES_COUNT_2] = {FIGHTERS, BOMBERS_LAND_AIR};

uint8_t ORDER_OF_SEA_DEFENDERS[DEFENDER_SEA_UNIT_TYPES_COUNT] = {
    SUBMARINES, DESTROYERS, CARRIERS, CRUISERS, FIGHTERS,    BS_DAMAGED, TRANS_EMPTY,
    TRANS_1I,   TRANS_1A,   TRANS_1T, TRANS_2I, TRANS_1I_1A, TRANS_1I_1T};
uint8_t ORDER_OF_SEA_ATTACKERS_1[ATTACKER_SEA_UNIT_TYPES_COUNT_1] = {SUBMARINES, DESTROYERS};
uint8_t ORDER_OF_SEA_ATTACKERS_2[ATTACKER_SEA_UNIT_TYPES_COUNT_2] = {FIGHTERS, BOMBERS_SEA};
uint8_t ORDER_OF_SEA_ATTACKERS_3[ATTACKER_SEA_UNIT_TYPES_COUNT_3] = {
    BS_DAMAGED, TRANS_EMPTY, TRANS_1I, TRANS_1A, TRANS_1T, TRANS_2I, TRANS_1I_1A, TRANS_1I_1T};
uint8_t valid_retreats[MAX_SEA_TO_SEA_CONNECTIONS + 1] = {0};
uint8_t retreat_count = 0;
uint8_t RANDOM_NUMBERS[65536] = {0};
u_short random_number_index = 0;
char printableGameStatus[PRINTABLE_GAME_STATUS_SIZE] = "";
GameData data = {0};
cJSON* json;
// int OTHER_LAND_UNITS_SIZE = sizeof(data.other_land_units[0]);
// int OTHER_SEA_UNITS_SIZE = sizeof(data.other_sea_units[0]);
#define OTHER_LAND_UNITS_SIZE 30
#define OTHER_SEA_UNITS_SIZE 42
int MULTI_OTHER_LAND_UNITS_SIZE = (PLAYERS_COUNT - 2) * OTHER_LAND_UNITS_SIZE;
int MULTI_OTHER_SEA_UNITS_SIZE = (PLAYERS_COUNT - 2) * OTHER_SEA_UNITS_SIZE;

uint8_t* land_units_state[LANDS_COUNT][LAND_UNIT_TYPES_COUNT];
uint8_t* sea_units_state[SEAS_COUNT][SEA_UNIT_TYPES_COUNT];
uint8_t* air_units_state[AIRS_COUNT][AIR_UNIT_TYPES_COUNT];
uint8_t* total_player_land_unit_types[PLAYERS_COUNT][LANDS_COUNT];
uint8_t* total_player_sea_unit_types[PLAYERS_COUNT][SEAS_COUNT];
uint8_t* owner_idx[LANDS_COUNT];
uint8_t* factory_max[LANDS_COUNT];
uint8_t* bombard_max[LANDS_COUNT];
int8_t* factory_hp[LANDS_COUNT];                                            // allow negative
uint8_t current_player_land_unit_types[LANDS_COUNT][LAND_UNIT_TYPES_COUNT]; // temp
uint8_t current_player_sea_unit_types[SEAS_COUNT][SEA_UNIT_TYPES_COUNT];    // temp
uint8_t total_land_unit_types_temp[LANDS_COUNT][LAND_UNIT_TYPES_COUNT];     // temp
uint8_t total_sea_units_temp[SEAS_COUNT][SEA_UNIT_TYPES_COUNT];             // temp
int income_per_turn[PLAYERS_COUNT + 1];
int total_factory_count[PLAYERS_COUNT + 1] = {0};
int factory_locations[PLAYERS_COUNT + 1][LANDS_COUNT] = {0};
int total_player_land_units[PLAYERS_COUNT + 1][LANDS_COUNT];
int total_player_sea_units[PLAYERS_COUNT + 1][SEAS_COUNT];
int hist_skipped_airs[AIRS_COUNT][AIRS_COUNT] = {0};
int hist_source_territories[AIRS_COUNT][AIRS_COUNT] = {0};
int hist_source_territories_count[AIRS_COUNT] = {0};

// int units_sea_blockade_total[PLAYERS_COUNT][SEAS_COUNT];
uint8_t enemy_blockade_total[SEAS_COUNT] = {0};
uint8_t enemy_destroyers_total[SEAS_COUNT] = {0};
bool is_land_path_blocked[LANDS_COUNT][LANDS_COUNT] = {0};
bool is_sea_path_blocked[SEAS_COUNT][SEAS_COUNT] = {0};
bool is_sub_path_blocked[SEAS_COUNT][SEAS_COUNT] = {0};
int transports_with_large_cargo_space[SEAS_COUNT];
int transports_with_small_cargo_space[SEAS_COUNT];

int enemies_0[PLAYERS_COUNT - 1] = {0};
bool is_allied_0[PLAYERS_COUNT] = {0};
int enemies_count_0 = 0;
int canal_state = 0;
int allied_carriers[SEAS_COUNT] = {0};
int enemy_units_count[AIRS_COUNT] = {0};
bool canFighterLandHere[AIRS_COUNT] = {0};
bool canFighterLandIn1Move[AIRS_COUNT] = {0};
bool canBomberLandHere[AIRS_COUNT] = {0};
bool canBomberLandIn1Move[AIRS_COUNT] = {0};
bool canBomberLandIn2Moves[AIRS_COUNT] = {0};
int status_message_id = 0;
void initializeGameData() {
  generate_total_land_distance();
  generate_total_sea_distance();
  generate_total_air_distance();
  generate_landMoveAllDestination();
  generate_seaMoveAllDestination();
  generate_airMoveAllDestination();
  generate_within_x_moves();
  generate_random_numbers();
  generate_unit_pointers();
  // set RETREAT_OPTIONS to be an array of values from 0 to 255
  // for (int i = 0; i < 255; i++) {
  //  RETREAT_OPTIONS[i] = i;
  //}
  json = serialize_game_data_to_json(&data);
  write_json_to_file("game_data_0.json", json);
  cJSON_Delete(json);
  DEBUG_PRINT("Exiting initializeGameData");
}

#define PATH_MAX 4096
void load_game_data(char* filename) {
  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    DEBUG_PRINT("Current working directory: ");
    DEBUG_PRINT(cwd);
  } else {
    perror("getcwd() error");
  }
  json = read_json_from_file(filename);
  deserialize_game_data_from_json(&data, json);
  cJSON_Delete(json);
  refresh_quick_totals();
  refresh_cache();
  DEBUG_PRINT("Exiting load_game_data");
}

void set_seed(uint16_t seed) { random_number_index = seed; }

void play_full_turn() {
  // clear printableGameStatus
  move_fighter_units();
  debug_checks();
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
  move_land_unit_type(AA_GUNS);
  debug_checks();
  land_fighter_units();
  land_bomber_units();
  buy_units();
  crash_air_units();
  reset_units_fully();
  buy_factory();
  collect_money();
  rotate_turns();
}

void generate_total_land_distance() {
  // Initialize the total_land_distance array
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    // LAND_NAMES[land_idx] = LANDS[land_idx].name;
    LAND_VALUE[land_idx] = LANDS[land_idx].land_value;
    LAND_TO_LAND_COUNT[land_idx] = LANDS[land_idx].land_conn_count;
    for (int conn_idx = 0; conn_idx < LAND_TO_LAND_COUNT[land_idx]; conn_idx++) {
      LAND_TO_LAND_CONN[land_idx][conn_idx] = LANDS[land_idx].connected_land_index[conn_idx];
    }
    LAND_TO_SEA_COUNT[land_idx] = LANDS[land_idx].sea_conn_count;
    for (int conn_idx = 0; conn_idx < LAND_TO_SEA_COUNT[land_idx]; conn_idx++) {
      LAND_TO_SEA_CONN[land_idx][conn_idx] = LANDS[land_idx].connected_sea_index[conn_idx];
    }
    for (int dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
      if (land_idx != dst_air) {
        LAND_DIST[land_idx][dst_air] = MAX_UINT8_T;
      }
    }
  }

  // Populate initial distances based on connected_sea_index
  for (int i = 0; i < LANDS_COUNT; i++) {
    for (int j = 0; j < LANDS[i].land_conn_count; j++) {
      int land_index = LANDS[i].connected_land_index[j];
      LAND_DIST[i][land_index] = 1;
      LAND_DIST[land_index][i] = 1;
    }
    for (int j = 0; j < LANDS[i].sea_conn_count; j++) {
      int air_index = LANDS[i].connected_sea_index[j] + LANDS_COUNT;
      LAND_DIST[i][air_index] = 1;
    }
  }

  // Floyd-Warshall algorithm to compute shortest paths
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    for (int land_idx2 = 0; land_idx2 < LANDS_COUNT; land_idx2++) {
      for (int air_idx = 0; air_idx < AIRS_COUNT; air_idx++) {
        if (LAND_DIST[land_idx2][land_idx] + LAND_DIST[land_idx][air_idx] <
            LAND_DIST[land_idx2][air_idx]) {
          LAND_DIST[land_idx2][air_idx] =
              LAND_DIST[land_idx2][land_idx] + LAND_DIST[land_idx][air_idx];
        }
      }
    }
  }
}
void generate_total_sea_distance() {
  // Initialize the total_land_distance array
  for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    // SEA_NAMES[src_sea] = SEAS[src_sea].name;
    uint8_t sea_to_sea_count = SEAS[src_sea].sea_conn_count;
    SEA_TO_SEA_COUNT[src_sea] = sea_to_sea_count;
    uint8_t* sea_to_sea_conn = SEA_TO_SEA_CONN[src_sea];
    const Sea* sea = &SEAS[src_sea];
    for (int conn_idx = 0; conn_idx < sea_to_sea_count; conn_idx++) {
      sea_to_sea_conn[conn_idx] = sea->connected_sea_index[conn_idx];
    }
    uint8_t sea_to_land_count = SEAS[src_sea].land_conn_count;
    SEA_TO_LAND_COUNT[src_sea] = sea_to_land_count;
    uint8_t* sea_to_land_conn = SEA_TO_LAND_CONN[src_sea];
    sea = &SEAS[src_sea];
    for (int conn_idx = 0; conn_idx < sea_to_land_count; conn_idx++) {
      sea_to_land_conn[conn_idx] = sea->connected_land_index[conn_idx];
    }
  }
  for (int canal_idx = 0; canal_idx < CANAL_STATES; canal_idx++) {
    // Initialize the total_sea_distance array
    for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      for (int dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
        if (src_sea != dst_sea) {
          SEA_DIST[canal_idx][src_sea][dst_sea] = MAX_UINT8_T;
        }
      }
    }
    int air_index;
    // Populate initial distances based on connected_sea_index
    for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      for (int conn_idx = 0; conn_idx < SEAS[src_sea].sea_conn_count; conn_idx++) {
        int sea_index = SEAS[src_sea].connected_sea_index[conn_idx];
        SEA_DIST[canal_idx][src_sea][sea_index] = 1;
        SEA_DIST[canal_idx][sea_index][src_sea] = 1;
      }
    }

    // convert canal_state to a bitmask and loop through CANALS for those
    // enabled for example if canal_state is 0, do not process any items in
    // CANALS, if canal_state is 1, process the first item in CANALS, if
    // canal_state is 2, process the second item in CANALS, if canal_state is
    // 3, process the first and second items in CANALS, etc.
    for (int j = 0; j < CANALS_COUNT; j++) {
      if ((canal_idx & (1 << j)) == 0) {
        continue;
      }
      SEA_DIST[canal_idx][CANALS[j].seas[0]][CANALS[j].seas[1]] = 1;
      SEA_DIST[canal_idx][CANALS[j].seas[1]][CANALS[j].seas[0]] = 1;
    }
    // Floyd-Warshall algorithm to compute shortest paths
    for (int j = 0; j < SEAS_COUNT; j++) {
      for (int k = 0; k < SEAS_COUNT; k++) {
        for (int l = 0; l < SEAS_COUNT; l++) {
          if (SEA_DIST[canal_idx][k][j] + SEA_DIST[canal_idx][j][l] < SEA_DIST[canal_idx][k][l]) {
            SEA_DIST[canal_idx][k][l] = SEA_DIST[canal_idx][k][j] + SEA_DIST[canal_idx][j][l];
          }
        }
      }
    }
  }
}
void generate_total_air_distance() {
  // Initialize the total_air_distance array
  int src_air;
  int dst_air;
  for (src_air = 0; src_air < AIRS_COUNT; src_air++) {
    for (dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
      if (src_air != dst_air) {
        AIR_DIST[src_air][dst_air] = MAX_UINT8_T;
      } // else {
        // total_air_distance[i][j] = 0;
      //}
    }
  }
  int conn_idx;
  // Populate initial distances based on connected_sea_index and
  // connected_land_index
  for (int src_land = 0; src_land < LANDS_COUNT; src_land++) {
    // AIR_NAMES[src_land] = LAND_NAMES[src_land];
    for (conn_idx = 0; conn_idx < LAND_TO_LAND_COUNT[src_land]; conn_idx++) {
      dst_air = LAND_TO_LAND_CONN[src_land][conn_idx];
      AIR_CONNECTIONS[src_land][AIR_CONN_COUNT[src_land]] = dst_air;
      AIR_CONN_COUNT[src_land]++;
      AIR_DIST[src_land][dst_air] = 1;
      AIR_DIST[dst_air][src_land] = 1;
    }
    for (conn_idx = 0; conn_idx < LAND_TO_SEA_COUNT[src_land]; conn_idx++) {
      dst_air = LAND_TO_SEA_CONN[src_land][conn_idx] + LANDS_COUNT;
      AIR_CONNECTIONS[src_land][AIR_CONN_COUNT[src_land]] = dst_air;
      AIR_CONN_COUNT[src_land]++;
      AIR_DIST[src_land][dst_air] = 1;
      AIR_DIST[dst_air][src_land] = 1;
    }
  }

  for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    int src_air = src_sea + LANDS_COUNT;
    // AIR_NAMES[src_air] = SEA_NAMES[src_sea];
    for (conn_idx = 0; conn_idx < SEA_TO_LAND_COUNT[src_sea]; conn_idx++) {
      dst_air = SEA_TO_LAND_CONN[src_sea][conn_idx];
      AIR_CONNECTIONS[src_air][AIR_CONN_COUNT[src_air]] = dst_air;
      AIR_CONN_COUNT[src_air]++;
      AIR_DIST[src_air][dst_air] = 1;
      AIR_DIST[dst_air][src_air] = 1;
    }
    for (conn_idx = 0; conn_idx < SEA_TO_SEA_COUNT[src_sea]; conn_idx++) {
      dst_air = SEA_TO_SEA_CONN[src_sea][conn_idx] + LANDS_COUNT;
      AIR_CONNECTIONS[src_air][AIR_CONN_COUNT[src_air]] = dst_air;
      AIR_CONN_COUNT[src_air]++;
      AIR_DIST[src_air][dst_air] = 1;
      AIR_DIST[dst_air][src_air] = 1;
    }
  }

  // Floyd-Warshall algorithm to compute shortest paths
  for (int air_index = 0; air_index < AIRS_COUNT; air_index++) {
    for (src_air = 0; src_air < AIRS_COUNT; src_air++) {
      for (dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
        if (AIR_DIST[src_air][air_index] + AIR_DIST[air_index][dst_air] <
            AIR_DIST[src_air][dst_air]) {
          AIR_DIST[src_air][dst_air] = AIR_DIST[src_air][air_index] + AIR_DIST[air_index][dst_air];
        }
      }
    }
  }
}
void generate_LandMoveDst(int hop, int src_land, int dst_air, int cur_land, int min_dist) {
  if (hop > MAX_LAND_HOPS)
    return;

  if (min_dist <= hop)
    LAND_PATH[hop - MIN_LAND_HOPS][src_land][dst_air] = cur_land;

  for (int conn_idx = 0; conn_idx < LAND_TO_LAND_COUNT[cur_land]; conn_idx++) {
    int next_land = LAND_TO_LAND_CONN[cur_land][conn_idx];
    int next_dist = LAND_DIST[next_land][dst_air];
    if (next_dist < min_dist) {
      generate_LandMoveDst(hop + 1, src_land, dst_air, next_land, next_dist);
    }
  }
}
void generate_landMoveAllDestination() {
  for (int src_land = 0; src_land < LANDS_COUNT; src_land++) {
    for (int dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
      int min_dist = LAND_DIST[src_land][dst_air];
      generate_LandMoveDst(1, src_land, dst_air, src_land, min_dist);
      if (min_dist <= 1)
        LAND_PATH_ALT[src_land][dst_air] = src_land;
      for (int conn_idx = LAND_TO_LAND_COUNT[src_land]; conn_idx >= 0; conn_idx--) {
        int next_land = LAND_TO_LAND_CONN[src_land][conn_idx];
        int next_dist = LAND_DIST[next_land][dst_air];
      }
    }
  }
  memcpy(land_path1, LAND_PATH[0], sizeof(land_path1));
}
void generate_SeaMoveDst(int hop, int src_sea, int dst_air, int cur_sea, int min_dist) {
  if (hop > MAX_SEA_HOPS)
    return;
  if (min_dist <= hop)
    SEA_PATH[hop - MIN_SEA_HOPS][canal_state][src_sea][dst_air] = cur_sea;
  for (int conn_idx = 0; conn_idx < SEA_TO_SEA_COUNT[cur_sea]; conn_idx++) {
    int next_sea = SEA_TO_SEA_CONN[cur_sea][conn_idx];
    int next_dist = SEA_DIST[canal_state][next_sea][dst_air];
    if (next_dist < min_dist) {
      generate_SeaMoveDst(hop + 1, src_sea, dst_air, next_sea, next_dist);
    }
  }
}
void generate_seaMoveAllDestination() {
  for (int canal_state = 0; canal_state < CANAL_STATES; canal_state++) {
    for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      for (int dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
        int min_dist = SEA_DIST[canal_state][src_sea][dst_air];
        generate_SeaMoveDst(1, src_sea, dst_air, src_sea, min_dist);
        if (min_dist <= 1)
          SEA_PATH_ALT[canal_state][src_sea][dst_air] = src_sea;
        for (int conn_idx = SEA_TO_SEA_COUNT[src_sea]; conn_idx >= 0; conn_idx--) {
          int next_sea = SEA_TO_SEA_CONN[src_sea][conn_idx];
          int next_dist = SEA_DIST[canal_state][next_sea][dst_air];
        }
      }
    }
  }
}
void generate_AirMoveDst(int hop, int src_air, int dst_air, int cur_air, int min_dist) {
  if (hop > MAX_AIR_HOPS)
    return;
  if (min_dist <= hop)
    AIR_PATH[hop - MIN_AIR_HOPS][src_air][dst_air] = cur_air;
  for (int conn_idx = 0; conn_idx < AIR_CONN_COUNT[cur_air]; conn_idx++) {
    int next_air = AIR_CONNECTIONS[cur_air][conn_idx];
    int next_dist = AIR_DIST[next_air][dst_air];
    if (next_dist < min_dist) {
      generate_AirMoveDst(hop + 1, src_air, dst_air, next_air, next_dist);
    }
  }
}
void generate_airMoveAllDestination() {
  for (int src_air = 0; src_air < AIRS_COUNT; src_air++) {
    for (int dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
      int min_dist = AIR_DIST[src_air][dst_air];
      generate_AirMoveDst(2, src_air, dst_air, src_air, min_dist);
    }
  }
}
void generate_within_x_moves() {
  for (int src_land = 0; src_land < LANDS_COUNT; src_land++) {
    for (int dst_land = 0; dst_land < LANDS_COUNT; dst_land++) {
      if (src_land == dst_land) {
        continue;
      }
      if (LAND_DIST[src_land][dst_land] <= 1) {
        LANDS_WITHIN_1_MOVE[src_land][LANDS_WITHIN_1_MOVE_COUNT[src_land]++] = dst_land;
      }
      if (LAND_DIST[src_land][dst_land] <= 2) {
        LANDS_WITHIN_2_MOVES[src_land][LANDS_WITHIN_2_MOVES_COUNT[src_land]++] = dst_land;
      }
    }
    for (int dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
      // if (LAND_DIST[src_land][dst_sea] <= 1) {
      //   LOAD_WITHIN_1_MOVE[src_land][LOAD_WITHIN_1_MOVE_COUNT[src_land]++] = dst_sea;
      // }
      if (LAND_DIST[src_land][dst_sea] <= 2) {
        LOAD_WITHIN_2_MOVES[src_land][LOAD_WITHIN_2_MOVES_COUNT[src_land]++] = dst_sea;
      }
    }
  }
  for (int canal_state_idx = 0; canal_state_idx < CANAL_STATES; canal_state_idx++) {
    for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      for (int dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
        if (src_sea == dst_sea) {
          continue;
        }
        if (SEA_DIST[canal_state_idx][src_sea][dst_sea] <= 1) {
          SEAS_WITHIN_1_MOVE[canal_state_idx][src_sea]
                            [SEAS_WITHIN_1_MOVE_COUNT[canal_state_idx][src_sea]++] = dst_sea;
        }
        if (SEA_DIST[canal_state_idx][src_sea][dst_sea] <= 2) {
          SEAS_WITHIN_2_MOVES[canal_state_idx][src_sea]
                             [SEAS_WITHIN_2_MOVES_COUNT[canal_state_idx][src_sea]++] = dst_sea;
        }
      }
      for (int dst_land = 0; dst_land < LANDS_COUNT; dst_land++) {
        if (SEA_DIST[canal_state_idx][src_sea][dst_land] <= 1) {
          UNLOAD_WITHIN_1_MOVE[src_sea][UNLOAD_WITHIN_1_MOVE_COUNT[src_sea]++] = dst_land;
        }
      }
    }
  }
  for (int src_air = 0; src_air < AIRS_COUNT; src_air++) {
    for (int dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
      if (src_air == dst_air) {
        continue;
      }
      for (int i = 0; i < 5; i++) {
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

  for (int src_air = 0; src_air < AIRS_COUNT; src_air++) {
    for (int dst_land = 0; dst_land < LANDS_COUNT; dst_land++) {
      if (src_air == dst_land) {
        continue;
      }
      for (int i = 0; i < 5; i++) {
        if (AIR_DIST[src_air][dst_land] <= i + 1) {
          AIR_TO_LAND_WITHIN_X_MOVES[i][src_air][AIR_TO_LAND_WITHIN_X_MOVES_COUNT[i][src_air]++] =
              dst_land;
        }
      }
    }
  }
}
void generate_random_numbers() {
  for (int i = 0; i < 65536; i++) {
    RANDOM_NUMBERS[i] = rand() % 256;
  }
}
void generate_unit_pointers() {
  for (int player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    income_per_turn[player_idx] = 0;
    total_factory_count[player_idx] = 0;
  }
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    air_units_state[land_idx][FIGHTERS] = (uint8_t*)data.land_state[land_idx].fighters;
    air_units_state[land_idx][BOMBERS_LAND_AIR] = (uint8_t*)data.land_state[land_idx].bombers;
    land_units_state[land_idx][FIGHTERS] = (uint8_t*)data.land_state[land_idx].fighters;
    land_units_state[land_idx][BOMBERS_LAND_AIR] = (uint8_t*)data.land_state[land_idx].bombers;
    land_units_state[land_idx][INFANTRY] = (uint8_t*)data.land_state[land_idx].infantry;
    land_units_state[land_idx][ARTILLERY] = (uint8_t*)data.land_state[land_idx].artillery;
    land_units_state[land_idx][TANKS] = (uint8_t*)data.land_state[land_idx].tanks;
    land_units_state[land_idx][AA_GUNS] = (uint8_t*)data.land_state[land_idx].aa_guns;
    total_player_land_unit_types[0][land_idx] = (uint8_t*)current_player_land_unit_types[land_idx];
    for (int player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      total_player_land_unit_types[player_idx][land_idx] =
          (uint8_t*)data.other_land_units[player_idx - 1][land_idx];
    }
    owner_idx[land_idx] = &data.land_state[land_idx].owner_idx;
    bombard_max[land_idx] = &data.land_state[land_idx].bombard_max;
    factory_hp[land_idx] = &data.land_state[land_idx].factory_hp;
    factory_max[land_idx] = &data.land_state[land_idx].factory_max;
  }
  for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    uint8_t air_idx = sea_idx + LANDS_COUNT;
    air_units_state[air_idx][FIGHTERS] = (uint8_t*)data.units_sea[sea_idx].fighters;
    air_units_state[air_idx][BOMBERS_LAND_AIR] = (uint8_t*)data.units_sea[sea_idx].bombers;
    sea_units_state[sea_idx][FIGHTERS] = (uint8_t*)data.units_sea[sea_idx].fighters;
    sea_units_state[sea_idx][TRANS_EMPTY] = (uint8_t*)data.units_sea[sea_idx].trans_empty;
    sea_units_state[sea_idx][TRANS_1I] = (uint8_t*)data.units_sea[sea_idx].trans_1i;
    sea_units_state[sea_idx][TRANS_1A] = (uint8_t*)data.units_sea[sea_idx].trans_1a;
    sea_units_state[sea_idx][TRANS_1T] = (uint8_t*)data.units_sea[sea_idx].trans_1t;
    sea_units_state[sea_idx][TRANS_2I] = (uint8_t*)data.units_sea[sea_idx].trans_2i;
    sea_units_state[sea_idx][TRANS_1I_1A] = (uint8_t*)data.units_sea[sea_idx].trans_1i_1a;
    sea_units_state[sea_idx][TRANS_1I_1T] = (uint8_t*)data.units_sea[sea_idx].trans_1i_1t;
    sea_units_state[sea_idx][SUBMARINES] = (uint8_t*)data.units_sea[sea_idx].submarines;
    sea_units_state[sea_idx][DESTROYERS] = (uint8_t*)data.units_sea[sea_idx].destroyers;
    sea_units_state[sea_idx][CARRIERS] = (uint8_t*)data.units_sea[sea_idx].carriers;
    sea_units_state[sea_idx][CRUISERS] = (uint8_t*)data.units_sea[sea_idx].cruisers;
    sea_units_state[sea_idx][BATTLESHIPS] = (uint8_t*)data.units_sea[sea_idx].battleships;
    sea_units_state[sea_idx][BS_DAMAGED] = (uint8_t*)data.units_sea[sea_idx].bs_damaged;
    sea_units_state[sea_idx][BOMBERS_SEA] = (uint8_t*)data.units_sea[sea_idx].bombers;
    total_player_sea_unit_types[0][sea_idx] = (uint8_t*)current_player_sea_unit_types[sea_idx];
    for (int player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      total_player_sea_unit_types[player_idx][sea_idx] =
          (uint8_t*)data.other_sea_units[player_idx - 1][sea_idx];
    }
  }
}

void refresh_quick_totals() {
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    int land_owner = *owner_idx[land_idx];
    if (*factory_max[land_idx] > 0) {
      factory_locations[land_owner][total_factory_count[land_owner]++] = land_idx;
    }
    income_per_turn[land_owner] += LANDS[land_idx].land_value;
    for (int player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
      uint8_t* cur_units_land_player_total = total_player_land_unit_types[player_idx][land_idx];
      *cur_units_land_player_total = 0;
      for (int unit_idx = 0; unit_idx < LAND_UNIT_TYPES_COUNT; unit_idx++) {
        *cur_units_land_player_total +=
            total_player_land_unit_types[player_idx][land_idx][unit_idx];
      }
    }
  }
  for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    for (int player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
      uint8_t* cur_units_sea_player_total = total_player_sea_unit_types[player_idx][sea_idx];
      *cur_units_sea_player_total = 0;
      for (int unit_idx = 0; unit_idx < SEA_UNIT_TYPES_COUNT; unit_idx++) {
        *cur_units_sea_player_total += total_player_sea_unit_types[player_idx][sea_idx][unit_idx];
      }
    }
  }
}

void refresh_cache() {
  // copy for quick cache lookups
  enemies_count_0 = 0;
  for (int player_idx2 = 0; player_idx2 < PLAYERS_COUNT; player_idx2++) {
    is_allied_0[player_idx2] =
        PLAYERS[data.player_index].is_allied[(data.player_index + player_idx2) % PLAYERS_COUNT];
    if (is_allied_0[player_idx2] == false) {
      enemies_0[enemies_count_0++] = player_idx2;
    }
  }

  canal_state = 0;
  for (int canal_idx = 0; canal_idx < CANALS_COUNT; canal_idx++) {
    if (is_allied_0[*owner_idx[CANALS[canal_idx].lands[0]]] &&
        is_allied_0[*owner_idx[CANALS[canal_idx].lands[1]]]) {
      canal_state += 1 << canal_idx;
    }
  }
  memcpy(sea_dist, SEA_DIST[canal_state], sizeof(sea_dist));
  memcpy(sea_path1, SEA_PATH[0][canal_state], sizeof(sea_path1));
  memcpy(sea_path2, SEA_PATH[1][canal_state], sizeof(sea_path2));
  memcpy(sea_path_alt1, SEA_PATH_ALT[canal_state], sizeof(sea_path_alt1));

  uint8_t unit_count;
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    enemy_units_count[land_idx] = 0;
    for (int enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
      int ememy_player_idx = enemies_0[enemy_idx];
      enemy_units_count[land_idx] += total_player_land_units[ememy_player_idx][land_idx];
    }
  }
  for (int src_land = 0; src_land < LANDS_COUNT; src_land++) {
    for (int dst_land = 0; dst_land < LANDS_COUNT; dst_land++) {
      int nextLandMovement = land_path1[src_land][dst_land];
      int nextLandMovementAlt = LAND_PATH_ALT[src_land][dst_land];
      is_land_path_blocked[src_land][dst_land] =
          (enemy_units_count[nextLandMovement] > 0 || *factory_max[nextLandMovement] > 0) &&
          (enemy_units_count[nextLandMovementAlt] > 0 || *factory_max[nextLandMovementAlt] > 0);
    }
  }
  for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    int air_idx = sea_idx + LANDS_COUNT;
    uint8_t* sea_units_0 = current_player_sea_unit_types[sea_idx];
    allied_carriers[sea_idx] = sea_units_0[CARRIERS];
    enemy_units_count[air_idx] = 0;
    enemy_destroyers_total[sea_idx] = 0;
    enemy_blockade_total[sea_idx] = 0;
    for (int player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      uint8_t* sea_units = total_player_sea_unit_types[player_idx][sea_idx];
      if (!is_allied_0[player_idx]) {
        enemy_units_count[air_idx] += total_player_sea_units[player_idx][sea_idx];
        enemy_destroyers_total[sea_idx] += sea_units[DESTROYERS];
        enemy_blockade_total[sea_idx] += sea_units[DESTROYERS] + sea_units[CARRIERS] +
                                         sea_units[CRUISERS] + sea_units[BATTLESHIPS] +
                                         sea_units[BS_DAMAGED];
      } else {
        allied_carriers[sea_idx] += sea_units[CARRIERS];
      }
    }
    transports_with_large_cargo_space[sea_idx] = sea_units_0[TRANS_EMPTY] + sea_units_0[TRANS_1I];
    transports_with_small_cargo_space[sea_idx] = sea_units_0[TRANS_EMPTY] + sea_units_0[TRANS_1I] +
                                                 sea_units_0[TRANS_1A] + sea_units_0[TRANS_1T];
  }
  for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    for (int dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
      int nextSeaMovement = sea_path1[src_sea][dst_sea];
      int nextSeaMovementAlt = sea_path_alt1[src_sea][dst_sea];
      is_sea_path_blocked[src_sea][dst_sea] =
          enemy_blockade_total[nextSeaMovement] > 0 && enemy_blockade_total[nextSeaMovementAlt] > 0;
      is_sub_path_blocked[src_sea][dst_sea] = enemy_destroyers_total[nextSeaMovement] > 0 &&
                                              enemy_destroyers_total[nextSeaMovementAlt] > 0;
    }
  }
}
void debug_checks() {
#ifdef DEBUG
  status_message_id++;
  printf("Status message id: %d\n", status_message_id);
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    for (int unit_idx = 0; unit_idx < LAND_UNIT_TYPES_COUNT; unit_idx++) {
      int temp_unit_type_total = 0;
      for (int cur_unit_state = 0; cur_unit_state < STATES_MOVE_LAND[unit_idx]; cur_unit_state++) {
        temp_unit_type_total += land_units_state[land_idx][unit_idx][cur_unit_state];
      }
      if (temp_unit_type_total != current_player_land_unit_types[land_idx][unit_idx]) {
        printf("temp_unit_type_total %d != current_player_land_unit_types[land_idx][unit_idx] %d",
               temp_unit_type_total, current_player_land_unit_types[land_idx][unit_idx]);
      }
    }
  }
  for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    for (int unit_idx = 0; unit_idx < SEA_UNIT_TYPES_COUNT; unit_idx++) {
      int temp_unit_type_total = 0;
      for (int cur_unit_state = 0; cur_unit_state < STATES_MOVE_SEA[unit_idx]; cur_unit_state++) {
        temp_unit_type_total += sea_units_state[sea_idx][unit_idx][cur_unit_state];
      }
      if (temp_unit_type_total != current_player_sea_unit_types[sea_idx][unit_idx]) {
        printf("temp_unit_type_total %d != current_player_sea_unit_types[sea_idx][unit_idx] %d",
               temp_unit_type_total, current_player_sea_unit_types[sea_idx][unit_idx]);
      }
    }
  }
  for (int player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
      int total = 0;
      int total_data = 0;
      for (int unit_idx = 0; unit_idx < LAND_UNIT_TYPES_COUNT; unit_idx++) {
        if (player_idx == 0 || unit_idx < LAND_UNIT_TYPES_COUNT)
          total += total_player_land_unit_types[player_idx][land_idx][unit_idx];
        if (player_idx == 0) {
          total_data += current_player_land_unit_types[land_idx][unit_idx];
        } else {
          if (unit_idx < LAND_UNIT_TYPES_COUNT)
            total_data += data.other_land_units[player_idx - 1][land_idx][unit_idx];
        }
      }
      if (total != total_player_land_units[player_idx][land_idx] || total != total_data) {
        printf("total %d != total_player_land_units[player_idx][land_idx] %d != total_data %d",
               total, total_player_land_units[player_idx][land_idx], total_data);
      }

      if (total_player_land_units[player_idx][land_idx] < 0 ||
          total_player_land_units[player_idx][land_idx] > 240) {
        printf("units_land_player_total[player_idx][land_idx] < 0");
      }
      if (enemy_units_count[land_idx] < 0 || enemy_units_count[land_idx] > 240) {
        printf("enemy_units_count[land_idx] < 0");
      }
      total = 0;
      for (int unit_idx = 0; unit_idx < LAND_UNIT_TYPES_COUNT; unit_idx++) {
        if (total_player_land_unit_types[player_idx][land_idx][unit_idx] < 0 ||
            total_player_land_unit_types[player_idx][land_idx][unit_idx] > 240) {
          printf("total_player_land_unit_types[player_idx][land_idx][unit_idx] < 0");
        }
        total += total_player_land_unit_types[player_idx][land_idx][unit_idx];
      }
      if (total != total_player_land_units[player_idx][land_idx]) {
        printf("total %d != total_player_land_units[player_idx][land_idx] %d", total,
               total_player_land_units[player_idx][land_idx]);
      }
      int enemy_total = 0;
      for (uint8_t enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
        uint8_t enemy_player_idx = enemies_0[enemy_idx];
        enemy_total += total_player_land_units[enemy_player_idx][land_idx];
      }
      if (enemy_total != enemy_units_count[land_idx]) {
        printf("enemy_total %d != enemy_units_count[land_idx] %d", enemy_total,
               enemy_units_count[land_idx]);
      }
    }
    for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
      if (total_player_sea_units[player_idx][sea_idx] < 0 ||
          total_player_sea_units[player_idx][sea_idx] > 240) {
        printf("units_sea_player_total[player_idx][sea_idx] < 0");
      }
      for (int unit_idx = 0; unit_idx < SEA_UNIT_TYPES_COUNT; unit_idx++) {
        if (total_player_sea_unit_types[player_idx][sea_idx][unit_idx] < 0 ||
            total_player_sea_unit_types[player_idx][sea_idx][unit_idx] > 240) {
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
          total += total_player_sea_unit_types[player_idx][sea_idx][unit_idx];
        if (player_idx == 0) {
          total_data += current_player_sea_unit_types[sea_idx][unit_idx];
        } else {
          if (unit_idx < SEA_UNIT_TYPES_COUNT - 1)
            total_data += data.other_sea_units[player_idx - 1][sea_idx][unit_idx];
        }
      }
      if (total != total_player_sea_units[player_idx][sea_idx] || total != total_data) {
        printf("total %d != total_player_sea_units[player_idx][sea_idx] %d != total_data %d", total,
               total_player_sea_units[player_idx][sea_idx], total_data);
      }
      int enemy_total = 0;
      for (uint8_t enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
        uint8_t enemy_player_idx = enemies_0[enemy_idx];
        enemy_total += total_player_sea_units[enemy_player_idx][sea_idx];
      }
      if (enemy_total != enemy_units_count[sea_idx + LANDS_COUNT]) {
        printf("enemy_total %d != enemy_units_count[sea_idx + LANDS_COUNT] %d", enemy_total,
               enemy_units_count[sea_idx + LANDS_COUNT]);
      }
    }
  }
#endif
}

void setPrintableStatus() {
  debug_checks();
  char threeCharStr[4];
  // set printableGameStatus to ""
  //-exec watch total_player_sea_units[0][0] if total_player_sea_units[0][0] > 240
  //-exec watch total_player_sea_units[0][0] if total_player_sea_units[0][0] < 0
  printableGameStatus[0] = '\0';

  strcat(printableGameStatus, "---\n");
  setPrintableStatusLands();
  setPrintableStatusSeas();
  strcat(printableGameStatus, "\n");
  strcat(printableGameStatus, PLAYERS[data.player_index].color);
  strcat(printableGameStatus, PLAYERS[data.player_index].name);
  strcat(printableGameStatus, "\033[0m");
  strcat(printableGameStatus, ": ");
  sprintf(threeCharStr, "%d", data.money[0]);
  strcat(printableGameStatus, threeCharStr);
  strcat(printableGameStatus, " IPC\n");
}
void setPrintableStatusLands() {
  char threeCharStr[4];
  char paddedStr[32];
  char* my_color = PLAYERS[data.player_index].color;
  char* my_name = PLAYERS[data.player_index].name;
  int player_index = data.player_index;
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    //    LandState land_state = gameData.land_state[i];
    int land_owner = (*owner_idx[land_idx] + player_index) % PLAYERS_COUNT;
    strcat(printableGameStatus, PLAYERS[land_owner].color);
    sprintf(threeCharStr, "%d ", land_idx);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, LANDS[land_idx].name);
    strcat(printableGameStatus, ": ");
    strcat(printableGameStatus, PLAYERS[land_owner].name);
    strcat(printableGameStatus, " ");
    sprintf(threeCharStr, "%d", data.builds_left[land_idx]);
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
    if (data.flagged_for_combat[land_idx]) {
      strcat(printableGameStatus, "true\n");
    } else {
      strcat(printableGameStatus, "false\n");
    }
    int grand_total = 0;
    for (uint8_t player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
      grand_total += total_player_land_units[player_idx][land_idx];
    }
    if (grand_total == 0) {
      strcat(printableGameStatus, "\033[0m");
      continue;
    }
    strcat(printableGameStatus, "                 |Tot| 0| 1| 2| 3| 4| 5| 6|\n");
    if (total_player_land_units[0][land_idx] > 0) {
      strcat(printableGameStatus, my_color);
      uint8_t* other_land_units_here = current_player_land_unit_types[land_idx];
      for (int land_unit_idx = 0; land_unit_idx < LAND_UNIT_TYPES_COUNT; land_unit_idx++) {
        uint8_t unit_count = other_land_units_here[land_unit_idx];
        if (unit_count > 0) {
          strcat(printableGameStatus, my_name);
          strcat(printableGameStatus, " ");
          sprintf(paddedStr, "%-14s", NAMES_UNIT_LAND[land_unit_idx]);
          strcat(printableGameStatus, paddedStr);
          sprintf(threeCharStr, "%3d", unit_count);
          strcat(printableGameStatus, threeCharStr);
          uint8_t* units_here = land_units_state[land_idx][land_unit_idx];
          for (int cur_state = 0; cur_state < STATES_MOVE_LAND[land_unit_idx]; cur_state++) {
            sprintf(threeCharStr, "%3d", units_here[cur_state]);
            strcat(printableGameStatus, threeCharStr);
          }
          strcat(printableGameStatus, "\n");
        }
      }
    }
    strcat(printableGameStatus, "\033[0m");
    for (int player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      if (total_player_land_units[player_idx][land_idx] == 0)
        continue;
      int current_player_idx = (player_index + player_idx) % PLAYERS_COUNT;
      strcat(printableGameStatus, PLAYERS[current_player_idx].color);
      uint8_t* other_land_units_here = total_player_land_unit_types[player_idx][land_idx];
#ifdef DEBUG
      if (*other_land_units_here == 255) {
        printf("ERROR: other_land_units_here == 255\n");
      }
#endif
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
      grand_total += total_player_sea_units[player_idx][sea_index];
    }
    if (grand_total == 0) {
      continue;
    }
    UnitsSea units_sea = data.units_sea[sea_index];
    sprintf(threeCharStr, "%d ", LANDS_COUNT + sea_index);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, SEAS[sea_index].name);
    strcat(printableGameStatus, " Combat:");
    if (data.flagged_for_combat[sea_index + LANDS_COUNT]) {
      strcat(printableGameStatus, "true\n");
    } else {
      strcat(printableGameStatus, "false\n");
    }
    strcat(printableGameStatus, "                 |Tot| 0| 1| 2| 3| 4| 5| 6|\n");
    strcat(printableGameStatus, PLAYERS[data.player_index].color);
    if (total_player_sea_units[0][sea_index] > 0) {
      for (int sea_unit_idx = 0; sea_unit_idx < SEA_UNIT_TYPES_COUNT; sea_unit_idx++) {
        uint8_t unit_count = current_player_sea_unit_types[sea_index][sea_unit_idx];
        if (unit_count > 0) {
          strcat(printableGameStatus, PLAYERS[data.player_index].name);
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
      if (total_player_sea_units[player_idx][sea_index] == 0)
        continue;
      strcat(printableGameStatus, PLAYERS[(data.player_index + player_idx) % PLAYERS_COUNT].color);
      for (int sea_unit_idx = 0; sea_unit_idx < SEA_UNIT_TYPES_COUNT - 1; sea_unit_idx++) {
        uint8_t unit_count = total_player_sea_unit_types[player_idx][sea_index][sea_unit_idx];
        if (unit_count > 0) {
          strcat(printableGameStatus,
                 PLAYERS[(data.player_index + player_idx) % PLAYERS_COUNT].name);
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
uint8_t getUserInput(uint8_t* valid_moves, int valid_moves_count) {
  char buffer[4]; // Buffer to hold input string (3 digits + null terminator)
  int user_input;

  while (true) {
    // 0-valid_moves_count
    // printf("Enter a number between 0 and 255: ");
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
      if (sscanf(buffer, "%d", &user_input) == 1 && user_input >= 0) {
        for (int i = 0; i < valid_moves_count; i++) {
          if (user_input == valid_moves[i]) {
            return (uint8_t)user_input;
          }
        }
        return valid_moves[0];
      }
    }
  }
}
uint8_t getAIInput(uint8_t* valid_moves, int valid_moves_count) {
  return valid_moves[RANDOM_NUMBERS[random_number_index++] % valid_moves_count];
}

void add_valid_land_move_if_history_allows_1(uint8_t* valid_moves, uint8_t* valid_moves_count,
                                             uint8_t dst_air, uint8_t src_land) {
  // get a list of all of the source territories that moved a unit into a territory that I can also
  // reach for each source territory, get a list of possible territory moves that were purposefully
  // skipped these skipped territories are also invalid moves
  bool has_checked_territory[AIRS_COUNT] = {0};
  int shared_dst;
  int src_air;
  int hist_source_territories_count_shared_dst;
  int land_to_land_count = LAND_TO_LAND_COUNT[src_land];
  int* hist_source_territories_shared_dst;
  uint8_t* land_to_land_conn = LAND_TO_LAND_CONN[src_land];
  for (int land_idx = 0; land_idx < land_to_land_count; land_idx++) {
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
  int land_to_sea_count = LAND_TO_SEA_COUNT[src_land];
  for (int sea_idx = 0; sea_idx < land_to_sea_count; sea_idx++) {
    shared_dst = LAND_TO_SEA_CONN[src_land][sea_idx] + LANDS_COUNT;
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
  valid_moves[*valid_moves_count] = dst_air;
  (*valid_moves_count)++;
}

void add_valid_land_move_if_history_allows_2(uint8_t* valid_moves, uint8_t* valid_moves_count,
                                             uint8_t dst_air, uint8_t src_land) {
  // get a list of all of the source territories that moved a unit into a territory that I can also
  // reach for each source territory, get a list of possible territory moves that were purposefully
  // skipped these skipped territories are also invalid moves
  bool has_checked_territory[AIRS_COUNT] = {0};
  int shared_dst;
  int src_air;
  int hist_source_territories_count_shared_dst;
  int lands_within_2_moves_count = LANDS_WITHIN_2_MOVES_COUNT[src_land];
  uint8_t* lands_within_2_moves = LANDS_WITHIN_2_MOVES[src_land];
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
  valid_moves[*valid_moves_count] = dst_air;
  (*valid_moves_count)++;
}

void add_valid_sea_move_if_history_allows_1(uint8_t* valid_moves, uint8_t* valid_moves_count,
                                            uint8_t dst_sea, uint8_t src_sea) {
  // get a list of all of the source territories that moved a unit into a territory that I can also
  // reach for each source territory, get a list of possible territory moves that were purposefully
  // skipped these skipped territories are also invalid moves
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
  valid_moves[*valid_moves_count] = dst_sea + LANDS_COUNT;
  (*valid_moves_count)++;
}

void add_valid_sea_move_if_history_allows_2(uint8_t* valid_moves, uint8_t* valid_moves_count,
                                            uint8_t dst_sea, uint8_t src_sea) {
  // get a list of all of the source territories that moved a unit into a territory that I can also
  // reach for each source territory, get a list of possible territory moves that were purposefully
  // skipped these skipped territories are also invalid moves
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
  valid_moves[*valid_moves_count] = dst_sea + LANDS_COUNT;
  (*valid_moves_count)++;
}

void add_valid_sub_move_if_history_allows_2(uint8_t* valid_moves, uint8_t* valid_moves_count,
                                            uint8_t dst_sea, uint8_t src_sea) {
  // get a list of all of the source territories that moved a unit into a territory that I can also
  // reach for each source territory, get a list of possible territory moves that were purposefully
  // skipped these skipped territories are also invalid moves
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
  valid_moves[*valid_moves_count] = dst_sea + LANDS_COUNT;
  (*valid_moves_count)++;
}

void add_valid_air_move_if_history_allows_X(uint8_t* valid_moves, uint8_t* valid_moves_count,
                                            uint8_t dst_air, uint8_t src_air, uint8_t moves) {
  // get a list of all of the source territories that moved a unit into a territory that I can also
  // reach for each source territory, get a list of possible territory moves that were purposefully
  // skipped these skipped territories are also invalid moves
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
  valid_moves[*valid_moves_count] = dst_air;
  (*valid_moves_count)++;
}

void clear_move_history() {
  memset(hist_skipped_airs, 0, sizeof(hist_skipped_airs));
  memset(hist_source_territories_count, 0, sizeof(hist_source_territories_count));
}

uint8_t get_user_purchase_input(uint8_t src_air, uint8_t* valid_purchases,
                                uint8_t valid_purchases_count) {
  uint8_t user_input;
  if (valid_purchases_count == 1) {
    return valid_purchases[0];
  }
  if (PLAYERS[data.player_index].is_human) {
    char stringBuffer[32];
    setPrintableStatus();
    strcat(printableGameStatus, "Purchasing at ");
    if (src_air < LANDS_COUNT) {
      strcat(printableGameStatus, LANDS[src_air].name);
      sprintf(stringBuffer, " %d=Finished ", valid_purchases[0]);
      strcat(printableGameStatus, stringBuffer);
      for (int i = 1; i < valid_purchases_count; i++) {
        sprintf(stringBuffer, "%d=%s ", valid_purchases[i], NAMES_UNIT_LAND[valid_purchases[i]]);
        strcat(printableGameStatus, stringBuffer);
      }
    } else {
      strcat(printableGameStatus, SEAS[src_air - LANDS_COUNT].name);
      sprintf(stringBuffer, " %d=Finished ", valid_purchases[0]);
      strcat(printableGameStatus, stringBuffer);
      for (int i = 1; i < valid_purchases_count; i++) {
        sprintf(stringBuffer, "%d=%s ", valid_purchases[i], NAMES_UNIT_SEA[valid_purchases[i]]);
        strcat(printableGameStatus, stringBuffer);
      }
    }
    printf("%s\n", printableGameStatus);
    return getUserInput(valid_purchases, valid_purchases_count);
  }
  return getAIInput(valid_purchases, valid_purchases_count);
}

uint8_t get_user_move_input(uint8_t unit_type, uint8_t src_air, uint8_t* valid_moves,
                            uint8_t valid_moves_count) {
  uint8_t user_input;
  if (valid_moves_count == 1) {
    return valid_moves[0];
  }
  if (PLAYERS[data.player_index].is_human) {
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
    for (int i = 0; i < valid_moves_count; i++) {
      char threeCharStr[4];
      sprintf(threeCharStr, "%d ", valid_moves[i]);
      strcat(printableGameStatus, threeCharStr);
    }
    printf("%s\n", printableGameStatus);
    return getUserInput(valid_moves, valid_moves_count);
  }
  return getAIInput(valid_moves, valid_moves_count);
}

void update_move_history(uint8_t user_input, uint8_t src_air, uint8_t* valid_moves,
                         uint8_t* valid_moves_count) {
  // hist_source_territories is a list of terrirtories that moved a unit into a specific territory
  // format is:
  //  hist_source_territories[dst_air][0] = src_air
  //  hist_source_territories[dst_air][1] = src_air2
  if (hist_source_territories[user_input][hist_source_territories_count[user_input]] == src_air)
    return;
  hist_source_territories[user_input][hist_source_territories_count[user_input]] = src_air;
  hist_source_territories_count[user_input]++;
  for (int move_index = *valid_moves_count - 1; move_index >= 0; move_index--) {
    uint8_t move = valid_moves[move_index];
    if (move == user_input) {
      break;
    } else {
      hist_skipped_airs[src_air][move] = true;
      *valid_moves_count -= 1;
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
         trans_state >= states_unloading; trans_state--) {
      if (units_sea_ptr_dst_sea_trans_type[trans_state] > 0) {
        uint8_t new_trans_type = load_unit_type[trans_type];
        units_sea_ptr_dst_sea[new_trans_type][trans_state]++;
        units_sea_ptr_dst_sea[trans_type][trans_state]--;
        current_player_sea_unit_types[dst_sea][new_trans_type]++;
        current_player_sea_unit_types[dst_sea][trans_type]--;
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
        debug_checks();
        return true;
      }
    }
  }
  printf("Error: Failed to load transport\n");
  return false;
}

void add_valid_land_moves(uint8_t* valid_moves, uint8_t* valid_moves_count, uint8_t src_land,
                          uint8_t moves_remaining, uint8_t unit_type) {
  if (moves_remaining == 2) {
    // check for moving from land to land (two moves away)
    uint8_t lands_within_2_moves_count = LANDS_WITHIN_2_MOVES_COUNT[src_land];
    uint8_t* lands_within_2_moves = LANDS_WITHIN_2_MOVES[src_land];
    uint8_t* land_dist = LAND_DIST[src_land];
    bool* is_land_path_blocked_src_land = is_land_path_blocked[src_land];
    for (int land_idx = 0; land_idx < lands_within_2_moves_count; land_idx++) {
      uint8_t dst_land = lands_within_2_moves[land_idx];
      if (land_dist[dst_land] == 1) {
        add_valid_land_move_if_history_allows_2(valid_moves, valid_moves_count, dst_land, src_land);
        continue;
      }
      if (is_land_path_blocked_src_land[dst_land]) {
        continue;
      }
      add_valid_land_move_if_history_allows_2(valid_moves, valid_moves_count, dst_land, src_land);
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
        add_valid_land_move_if_history_allows_2(valid_moves, valid_moves_count, dst_air, src_land);
        continue;
      }
      if (is_land_path_blocked_src_land[dst_air]) {
        continue;
      }
      add_valid_land_move_if_history_allows_2(valid_moves, valid_moves_count, dst_air, src_land);
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
      add_valid_land_move_if_history_allows_1(valid_moves, valid_moves_count, dst_land, src_land);
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
      add_valid_land_move_if_history_allows_1(valid_moves, valid_moves_count, dst_sea + LANDS_COUNT,
                                              src_land);
    }
  }
}

void add_valid_sea_moves(uint8_t* valid_moves, uint8_t* valid_moves_count, uint8_t src_sea,
                         uint8_t moves_remaining) {
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
      add_valid_sea_move_if_history_allows_2(valid_moves, valid_moves_count, dst_sea, src_sea);
    }
  } else {
    // check for moving from sea to sea (one move away)
    uint8_t seas_within_1_move_count = SEAS_WITHIN_1_MOVE_COUNT[canal_state][src_sea];
    uint8_t* seas_within_1_move =
        SEAS_WITHIN_1_MOVE[canal_state][src_sea]; // TODO optimize canal_state
    for (uint8_t sea_idx = 0; sea_idx < seas_within_1_move_count; sea_idx++) {
      uint8_t dst_sea = seas_within_1_move[sea_idx];
      add_valid_sea_move_if_history_allows_1(valid_moves, valid_moves_count, dst_sea, src_sea);
    }
  }
}

void add_valid_sub_moves(uint8_t* valid_moves, uint8_t* valid_moves_count, uint8_t src_sea,
                         uint8_t moves_remaining) {
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
      add_valid_sub_move_if_history_allows_2(valid_moves, valid_moves_count, dst_sea, src_sea);
    }
  } else {
    // check for moving from sea to sea (one move away)
    uint8_t seas_within_1_move_count = SEAS_WITHIN_1_MOVE_COUNT[canal_state][src_sea];
    uint8_t* seas_within_1_move =
        SEAS_WITHIN_1_MOVE[canal_state][src_sea]; // TODO optimize canal_state
    for (uint8_t sea_idx = 0; sea_idx < seas_within_1_move_count; sea_idx++) {
      uint8_t dst_sea = seas_within_1_move[sea_idx];
      add_valid_sea_move_if_history_allows_1(valid_moves, valid_moves_count, dst_sea, src_sea);
    }
  }
}

void stage_transport_units() {
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
      uint8_t valid_moves[AIRS_COUNT];
      uint8_t src_air = src_sea + LANDS_COUNT;
      valid_moves[0] = src_air;
      uint8_t valid_moves_count = 1;
      add_valid_sea_moves(valid_moves, &valid_moves_count, src_sea, 2);
      uint8_t* units_sea_ptr_src_sea_unit_type = sea_units_state[src_sea][unit_type];
      while (*total_ships > 0) {
        uint8_t dst_air = get_user_move_input(unit_type, src_air, valid_moves, valid_moves_count);
#ifdef DEBUG
        printf("stage_transport_units: unit_type=%d src_air=%d dst_air=%d\n", unit_type, src_air,
               dst_air);
        setPrintableStatus();
        printf("%s\n", printableGameStatus);
#endif
        update_move_history(dst_air, src_sea, valid_moves, &valid_moves_count);
        if (src_air == dst_air) {
          units_sea_ptr_src_sea_unit_type[done_staging] += *total_ships;
          *total_ships = 0;
          continue;
        }
        uint8_t dst_sea = dst_air - LANDS_COUNT;
        uint8_t sea_distance = sea_dist[src_sea][dst_air];
        if (enemy_blockade_total[dst_sea] > 0) {
          DEBUG_PRINT("Enemy blockade detected, staging transports\n");
          data.flagged_for_combat[dst_air] = true;
          sea_distance = MAX_MOVE_SEA[unit_type];
          break;
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
}
void move_fighter_units() {
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
    canFighterLandHere[land_idx] = (is_allied_0[land_owner] && !data.flagged_for_combat[land_idx]);
    // check for possiblity to build carrier under fighter
    if (land_owner == data.player_index && *factory_max[land_idx] > 0) {
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
  for (int src_air = 0; src_air < AIRS_COUNT; src_air++) {
    uint8_t* total_fighters = &air_units_state[src_air][FIGHTERS][FIGHTER_MOVES_MAX];
    if (*total_fighters == 0) {
      continue;
    }
    uint8_t valid_moves[AIRS_COUNT];
    valid_moves[0] = src_air;
    uint8_t valid_moves_count = 1;
    add_valid_fighter_moves(valid_moves, &valid_moves_count, src_air, FIGHTER_MOVES_MAX);
    while (*total_fighters > 0) {
      uint8_t dst_air = get_user_move_input(FIGHTERS, src_air, valid_moves, valid_moves_count);
#ifdef DEBUG
      setPrintableStatus();
      printf("%s\n", printableGameStatus);
      printf("DEBUG: player: %s moving fighters %d, src_air: %d, dst_air: %d\n",
             PLAYERS[data.player_index].name, FIGHTERS, src_air, dst_air);
#endif
      update_move_history(dst_air, src_air, valid_moves, &valid_moves_count);
      if (src_air == dst_air) {
        air_units_state[src_air][FIGHTERS][0] += *total_fighters;
        *total_fighters = 0;
        continue;
      }
      uint8_t airDistance = AIR_DIST[src_air][dst_air];
      if (dst_air < LANDS_COUNT) {
        if (!is_allied_0[*owner_idx[dst_air]]) {
          DEBUG_PRINT("Fighter moving to enemy territory. Automatically flagging for combat");
          data.flagged_for_combat[dst_air] =
              true; // assuming enemy units are present based on valid moves
        } else {
          airDistance = 4; // use up all moves if this is a friendly rebase
        }
      } else {
        DEBUG_PRINT("Fighter moving to sea. Possibly flagging for combat");
        data.flagged_for_combat[dst_air] = enemy_units_count[dst_air] > 0;
      }
      if (dst_air < LANDS_COUNT) {
        land_units_state[dst_air][FIGHTERS][FIGHTER_MOVES_MAX - airDistance]++;
        current_player_land_unit_types[dst_air][FIGHTERS]++;
        total_player_land_units[0][dst_air]++;
      } else {
        uint8_t dst_sea = dst_air - LANDS_COUNT;
        sea_units_state[dst_sea][FIGHTERS][FIGHTER_MOVES_MAX - airDistance]++;
        current_player_sea_unit_types[dst_sea][FIGHTERS]++;
        total_player_sea_units[0][dst_sea]++;
      }
      if (src_air < LANDS_COUNT) {
        current_player_land_unit_types[src_air][FIGHTERS]--;
        total_player_land_units[0][src_air]--;
      } else {
        uint8_t src_sea = src_air - LANDS_COUNT;
        current_player_sea_unit_types[src_sea][FIGHTERS]--;
        total_player_sea_units[0][src_sea]--;
      }
      *total_fighters -= 1;
    }
  }
}
void move_bomber_units() {
  // check if any bombers have full moves remaining
  clear_move_history();
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    // is allied owned and not recently conquered?
    canBomberLandHere[land_idx] =
        (is_allied_0[*owner_idx[land_idx]] && !data.flagged_for_combat[land_idx]);
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
    uint8_t* total_bombers = &land_units_state[src_land][BOMBERS_LAND_AIR][BOMBER_MOVES_MAX];
    if (*total_bombers == 0) {
      continue;
    }
    uint8_t valid_moves[AIRS_COUNT];
    valid_moves[0] = src_land;
    uint8_t valid_moves_count = 1;
    add_valid_bomber_moves(valid_moves, &valid_moves_count, src_land, BOMBER_MOVES_MAX);
    while (*total_bombers > 0) {
      uint8_t dst_air =
          get_user_move_input(BOMBERS_LAND_AIR, src_land, valid_moves, valid_moves_count);
#ifdef DEBUG
      printf("DEBUG: player: %s bombers fighters %d, src_air: %d, dst_air: %d\n",
             PLAYERS[data.player_index].name, BOMBERS_LAND_AIR, src_land, dst_air);
#endif
      update_move_history(dst_air, src_land, valid_moves, &valid_moves_count);
      if (src_land == dst_air) {
        land_units_state[src_land][BOMBERS_LAND_AIR][0] += *total_bombers;
        *total_bombers = 0;
        continue;
      }
      if (dst_air < LANDS_COUNT) {
        if (!is_allied_0[*owner_idx[dst_air]]) {
          DEBUG_PRINT("Bomber moving to enemy territory. Automatically flagging for combat");
          data.flagged_for_combat[dst_air] =
              true; // assuming enemy units are present based on valid moves
        }
      } else {
        DEBUG_PRINT("Bomber moving to sea. Possibly flagging for combat");
        uint8_t dst_sea = dst_air - LANDS_COUNT;
        data.flagged_for_combat[dst_air] = enemy_units_count[dst_air] > 0;
      }
      uint8_t airDistance = AIR_DIST[src_land][dst_air];
      if (dst_air < LANDS_COUNT) {
        land_units_state[dst_air][BOMBERS_LAND_AIR][BOMBER_MOVES_MAX - airDistance]++;
        total_player_land_unit_types[0][dst_air][BOMBERS_LAND_AIR]++;
        total_player_land_units[0][dst_air]++;
      } else {
        uint8_t dst_sea = dst_air - LANDS_COUNT;
        sea_units_state[dst_sea][BOMBERS_SEA][BOMBER_MOVES_MAX - 1 - airDistance]++;
        current_player_sea_unit_types[dst_sea][BOMBERS_SEA]++;
        total_player_sea_units[0][dst_sea]++;
      }
      current_player_land_unit_types[src_land][BOMBERS_LAND_AIR]--;
      total_player_land_units[0][src_land]--;
      *total_bombers -= 1;
    }
  }
}

void conquer_land(uint8_t dst_land) {
  uint8_t old_owner_id = *owner_idx[dst_land];
  if (PLAYERS[(data.player_index + old_owner_id) % PLAYERS_COUNT].capital_territory_index ==
      dst_land) {
    data.money[0] += data.money[old_owner_id];
    data.money[old_owner_id] = 0;
  }
  income_per_turn[old_owner_id] -= LAND_VALUE[dst_land];
  uint8_t new_owner_id = 0;
  uint8_t orig_owner_id =
      (LANDS[dst_land].original_owner_index + PLAYERS_COUNT - data.player_index) % PLAYERS_COUNT;
  if (is_allied_0[orig_owner_id]) {
    new_owner_id = orig_owner_id;
  }
  *owner_idx[dst_land] = new_owner_id;
  income_per_turn[new_owner_id] += LAND_VALUE[dst_land];
}

void move_land_unit_type(uint8_t unit_type) {
  clear_move_history();
  for (int src_land = 0; src_land < LANDS_COUNT; src_land++) {
    for (int moves_remaining = MAX_MOVE_LAND[unit_type]; moves_remaining > 0; moves_remaining--) {
      uint8_t* total_units = &land_units_state[src_land][unit_type][moves_remaining];
      if (*total_units == 0) {
        continue;
      }
      uint8_t valid_moves[AIRS_COUNT];
      valid_moves[0] = src_land;
      uint8_t valid_moves_count = 1;
      add_valid_land_moves(valid_moves, &valid_moves_count, src_land, moves_remaining, unit_type);
      while (*total_units > 0) {
        uint8_t dst_air = get_user_move_input(unit_type, src_land, valid_moves, valid_moves_count);
#ifdef DEBUG
        setPrintableStatus();
        printf("%s\n", printableGameStatus);
        printf("DEBUG: player: %s moving land unit %d, src_air: %d, dst_air: %d enemy count: %d\n",
               PLAYERS[data.player_index].name, unit_type, src_land, dst_air,
               enemy_units_count[dst_air]);
#endif
        update_move_history(dst_air, src_land, valid_moves, &valid_moves_count);
        if (src_land == dst_air) {
          land_units_state[src_land][unit_type][0] += *total_units;
          *total_units = 0;
          continue;
        }
        if (dst_air >= LANDS_COUNT) {
          load_transport(unit_type, src_land, dst_air - LANDS_COUNT, moves_remaining);
          // recalculate valid moves since transport cargo has changed
          valid_moves_count = 1;
          add_valid_land_moves(valid_moves, &valid_moves_count, src_land, moves_remaining,
                               unit_type);
          continue;
        }
        data.flagged_for_combat[dst_air] = enemy_units_count[dst_air] > 0;
        // if the destination is not blitzable, then end unit turn
        uint8_t landDistance;
        if (is_allied_0[*owner_idx[dst_air]] || enemy_units_count[dst_air] > 0) {
          landDistance = MAX_MOVE_LAND[unit_type];
        } else {
          DEBUG_PRINT("Conquering land");
          landDistance = LAND_DIST[src_land][dst_air];
          conquer_land(dst_air);
          data.flagged_for_combat[dst_air] = true;
        }
        land_units_state[dst_air][unit_type][moves_remaining - landDistance]++;
        current_player_land_unit_types[dst_air][unit_type]++;
        total_player_land_units[0][dst_air]++;
        current_player_land_unit_types[src_land][unit_type]--;
        total_player_land_units[0][src_land]--;
        *total_units -= 1;
      }
    }
  }
}

void move_transport_units() {
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
        uint8_t valid_moves[AIRS_COUNT];
        uint8_t src_air = src_sea + LANDS_COUNT;
        valid_moves[0] = src_air;
        uint8_t valid_moves_count = 1;
        add_valid_sea_moves(valid_moves, &valid_moves_count, src_sea, moves_remaining);
        while (*total_ships > 0) {
          uint8_t dst_air = get_user_move_input(unit_type, src_air, valid_moves, valid_moves_count);
#ifdef DEBUG
          printf("DEBUG: moving transport units unit_type: %d, src_air: %d, dst_air: %d\n",
                 unit_type, src_air, dst_air);
#endif
          update_move_history(dst_air, src_air, valid_moves, &valid_moves_count);
          uint8_t dst_sea = dst_air - LANDS_COUNT;
          if (enemy_blockade_total[dst_sea] > 0) {
            DEBUG_PRINT("Enemy units detected, flagging for combat");
            data.flagged_for_combat[dst_sea] = true;
            break;
          }
          if (src_air == dst_air) {
            sea_units_state[src_sea][unit_type][done_moving] += *total_ships;
            *total_ships = 0;
            continue;
          }
          sea_units_state[dst_sea][unit_type][done_moving]++;
          current_player_sea_unit_types[dst_sea][unit_type]++;
          total_player_sea_units[0][dst_sea]++;
          *total_ships -= 1;
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
}
void move_subs() {
  clear_move_history();
  for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    uint8_t* total_subs = &sea_units_state[src_sea][SUBMARINES][SUB_UNMOVED];
    if (*total_subs == 0)
      continue;
    uint8_t valid_moves[AIRS_COUNT];
    uint8_t src_air = src_sea + LANDS_COUNT;
    valid_moves[0] = src_air;
    uint8_t valid_moves_count = 1;
    add_valid_sub_moves(valid_moves, &valid_moves_count, src_sea, SUB_MOVES_MAX);
    while (*total_subs > 0) {
      uint8_t dst_air = get_user_move_input(SUBMARINES, src_air, valid_moves, valid_moves_count);
#ifdef DEBUG
      setPrintableStatus();
      printf("%s\n", printableGameStatus);
      printf("DEBUG: moving sub units unit_type: %d, src_air: %d, dst_air: %d\n", SUBMARINES,
             src_air, dst_air);
#endif
      update_move_history(dst_air, src_air, valid_moves, &valid_moves_count);
      uint8_t dst_sea = dst_air - LANDS_COUNT;
      if (enemy_units_count[dst_sea] > 0) {
        DEBUG_PRINT("Submarine moving to where enemy units are present, flagging for combat");
        data.flagged_for_combat[dst_sea] = true;
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
}

void move_destroyers_battleships() {
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
      uint8_t valid_moves[AIRS_COUNT];
      valid_moves[0] = src_air;
      uint8_t valid_moves_count = 1;
      add_valid_sea_moves(valid_moves, &valid_moves_count, src_sea, moves_remaining);
      while (*total_ships > 0) {
        uint8_t dst_air = get_user_move_input(unit_type, src_air, valid_moves, valid_moves_count);
#ifdef DEBUG
        debug_checks();
        printf("DEBUG: moving ships units unit_type: %d, src_air: %d, dst_air: %d\n", unit_type,
               src_air, dst_air);
        if (unit_type == DESTROYERS) {
          printf("DEBUG: DESTROYERS src_sea: %d, dst_air: %d\n", src_sea, dst_air);
        }
#endif
        update_move_history(dst_air, src_air, valid_moves, &valid_moves_count);
        if (enemy_units_count[dst_air] > 0) {
          DEBUG_PRINT("Moving large ships. Enemy units detected, flagging for combat");
          data.flagged_for_combat[dst_air] = true;
          // break;
        }
        if (src_air == dst_air) {
          sea_units_state[src_sea][unit_type][done_moving] += *total_ships;
          *total_ships = 0;
          continue;
        }
        uint8_t dst_sea = dst_air - LANDS_COUNT;
        sea_units_state[dst_sea][unit_type][done_moving]++;
        current_player_sea_unit_types[dst_sea][unit_type]++;
        total_player_sea_units[0][dst_sea]++;
        *total_ships -= 1;
        current_player_sea_unit_types[src_sea][unit_type]--;
        total_player_sea_units[0][src_sea]--;
        if (unit_type == CARRIERS)
          carry_allied_fighters(src_sea, dst_sea);
        debug_checks();
      }
    }
  }
}

void carry_allied_fighters(uint8_t src_sea, uint8_t dst_sea) {
#ifdef DEBUG
  printf("DEBUG: carry_allied_fighters: src_sea: %d, dst_sea: %d\n", src_sea, dst_sea);
  setPrintableStatus();
  printf("%s\n", printableGameStatus);
#endif
  int allied_fighters_moved = 0;
  for (int other_player_idx = 1; other_player_idx < PLAYERS_COUNT; other_player_idx++) {
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

void resolve_sea_battles() {
  for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    // check if battle is over (e.g. untargetable subs/air/trans or zero units)
    // target options:
    // 1. attacker has air, defender has std ships or air
    // 2. attacker has any ships, defender has any non-transports
    // 3. defender has air, attacker has std ships or air
    // 4. defender has any ships, attacker has any non-transports
    uint8_t src_air = src_sea + LANDS_COUNT;
    // if not flagged for combat, continue
    if (!data.flagged_for_combat[src_air]) {
      continue;
    }
    if (total_player_sea_units[0][src_sea] == 0) {
      continue;
    }
    setPrintableStatus();
    printf("%s\n", printableGameStatus);
#ifdef DEBUG
    printf("DEBUG: resolve_sea_battles: src_sea: %d\n", src_sea);
#endif
    // does enemy only have submerged subs?
    bool defender_submerged = current_player_sea_unit_types[src_sea][DESTROYERS] == 0;
    if (defender_submerged) {
      uint8_t total_enemy_subs = 0;
      for (uint8_t enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
        total_enemy_subs += total_player_sea_unit_types[enemies_0[enemy_idx]][src_sea][SUBMARINES];
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
      if (current_player_land_unit_types[2][FIGHTERS] == 255) {
        printf("DEBUG: units_land_ptr[0][FIGHTERS][0] == 255\n");
      }
      if (enemy_units_count[2] == -1) {
        printf("DEBUG: enemy_units_count[2] == -1\n");
      }
#endif
      uint8_t* units11 = &sea_units_state[src_sea][DESTROYERS][0];
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
          for (int enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
            int enemy_player_idx = enemies_0[enemy_idx];
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
        for (int enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
          int enemy_player_idx = enemies_0[enemy_idx];
          if (total_player_sea_units[enemy_player_idx][src_sea] -
                  total_player_sea_unit_types[enemy_player_idx][src_sea][FIGHTERS] >
              0) {
            targets_exist = true;
            break;
          }
        }
      } else if (current_player_sea_unit_types[src_sea][FIGHTERS] +
                     current_player_sea_unit_types[src_sea][BOMBERS_SEA] >
                 0) { // no ships, only air
        for (int enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
          int enemy_player_idx = enemies_0[enemy_idx];
          if (total_player_sea_units[enemy_player_idx][src_sea] -
                  total_player_sea_unit_types[enemy_player_idx][src_sea][SUBMARINES] >
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
        data.flagged_for_combat[src_air] = false;
        break;
      }
      // fire subs (defender always submerges if possible)
      int attacker_damage = sea_units_state[src_sea][SUBMARINES][0] * SUB_ATTACK;
      uint8_t attacker_hits =
          (attacker_damage / 6) +
          (RANDOM_NUMBERS[random_number_index++] % 6 < attacker_damage % 6 ? 1 : 0);

      int defender_damage;
      int defender_hits;
      if (!defender_submerged) {
        defender_damage = 0; // TODO skip if no subs
        for (int enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
          defender_damage += total_player_sea_unit_types[enemies_0[enemy_idx]][src_sea][SUBMARINES];
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
        attacker_damage +=
            current_player_sea_unit_types[src_sea][unit_type] * ATTACK_UNIT_SEA[unit_type];
      }
      attacker_damage +=
          current_player_sea_unit_types[src_sea][FIGHTERS] * ATTACK_UNIT_SEA[FIGHTERS];
      attacker_damage +=
          current_player_sea_unit_types[src_sea][BOMBERS_SEA] * ATTACK_UNIT_SEA[BOMBERS_SEA];
      attacker_hits = (attacker_damage / 6) +
                      (RANDOM_NUMBERS[random_number_index++] % 6 < attacker_damage % 6 ? 1 : 0);

      defender_damage = 0;
      for (uint8_t enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
        uint8_t* enemy_units = total_player_sea_unit_types[enemies_0[enemy_idx]][src_sea];
        for (uint8_t unit_type = 0; unit_type < BLOCKADE_UNIT_TYPES_COUNT; unit_type++) {
          defender_damage += enemy_units[unit_type] * DEFENSE_UNIT_SEA[unit_type];
        }
        defender_damage += enemy_units[FIGHTERS] * DEFENSE_UNIT_SEA[FIGHTERS];
      }
      defender_hits = (defender_damage / 6) +
                      (RANDOM_NUMBERS[random_number_index++] % 6 < defender_damage % 6 ? 1 : 0);
      // remove casualties
      if (defender_hits > 0)
        remove_sea_attackers(src_sea, defender_hits);
      if (attacker_hits > 0)
        remove_sea_defenders(src_sea, attacker_hits, defender_submerged);

      if (enemy_units_count[src_air] == 0 || total_player_sea_units[0][src_sea] == 0) {
        data.flagged_for_combat[src_air] = false;
        break;
      }

      // ask to retreat (0-255, any non valid retreat zone is considered a no)
      if (current_player_sea_unit_types[src_sea][FIGHTERS] +
              current_player_sea_unit_types[src_sea][BOMBERS_SEA] +
              current_player_sea_unit_types[src_sea][SUBMARINES] +
              current_player_sea_unit_types[src_sea][DESTROYERS] +
              current_player_sea_unit_types[src_sea][CARRIERS] +
              current_player_sea_unit_types[src_sea][CRUISERS] +
              current_player_sea_unit_types[src_sea][BATTLESHIPS] +
              current_player_sea_unit_types[src_sea][BS_DAMAGED] >
          0) {
        valid_retreats[0] = src_sea;
        retreat_count = 1;
      } else {
        retreat_count = 0;
      }
      uint8_t* sea_to_sea_conn = SEA_TO_SEA_CONN[src_sea];
      for (int sea_conn_idx = 0; sea_conn_idx < SEA_TO_SEA_COUNT[src_sea]; sea_conn_idx++) {
        uint8_t sea_dst = sea_to_sea_conn[sea_conn_idx];
        if (enemy_blockade_total[sea_dst] == 0)
          valid_retreats[retreat_count++] = sea_dst;
      }
      uint8_t retreat = ask_to_retreat();
      // if retreat, move units to retreat zone immediately and end battle
      if (sea_dist[src_sea][retreat] == 1 && !data.flagged_for_combat[retreat]) {
        sea_retreat(src_sea, retreat);
        break;
      }
      // loop
    }
  }
}

void sea_retreat(uint8_t src_sea, uint8_t retreat) {
#ifdef DEBUG
  debug_checks();
  printf("DEBUG: retreating to %d\n", retreat);
#endif
  for (uint8_t unit_type = TRANS_EMPTY; unit_type <= BS_DAMAGED; unit_type++) {
    sea_units_state[retreat][unit_type][0] += current_player_sea_unit_types[src_sea][unit_type];
    current_player_sea_unit_types[retreat][unit_type] +=
        current_player_sea_unit_types[src_sea][unit_type];
    total_player_sea_units[0][retreat] += current_player_sea_unit_types[src_sea][unit_type];
    total_player_sea_units[0][src_sea] -= current_player_sea_unit_types[src_sea][unit_type];
    sea_units_state[src_sea][unit_type][STATES_UNLOADING[unit_type]] = 0;
    current_player_sea_unit_types[src_sea][unit_type] = 0;
  }
  data.flagged_for_combat[src_sea + LANDS_COUNT] = false;
  debug_checks();
}

uint8_t ask_to_retreat() {
  if (PLAYERS[data.player_index].is_human) {
    setPrintableStatus();
    strcat(printableGameStatus, "To where do you want to retreat (255 for no)? ");
    printf("%s\n", printableGameStatus);
    return getUserInput(valid_retreats, AIRS_COUNT);
  }
  return getAIInput(valid_retreats, AIRS_COUNT);
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
void remove_land_attackers(uint8_t src_land, uint8_t hits) {
  uint8_t* total_units;
  for (uint8_t unit_idx; unit_idx < ATTACKER_LAND_UNIT_TYPES_COUNT_1; unit_idx++) {
    uint8_t unit_type = ORDER_OF_LAND_ATTACKERS_1[unit_idx];
    total_units = &land_units_state[src_land][unit_type][0];
    if (*total_units > 0) {
#ifdef DEBUG
      printf("DEBUG: remove_land_attackers: unit_type: %d src_land: %d, hits: %d\n", unit_type,
             src_land, hits);
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
  for (uint8_t unit_idx = 0; unit_idx < ATTACKER_LAND_UNIT_TYPES_COUNT_2; unit_idx++) {
    uint8_t unit_type = ORDER_OF_LAND_ATTACKERS_2[unit_idx];
    if (total_player_land_unit_types[0][src_land][unit_type] == 0)
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
void remove_sea_defenders(uint8_t src_sea, uint8_t hits, bool defender_submerged) {
#ifdef DEBUG
  printf("DEBUG: remove_sea_defenders: src_sea: %d, hits: %d\n", src_sea, hits);
#endif
  uint8_t src_air = src_sea + LANDS_COUNT;
  for (uint8_t enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
    uint8_t enemy_player_idx = enemies_0[enemy_idx];
    uint8_t* total_battleships =
        &total_player_sea_unit_types[enemy_player_idx][src_sea][BATTLESHIPS];
    uint8_t* total_bs_damaged = &total_player_sea_unit_types[enemy_player_idx][src_sea][BS_DAMAGED];
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
      uint8_t* total_units = &total_player_sea_unit_types[enemy_player_idx][src_sea][SUBMARINES];
      if (*total_units > 0) {
#ifdef DEBUG
        printf("DEBUG: remove_sea_defenders subs: src_sea: %d, hits: %d, total_units: %d\n",
               src_sea, hits, *total_units);
#endif
        if (*total_units < hits) {
          hits -= *total_units;
          total_player_sea_units[enemy_player_idx][src_sea] -= *total_units;
          enemy_units_count[src_air] -= *total_units;
          *total_units = 0;
          debug_checks();
        } else {
          *total_units -= hits;
          total_player_sea_units[enemy_player_idx][src_sea] -= hits;
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
          total_player_sea_units[enemy_player_idx][src_sea] -= *total_units;
          enemy_units_count[src_air] -= *total_units;
          if (unit_idx >= DESTROYERS && unit_idx <= BS_DAMAGED) {
            enemy_blockade_total[src_sea] -= *total_units;
          }
          *total_units = 0;
          debug_checks();
        } else {
          *total_units -= hits;
          total_player_sea_units[enemy_player_idx][src_sea] -= hits;
          enemy_units_count[src_air] -= hits;
          if (unit_idx >= DESTROYERS && unit_idx <= BS_DAMAGED) {
            enemy_blockade_total[src_sea] -= hits;
          }
          hits = 0;
          debug_checks();
          return;
        }
      }
    }
  }
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
  uint8_t* total_units;
  for (uint8_t unit_idx; unit_idx < ATTACKER_SEA_UNIT_TYPES_COUNT_1; unit_idx++) {
    uint8_t unit_type = ORDER_OF_SEA_ATTACKERS_1[unit_idx];
    uint8_t* total_units = &sea_units_state[src_sea][unit_type][0];
    if (*total_units > 0) {
#ifdef DEBUG
      printf("DEBUG: remove_sea_attackers: unit_type: %d src_sea: %d, hits: %d\n", unit_type,
             src_sea, hits);
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
      printf("DEBUG: remove_sea_attackers carriers: unit_type: %d src_sea: %d, hits: %d\n",
             CARRIERS, src_sea, hits);
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
      printf("DEBUG: remove_sea_attackers cruisers: unit_type: %d src_sea: %d, hits: %d\n",
             CRUISERS, src_sea, hits);
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
  for (uint8_t unit_idx = 0; unit_idx < ATTACKER_SEA_UNIT_TYPES_COUNT_2; unit_idx++) {
    uint8_t unit_type = ORDER_OF_SEA_ATTACKERS_2[unit_idx];
    if (total_player_land_unit_types[0][src_sea][unit_type] == 0)
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
          total_player_sea_units[0][src_sea] -= *total_units;
          *total_units = 0;
          current_player_sea_unit_types[src_sea][unit_type] = 0;
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
  for (uint8_t unit_idx; unit_idx < ATTACKER_SEA_UNIT_TYPES_COUNT_3; unit_idx++) {
    uint8_t unit_type = ORDER_OF_SEA_ATTACKERS_3[unit_idx];
    uint8_t* total_units = &sea_units_state[src_sea][unit_type][0];
    if (*total_units > 0) {
#ifdef DEBUG
      printf("DEBUG: remove_sea_attackers: unit_type: %d src_sea: %d, hits: %d\n", unit_type,
             src_sea, hits);
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
}

void unload_transports() {
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
      uint8_t valid_moves[AIRS_COUNT];
      valid_moves[0] = src_air;
      uint8_t valid_moves_count = 1;
      add_valid_unload_moves(valid_moves, &valid_moves_count, src_sea);
      while (*total_units > 0) {
        uint8_t dst_air = get_user_move_input(unit_type, src_air, valid_moves, valid_moves_count);
#ifdef DEBUG
        printf("DEBUG: unload_transports unit_type: %d, src_sea: %d, dst_air: %d, unload_cargo: "
               "%d, %d\n",
               unit_type, src_sea, dst_air, unload_cargo1, unload_cargo2);
        setPrintableStatus();
        printf("%s\n", printableGameStatus);
#endif
        update_move_history(dst_air, src_sea, valid_moves, &valid_moves_count);
        if (src_air == dst_air) {
          sea_units_state[src_sea][unit_type][0] += *total_units;
          *total_units = 0;
          continue;
        }
        if (enemy_units_count[dst_air] > 0) {
          data.flagged_for_combat[dst_air] = true;
        } else {
          conquer_land(dst_air);
        }
        bombard_max[dst_air]++;
        land_units_state[dst_air][unload_cargo1][0]++;
        current_player_land_unit_types[dst_air][unload_cargo1]++;
        total_player_land_units[0][dst_air]++;
        sea_units_state[src_sea][TRANS_EMPTY][0]++;
        current_player_sea_unit_types[src_sea][TRANS_EMPTY]++;
        current_player_sea_unit_types[src_sea][unit_type]--;
        *total_units -= 1;
        if (unit_type > TRANS_1T) {
          bombard_max[dst_air]++;
          land_units_state[dst_air][unload_cargo2][0]++;
          current_player_land_unit_types[dst_air][unload_cargo2]++;
          total_player_land_units[0][dst_air]++;
        }
        debug_checks();
      }
    }
  }
}
void resolve_land_battles() {
  for (uint8_t src_land = 0; src_land < LANDS_COUNT; src_land++) {
    // check if battle is over
    if (!data.flagged_for_combat[src_land]) {
      continue;
    }
#ifdef DEBUG
    // debug print the current src_land and its name
    printf("Resolve land combat in: %d, Name: %s", src_land, LANDS[src_land].name);
    setPrintableStatus();
    printf("%s\n", printableGameStatus);
#endif
    int* units_land_player_total_0_src_land = &total_player_land_units[0][src_land];
    // check if no friendlies remain
    if (*units_land_player_total_0_src_land == 0) {
      DEBUG_PRINT("No friendlies remain");
      continue;
    }
    uint8_t attacker_damage;
    uint8_t attacker_hits;
    // only bombers exist
    uint8_t* other_land_units_ptr_0_src_land = total_player_land_unit_types[0][src_land];
    uint8_t* bombers_count = &other_land_units_ptr_0_src_land[BOMBERS_LAND_AIR];
    if (*bombers_count > 0 && total_player_land_units[0][src_land] == *bombers_count) {
      if (*factory_hp[src_land] > -*factory_max[src_land]) {
        DEBUG_PRINT("Strategic Bombing");
        // fire_strat_aa_guns();
        uint8_t defender_damage = *bombers_count;
        uint8_t defender_hits =
            (defender_damage / 6) +
            (RANDOM_NUMBERS[random_number_index++] % 6 < defender_damage % 6 ? 1 : 0);
        if (defender_hits > 0) {
          uint8_t* units_land_ptr_src_land_bombers = land_units_state[src_land][BOMBERS_LAND_AIR];
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
              return;
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
    DEBUG_PRINT("Normal Land Combat");
    uint8_t* other_land_units_0_src_land = current_player_land_unit_types[src_land];
    // bombard_shores
    if (bombard_max[src_land] > 0) {
      attacker_damage = 0;
      DEBUG_PRINT("Sea Bombardment");
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
    uint8_t total_air_units = other_land_units_ptr_0_src_land[FIGHTERS] +
                              other_land_units_ptr_0_src_land[BOMBERS_LAND_AIR];
    uint8_t defender_damage;
    uint8_t defender_hits;
    if (total_air_units > 0) {
      int total_aa_guns = 0;
      for (uint8_t enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
        total_aa_guns += total_player_land_unit_types[enemies_0[enemy_idx]][src_land][AA_GUNS];
      }
      if (total_aa_guns > 0) {
        DEBUG_PRINT("Firing AA");
        // fire_tact_aa_guns();
        defender_damage = total_air_units * 3;
        defender_hits = (defender_damage / 6) +
                        (RANDOM_NUMBERS[random_number_index++] % 6 < defender_damage % 6 ? 1 : 0);
        if (defender_hits > 0) {
          for (uint8_t cur_state = 0; cur_state < FIGHTER_STATES; cur_state++) {
            uint8_t* total_units = &land_units_state[src_land][FIGHTERS][cur_state];
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
          for (uint8_t cur_state = 0; cur_state < BOMBER_LAND_STATES; cur_state++) {
            uint8_t* total_units = &land_units_state[src_land][BOMBERS_LAND_AIR][cur_state];
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
      }
    }
    while (true) {
      // print land location name
      printf("Current land battle start src_land: %d, Name: %s", src_land, LANDS[src_land].name);
      setPrintableStatus();
      printf("%s\n", printableGameStatus);

      if (*units_land_player_total_0_src_land == 0) {
        DEBUG_PRINT("No friendlies remain");
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
      // land_battle
      int infantry_count = current_player_land_unit_types[src_land][INFANTRY];
      int artillery_count = current_player_land_unit_types[src_land][ARTILLERY];
      attacker_damage =
          (current_player_land_unit_types[src_land][FIGHTERS] * FIGHTER_ATTACK) +
          (current_player_land_unit_types[src_land][BOMBERS_LAND_AIR] * BOMBER_ATTACK) +
          (infantry_count * INFANTRY_ATTACK) + (artillery_count * ARTILLERY_ATTACK) +
          (current_player_land_unit_types[src_land][TANKS] * TANK_ATTACK);
      // add damage for the minimum of count of infantry/artillery
      attacker_damage += infantry_count < artillery_count ? infantry_count : artillery_count;
      attacker_hits = (attacker_damage / 6) +
                      (RANDOM_NUMBERS[random_number_index++] % 6 < attacker_damage % 6 ? 1 : 0);
      defender_damage = 0;
      printf("Enemy Count: %d\n", enemy_units_count[src_land]);
      for (uint8_t enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
        uint8_t* land_units = total_player_land_unit_types[enemies_0[enemy_idx]][src_land];
        defender_damage += (land_units[INFANTRY] * INFANTRY_DEFENSE) +
                           (land_units[ARTILLERY] * ARTILLERY_DEFENSE) +
                           (land_units[TANKS] * TANK_DEFENSE) +
                           (land_units[FIGHTERS] * FIGHTER_DEFENSE) +
                           (land_units[BOMBERS_LAND_AIR] * BOMBER_DEFENSE);
        defender_hits = (defender_damage / 6) +
                        (RANDOM_NUMBERS[random_number_index++] % 6 < defender_damage % 6 ? 1 : 0);
      }
      if (defender_hits > 0) {
        printf("Defender Hits: %d", defender_hits);
        remove_land_attackers(src_land, defender_hits);
      }
      if (attacker_hits > 0) {
        printf("Attacker Hits: %d", attacker_hits);
        remove_land_defenders(src_land, attacker_hits);
      }
      // ask to retreat (0-255, any non valid retreat zone is considered a no)
      uint8_t retreat = ask_to_retreat();
      // if retreat, move units to retreat zone immediately and end battle
      if (LAND_DIST[src_land][retreat] == 1 && enemy_units_count[retreat] == 0 &&
          !data.flagged_for_combat[retreat]) {
        for (uint8_t unit_type = INFANTRY; unit_type <= TANKS; unit_type++) {
          int total_units = land_units_state[src_land][unit_type][0];
          land_units_state[retreat][unit_type][0] += total_units;
          current_player_land_unit_types[retreat][unit_type] += total_units;
          total_player_land_units[0][retreat] += total_units;
          current_player_land_unit_types[src_land][unit_type] -= total_units;
          *units_land_player_total_0_src_land -= total_units;
          land_units_state[src_land][unit_type][0] = 0;
        }
        data.flagged_for_combat[src_land] = false;
        break;
      }
    }
  }
}

void add_valid_unload_moves(uint8_t* valid_moves, uint8_t* valid_moves_count, uint8_t src_sea) {
  uint8_t* near_land = SEA_TO_LAND_CONN[src_sea];
  uint8_t near_land_count = SEA_TO_LAND_COUNT[src_sea];
  for (int land_idx = 0; land_idx < near_land_count; land_idx++) {
    uint8_t dst_land = near_land[land_idx];
    add_valid_air_move_if_history_allows_X(valid_moves, valid_moves_count, dst_land,
                                           src_sea + LANDS_COUNT, 1);
  }
}

void add_valid_fighter_moves(uint8_t* valid_moves, uint8_t* valid_moves_count, uint8_t src_air,
                             uint8_t remaining_moves) {
  uint8_t* near_air = AIR_WITHIN_X_MOVES[remaining_moves - 1][src_air];
  uint8_t near_air_count = AIR_WITHIN_X_MOVES_COUNT[remaining_moves - 1][src_air];
  for (int i = 0; i < near_air_count; i++) {
    uint8_t dst_air = near_air[i];
    uint8_t air_dist = AIR_DIST[src_air][dst_air];
    if (air_dist <= 2 || canFighterLandHere[dst_air] ||
        (air_dist == 3 && canFighterLandIn1Move[dst_air])) {
      if (!canFighterLandHere[dst_air] && enemy_units_count[dst_air] == 0) // waste of a move
        continue;
      add_valid_air_move_if_history_allows_X(valid_moves, valid_moves_count, dst_air, src_air,
                                             air_dist);
    }
  }
}

void add_valid_fighter_landing(uint8_t* valid_moves, uint8_t* valid_moves_count, uint8_t src_air,
                               uint8_t remaining_moves) {
  uint8_t* near_air = AIR_WITHIN_X_MOVES[remaining_moves - 1][src_air];
  uint8_t near_air_count = AIR_WITHIN_X_MOVES_COUNT[remaining_moves - 1][src_air];
  for (int i = 0; i < near_air_count; i++) {
    uint8_t dst_air = near_air[i];
    if (canFighterLandHere[dst_air]) {
      add_valid_air_move_if_history_allows_X(valid_moves, valid_moves_count, dst_air, src_air,
                                             remaining_moves);
    }
  }
}

void add_valid_bomber_moves(uint8_t* valid_moves, uint8_t* valid_moves_count, uint8_t src_air,
                            uint8_t remaining_moves) {
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
      add_valid_air_move_if_history_allows_X(valid_moves, valid_moves_count, dst_air, src_air,
                                             air_dist);
    }
  }
}

void land_fighter_units() {
  //  refresh_canFighterLandHere_final
  for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    // if (allied_carriers[sea_idx] > 0) {
    canFighterLandHere[sea_idx] = allied_carriers[sea_idx] > 0;
    //}
  }
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    // is allied owned and not recently conquered?
    // canFighterLandHere[land_idx] =
    //    (is_allied_0[owner_idx[land_idx]] && !data.flagged_for_combat[land_idx]);
    // check for possiblity to build carrier under fighter
    int land_owner = *owner_idx[land_idx];
    if (*factory_max[land_idx] > 0 && land_owner == data.player_index) {
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
      uint8_t* total_fighter_count = &air_units_state[src_air][FIGHTERS][cur_state];
      if (*total_fighter_count == 0)
        continue;
      uint8_t valid_moves[AIRS_COUNT];
      // valid_moves[0] = src_air;
      uint8_t valid_moves_count = 0;
      add_valid_fighter_landing(valid_moves, &valid_moves_count, src_air, cur_state);
      if (valid_moves_count == 0 || canFighterLandHere[src_air]) {
        valid_moves[valid_moves_count++] = src_air;
      }
      while (*total_fighter_count > 0) {
        uint8_t dst_air = get_user_move_input(FIGHTERS, src_air, valid_moves, valid_moves_count);
#ifdef DEBUG
        setPrintableStatus();
        printf("%s\n", printableGameStatus);
        printf("DEBUG: player: %s landing fighters %d, src_air: %d, dst_air: %d\n",
               PLAYERS[data.player_index].name, FIGHTERS, src_air, dst_air);
#endif
        update_move_history(dst_air, src_air, valid_moves, &valid_moves_count);
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
          uint8_t dst_sea = dst_air - LANDS_COUNT;
          total_player_sea_units[0][dst_sea]++;
          current_player_sea_unit_types[dst_sea][FIGHTERS]++;
        }
        if (src_air < LANDS_COUNT) {
          total_player_land_units[0][src_air]--;
          current_player_land_unit_types[dst_air][FIGHTERS]--;
        } else {
          uint8_t src_sea = src_air - LANDS_COUNT;
          total_player_sea_units[0][src_sea]--;
          current_player_sea_unit_types[src_sea][FIGHTERS]--;
        }
        *total_fighter_count -= 1;
      }
    }
  }
}

void add_valid_bomber_landing(uint8_t* valid_moves, uint8_t* valid_moves_count, uint8_t src_air,
                              uint8_t movement_remaining) {
  uint8_t* near_land = AIR_TO_LAND_WITHIN_X_MOVES[movement_remaining - 1][src_air];
  for (int i = 0; i < AIR_TO_LAND_WITHIN_X_MOVES_COUNT[movement_remaining - 1][src_air]; i++) {
    uint8_t dst_air = near_land[i];
    if (canBomberLandHere[dst_air] == 1) {
      valid_moves[(*valid_moves_count)++] = dst_air;
    }
  }
}

void land_bomber_units() {
  // check if any bombers have moves remaining
  for (uint8_t cur_state = 1; cur_state < BOMBER_LAND_STATES - 1;
       cur_state++) { // TODO optimize to find next bomber faster
    uint8_t* total_bomber_count = &air_units_state[0][BOMBERS_LAND_AIR][cur_state];
    clear_move_history();
    for (int src_air = 0; src_air < AIRS_COUNT; src_air++) {
      uint8_t valid_moves[AIRS_COUNT];
      valid_moves[0] = src_air;
      uint8_t valid_moves_count = 1;
      uint8_t movement_remaining = cur_state + (src_air < LANDS_COUNT ? 0 : 1);
      add_valid_bomber_landing(valid_moves, &valid_moves_count, src_air, movement_remaining);
      while (*total_bomber_count > 0) {
        uint8_t dst_air =
            get_user_move_input(BOMBERS_LAND_AIR, src_air, valid_moves, valid_moves_count);
#ifdef DEBUG
        setPrintableStatus();
        printf("%s\n", printableGameStatus);
        printf("DEBUG: player: %s landing bombers %d, src_air: %d, dst_air: %d\n",
               PLAYERS[data.player_index].name, BOMBERS_LAND_AIR, src_air, dst_air);
#endif
        update_move_history(dst_air, src_air, valid_moves, &valid_moves_count);
        if (src_air == dst_air) {
          air_units_state[src_air][BOMBERS_LAND_AIR][0]++;
          *total_bomber_count -= 1;
          continue;
        }
        air_units_state[dst_air][BOMBERS_LAND_AIR][0]++;
        total_player_land_units[0][dst_air]++;
        current_player_land_unit_types[dst_air][BOMBERS_LAND_AIR]++;
        if (src_air < LANDS_COUNT) {
          total_player_land_units[0][src_air]--;
          current_player_land_unit_types[src_air][BOMBERS_LAND_AIR]--;
        } else {
          uint8_t src_sea = src_air - LANDS_COUNT;
          total_player_sea_units[0][src_sea]--;
          current_player_land_unit_types[src_sea][BOMBERS_LAND_AIR]--;
        }
        *total_bomber_count -= 1;
      }
    }
  }
}
void buy_units() {
  setPrintableStatus();
  printf("%s\n", printableGameStatus);

  for (uint8_t factory_idx = 0; factory_idx < total_factory_count[0]; factory_idx++) {
    uint8_t dst_land = factory_locations[0][factory_idx];
    if (data.builds_left[dst_land] == 0) {
      continue;
    }
    uint8_t repair_cost = 0;
    // buy sea units
    for (uint8_t sea_idx = 0; sea_idx < LAND_TO_SEA_COUNT[dst_land]; sea_idx++) {
      uint8_t dst_sea = LAND_TO_SEA_CONN[dst_land][sea_idx];
      uint8_t dst_air = dst_sea + LANDS_COUNT;
      uint8_t valid_purchases[COST_UNIT_SEA_COUNT + 1];
      valid_purchases[0] = SEA_UNIT_TYPES_COUNT; // pass all units
      uint8_t last_purchased = 0;
      while (data.builds_left[dst_air] > 0) {
        if (data.money[0] < TRANSPORT_COST) {
          data.builds_left[dst_air] = 0;
          break;
        }
        uint8_t units_built = *factory_max[dst_land] - data.builds_left[dst_land];
        if (*factory_hp[dst_land] <= units_built)
          repair_cost = 1 + units_built - *factory_hp[dst_land]; // subtracting a negative
        // add all units that can be bought
        uint8_t valid_purchases_count = 1;
        for (int unit_type_idx = COST_UNIT_SEA_COUNT - 1; unit_type_idx >= 0; unit_type_idx--) {
          uint8_t unit_type = BUY_UNIT_SEA[unit_type_idx];
          if (unit_type < last_purchased)
            break;
          if (data.money[0] < COST_UNIT_SEA[unit_type] + repair_cost)
            continue;
          valid_purchases[valid_purchases_count++] = unit_type;
        }
        if (valid_purchases_count == 1) {
          data.builds_left[dst_air] = 0;
          break;
        }
        uint8_t purchase = get_user_purchase_input(dst_air, valid_purchases, valid_purchases_count);
        if (purchase == SEA_UNIT_TYPES_COUNT) { // pass all units
          data.builds_left[dst_air] = 0;
          break;
        }
#ifdef DEBUG
        // print which player is buying which unit at which location
        char debug_message[100];
        snprintf(debug_message, sizeof(debug_message), "Player %d buying %s at %s",
                 data.player_index, NAMES_UNIT_SEA[purchase], SEAS[dst_sea].name);
#endif
        for (uint8_t sea_idx2 = sea_idx; sea_idx2 < LAND_TO_SEA_COUNT[dst_land]; sea_idx2++) {
          data.builds_left[LAND_TO_SEA_CONN[dst_land][sea_idx2] + LANDS_COUNT]--;
        }
        data.builds_left[dst_land]--;
        *factory_hp[dst_land] += repair_cost;
        data.money[0] -= COST_UNIT_SEA[purchase] + repair_cost;
        sea_units_state[dst_sea][purchase][0]++;
        total_player_sea_units[0][dst_sea]++;
        current_player_sea_unit_types[dst_sea][purchase]++;
        last_purchased = purchase;
      }
    }
    // buy land units
    uint8_t valid_purchases[LAND_UNIT_TYPES_COUNT + 1];
    valid_purchases[0] = LAND_UNIT_TYPES_COUNT; // pass all units
    uint8_t unit_type = 0;
    uint8_t last_purchased = 0;
    while (data.builds_left[dst_land] > 0) {
      if (data.money[0] < INFANTRY_COST) {
        data.builds_left[dst_land] = 0;
        break;
      }
      uint8_t units_built = *factory_max[dst_land] - data.builds_left[dst_land];
      if (*factory_hp[dst_land] <= units_built)
        repair_cost = 1 + units_built - *factory_hp[dst_land]; // subtracting a negative
      // add all units that can be bought
      uint8_t valid_purchases_count = 1;
      for (int unit_type = LAND_UNIT_TYPES_COUNT - 1; unit_type >= 0; unit_type--) {
        if (unit_type < last_purchased)
          break;
        if (data.money[0] < COST_UNIT_LAND[unit_type] + repair_cost)
          continue;
        valid_purchases[valid_purchases_count++] = unit_type;
      }
      if (valid_purchases_count == 1) {
        data.builds_left[dst_land] = 0;
        break;
      }
      uint8_t purchase = get_user_purchase_input(dst_land, valid_purchases, valid_purchases_count);
      if (purchase == LAND_UNIT_TYPES_COUNT) { // pass all units
        data.builds_left[dst_land] = 0;
        break;
      }
#ifdef DEBUG
      // print which player is buying which unit at which location
      char debug_message[100];
      snprintf(debug_message, sizeof(debug_message), "Player %d buying %s at %s", data.player_index,
               NAMES_UNIT_LAND[purchase], LANDS[dst_land].name);
#endif
      data.builds_left[dst_land]--;
      *factory_hp[dst_land] += repair_cost;
      data.money[0] -= COST_UNIT_LAND[purchase] + repair_cost;
      land_units_state[dst_land][purchase][0]++;
      total_player_land_units[0][dst_land]++;
      total_player_land_unit_types[0][dst_land][purchase]++;
      last_purchased = purchase;
    }
  }
}

void crash_air_units() {
  // crash planes not on friendly land
  for (uint8_t air_idx = 0; air_idx < LANDS_COUNT; air_idx++) {
    if (canFighterLandHere[air_idx]) {
      continue;
    }
    if (current_player_land_unit_types[air_idx][FIGHTERS] == 0) {
      continue;
    }
#ifdef DEBUG
    printf("DEBUG: Crashing fighters at %d\n", air_idx);
#endif
    total_player_land_units[0][air_idx] -= current_player_land_unit_types[air_idx][FIGHTERS];
    current_player_land_unit_types[air_idx][FIGHTERS] = 0;
    air_units_state[air_idx][FIGHTERS][0] = 0;
  }
  // crash planes not on allied carriers
  for (uint8_t air_idx = LANDS_COUNT; air_idx < AIRS_COUNT; air_idx++) {
    // units_air_ptr[air_idx][BOMBERS_LAND_AIR][0] = 0;
    uint8_t sea_idx = air_idx - LANDS_COUNT;
    uint8_t free_space = allied_carriers[sea_idx] * 2;
    for (uint8_t player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      free_space -= total_player_sea_unit_types[player_idx][sea_idx][FIGHTERS];
    }
    uint8_t* total_fighter_count = &air_units_state[air_idx][FIGHTERS][0];
    if (free_space < *total_fighter_count) {
      uint8_t fighters_lost = *total_fighter_count - free_space;
#ifdef DEBUG
      printf("DEBUG: Crashing %d fighters at %d\n", fighters_lost, air_idx);
#endif
      total_player_sea_units[0][sea_idx] -= fighters_lost;
      current_player_sea_unit_types[sea_idx][FIGHTERS] -= fighters_lost;
      *total_fighter_count = free_space;
    }
  }
}
void reset_units_fully() {
  // reset battleship health
  for (uint8_t sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    sea_units_state[sea_idx][BATTLESHIPS][0] += sea_units_state[sea_idx][BS_DAMAGED][0];
    sea_units_state[sea_idx][BS_DAMAGED][0] = 0;
  }
}
// TODO BUY FACTORY
void buy_factory() {}
void collect_money() {
  // if player still owns their capital, collect income
  data.money[0] +=
      (income_per_turn[0] * (*owner_idx[PLAYERS[data.player_index].capital_territory_index] == 0));
}
void rotate_turns() {
  // rotate units
#ifdef DEBUG
  setPrintableStatus();
  printf("%s\n", printableGameStatus);
  printf("DEBUG: Rotating turns\n");
#endif
  memcpy(&total_land_unit_types_temp, &current_player_land_unit_types, OTHER_LAND_UNITS_SIZE);
  memcpy(&current_player_land_unit_types, &data.other_land_units[0], OTHER_LAND_UNITS_SIZE);
  memmove(&data.other_land_units[0], &data.other_land_units[1], MULTI_OTHER_LAND_UNITS_SIZE);
  memcpy(&data.other_land_units[PLAYERS_COUNT - 2], &total_land_unit_types_temp,
         OTHER_LAND_UNITS_SIZE);
  for (uint8_t dst_land = 0; dst_land < LANDS_COUNT; dst_land++) {
    LandState* land0 = &data.land_state[dst_land];
    uint8_t* land1 = current_player_land_unit_types[dst_land];
    // TODO optimize restructuring
    memset(&data.land_state[dst_land].fighters, 0, sizeof(data.land_state[0].fighters));
    land0->fighters[FIGHTER_STATES - 1] = land1[FIGHTERS];
    memset(&data.land_state[dst_land].bombers, 0, sizeof(data.land_state[0].bombers));
    land0->bombers[BOMBER_LAND_STATES - 1] = land1[BOMBERS_LAND_AIR];
    memset(&data.land_state[dst_land].infantry, 0, sizeof(data.land_state[0].infantry));
    land0->infantry[INFANTRY_STATES - 1] = land1[INFANTRY];
    memset(&data.land_state[dst_land].artillery, 0, sizeof(data.land_state[0].artillery));
    land0->artillery[ARTILLERY_STATES - 1] = land1[ARTILLERY];
    memset(&data.land_state[dst_land].tanks, 0, sizeof(data.land_state[0].tanks));
    land0->tanks[TANK_STATES - 1] = land1[TANKS];
    memset(&data.land_state[dst_land].aa_guns, 0, sizeof(data.land_state[0].aa_guns));
    land0->aa_guns[AA_GUN_STATES - 1] = land1[AA_GUNS];
  }
  memcpy(&total_sea_units_temp, &current_player_sea_unit_types, SEA_UNIT_TYPES_COUNT * SEAS_COUNT);
  //  memcpy(&other_sea_units_0, &data.other_sea_units[0], OTHER_SEA_UNITS_SIZE);
  for (uint8_t dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
    memcpy(&current_player_sea_unit_types[dst_sea], &data.other_sea_units[0][dst_sea],
           SEA_UNIT_TYPES_COUNT - 1);
  }
  memmove(&data.other_sea_units[0], &data.other_sea_units[1], MULTI_OTHER_SEA_UNITS_SIZE);
  //  memcpy(&data.other_sea_units[PLAYERS_COUNT - 2], &other_sea_units_temp, OTHER_SEA_UNITS_SIZE);
  memset(&data.units_sea, 0, sizeof(data.units_sea));
  for (uint8_t dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
    memcpy(&data.other_sea_units[PLAYERS_COUNT - 2][dst_sea], &total_sea_units_temp[dst_sea],
           SEA_UNIT_TYPES_COUNT - 1);
    for (uint8_t unit_type = 0; unit_type < LAND_UNIT_TYPES_COUNT; unit_type++) {
      UnitsSea* sea0 = &data.units_sea[dst_sea];
      uint8_t* sea1 = current_player_sea_unit_types[dst_sea];
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
  uint8_t temp_money = data.money[0];
  memmove(&data.money[0], &data.money[1], sizeof(data.money[0]) * (PLAYERS_COUNT - 1));
  data.money[PLAYERS_COUNT - 1] = temp_money;
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
  memset(&data.flagged_for_combat, 0, sizeof(data.flagged_for_combat));
  data.player_index = (data.player_index + 1) % PLAYERS_COUNT;

  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    data.land_state[land_idx].owner_idx =
        (data.land_state[land_idx].owner_idx + PLAYERS_COUNT - 1) % PLAYERS_COUNT;
  }

  for (int land_idx = 0; land_idx < total_factory_count[0]; land_idx++) {
    int dst_land = factory_locations[0][land_idx];
    data.builds_left[dst_land] = *factory_max[dst_land];
    for (int sea_idx = 0; sea_idx < LAND_TO_SEA_COUNT[dst_land]; sea_idx++) {
      data.builds_left[LAND_TO_SEA_CONN[dst_land][sea_idx] + LANDS_COUNT] += *factory_max[dst_land];
    }
  }
  refresh_cache();
#ifdef DEBUG
  printf("DEBUG: Cache refreshed. Player %s's turn\n", PLAYERS[data.player_index].name);
  setPrintableStatus();
  printf("%s\n", printableGameStatus);
#endif

  json = serialize_game_data_to_json(&data);
  write_json_to_file("backup_game.json", json);
  cJSON_Delete(json);
}

double get_score() {
  int allied_score = 1; // one helps prevent division by zero
  int enemy_score = 1;
  for (int player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    int score = data.money[player_idx];
    for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
      for (int unit_type = 0; unit_type < LAND_UNIT_TYPES_COUNT; unit_type++) {
        score += total_player_land_unit_types[player_idx][land_idx][unit_type] *
                 COST_UNIT_LAND[unit_type];
      }
    }
    for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
      for (int unit_type = 0; unit_type < SEA_UNIT_TYPES_COUNT; unit_type++) {
        score +=
            total_player_sea_unit_types[player_idx][sea_idx][unit_type] * COST_UNIT_SEA[unit_type];
      }
    }
    if (is_allied_0[player_idx]) {
      allied_score += score;
    } else {
      enemy_score += score;
    }
  }
  return ((double)2 * (double)allied_score / (double)(enemy_score + allied_score)) - (double)1;
}