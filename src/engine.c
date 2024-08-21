#include "engine.h"
#include "serialize_data.h"
#include "units/units.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

char buffer[STRING_BUFFER_SIZE];
char threeCharStr[5];
GameData data = {0};
cJSON* json;
// GameCache cache = {0};
uint8_t current_player_index;
uint8_t current_player_index_plus_one;
Player current_player;
char* current_player_name;
char* current_player_color;
bool current_player_is_human;
uint8_t current_player_money;
char printableGameStatus[5000] = "";
char* player_names[PLAYERS_COUNT];
char* player_colors[PLAYERS_COUNT];
uint8_t money[PLAYERS_COUNT];
// uint8_t units_land_type_total[LANDS_COUNT][LAND_UNIT_TYPES];
uint8_t units_land_player_total[LANDS_COUNT][PLAYERS_COUNT];
uint8_t units_land_grand_total[LANDS_COUNT];
uint8_t* units_land_ptr[LANDS_COUNT][LAND_UNIT_TYPES];
// uint8_t units_sea_type_total[SEAS_COUNT][SEA_UNIT_TYPES];
uint8_t units_sea_player_total[SEAS_COUNT][PLAYERS_COUNT];
#define BLOCKADE_UNIT_TYPES_COUNT 5
uint8_t BLOCKADE_UNIT_TYPES[BLOCKADE_UNIT_TYPES_COUNT] = {DESTROYERS, CARRIERS, CRUISERS,
                                                          BATTLESHIPS, BS_DAMAGED};
uint8_t units_sea_blockade_total[SEAS_COUNT][PLAYERS_COUNT];
uint8_t units_sea_grand_total[SEAS_COUNT];
uint8_t air_total[AIRS_COUNT][AIR_UNIT_TYPES_COUNT];
uint8_t* units_sea_ptr[SEAS_COUNT][SEA_UNIT_TYPES];
uint8_t* units_air_ptr[AIRS_COUNT][AIR_UNIT_TYPES_COUNT];
uint8_t income_per_turn[PLAYERS_COUNT];
uint8_t enemies[PLAYERS_COUNT - 1];
uint8_t enemies_count;
uint8_t canal_state;
uint8_t gamestate_player_to_mapstate_player[PLAYERS_COUNT];
uint8_t mapstate_player_to_gamestate_player[PLAYERS_COUNT];
bool is_allied[PLAYERS_COUNT];
char* LAND_NAMES[LANDS_COUNT] = {0};
char* SEA_NAMES[SEAS_COUNT] = {0};
char* AIR_NAMES[AIRS_COUNT] = {0};
uint8_t original_owner_idx[LANDS_COUNT] = {0};
uint8_t LAND_VALUE[LANDS_COUNT] = {0};
uint8_t LAND_DIST[LANDS_COUNT][AIRS_COUNT] = {INFINITY};
uint8_t AIR_CONN_COUNT[AIRS_COUNT] = {0};
uint8_t AIR_CONNECTIONS[AIRS_COUNT][MAX_AIR_TO_AIR_CONNECTIONS] = {0};
uint8_t AIR_DIST[AIRS_COUNT][AIRS_COUNT] = {INFINITY};
uint8_t LAND_PATH[LAND_MOVE_SIZE][LANDS_COUNT][AIRS_COUNT] = {INFINITY};
uint8_t land_path1[LANDS_COUNT][AIRS_COUNT] = {INFINITY};
uint8_t land_path2[LANDS_COUNT][AIRS_COUNT] = {INFINITY};
uint8_t LAND_PATH_ALT[LANDS_COUNT][AIRS_COUNT] = {INFINITY};
uint8_t LANDS_WITHIN_1_MOVE[LANDS_COUNT][LANDS_COUNT - 1] = {0};
uint8_t LANDS_WITHIN_1_MOVE_COUNT[LANDS_COUNT] = {0};
uint8_t LANDS_WITHIN_2_MOVES[LANDS_COUNT][LANDS_COUNT - 1] = {0};
uint8_t LANDS_WITHIN_2_MOVES_COUNT[LANDS_COUNT] = {0};
// uint8_t LOAD_WITHIN_1_MOVE[LANDS_COUNT][SEAS_COUNT] = {0};
// uint8_t LOAD_WITHIN_1_MOVE_COUNT[LANDS_COUNT] = {0};
uint8_t LOAD_WITHIN_2_MOVES[LANDS_COUNT][SEAS_COUNT] = {0};
uint8_t LOAD_WITHIN_2_MOVES_COUNT[LANDS_COUNT] = {0};
uint8_t UNLOAD_WITHIN_1_MOVE[SEAS_COUNT][LANDS_COUNT] = {0};
uint8_t UNLOAD_WITHIN_1_MOVE_COUNT[SEAS_COUNT] = {0};
uint8_t SEAS_WITHIN_1_MOVE[CANALS_COUNT][SEAS_COUNT][SEAS_COUNT - 1] = {0};
uint8_t SEAS_WITHIN_1_MOVE_COUNT[CANALS_COUNT][SEAS_COUNT] = {0};
uint8_t SEAS_WITHIN_2_MOVES[CANALS_COUNT][SEAS_COUNT][SEAS_COUNT - 1] = {0};
uint8_t SEAS_WITHIN_2_MOVES_COUNT[CANALS_COUNT][SEAS_COUNT] = {0};
uint8_t AIR_WITHIN_X_MOVES[6][AIRS_COUNT][AIRS_COUNT - 1] = {0};
uint8_t AIR_WITHIN_X_MOVES_COUNT[6][AIRS_COUNT] = {0};
// uint8_t AIR_WITHIN_1_MOVE[AIRS_COUNT][AIRS_COUNT - 1] = {0};
// uint8_t AIR_WITHIN_1_MOVE_COUNT[AIRS_COUNT] = {0};
// uint8_t AIR_WITHIN_2_MOVES[AIRS_COUNT][AIRS_COUNT - 1] = {0};
// uint8_t AIR_WITHIN_2_MOVES_COUNT[AIRS_COUNT] = {0};
// uint8_t AIR_WITHIN_3_MOVES[AIRS_COUNT][AIRS_COUNT - 1] = {0};
// uint8_t AIR_WITHIN_3_MOVES_COUNT[AIRS_COUNT] = {0};
// uint8_t AIR_WITHIN_4_MOVES[AIRS_COUNT][AIRS_COUNT - 1] = {0};
// uint8_t AIR_WITHIN_4_MOVES_COUNT[AIRS_COUNT] = {0};
// uint8_t AIR_WITHIN_5_MOVES[AIRS_COUNT][AIRS_COUNT - 1] = {0};
// uint8_t AIR_WITHIN_5_MOVES_COUNT[AIRS_COUNT] = {0};
// uint8_t AIR_WITHIN_6_MOVES[AIRS_COUNT][AIRS_COUNT - 1] = {0};
// uint8_t AIR_WITHIN_6_MOVES_COUNT[AIRS_COUNT] = {0};
uint8_t AIR_TO_LAND_WITHIN_X_MOVES[5][AIRS_COUNT][LANDS_COUNT - 1] = {0};
uint8_t AIR_TO_LAND_WITHIN_X_MOVES_COUNT[5][AIRS_COUNT] = {0};
// uint8_t AIR_TO_LAND_WITHIN_1_MOVE[AIRS_COUNT][LANDS_COUNT - 1] = {0};
// uint8_t AIR_TO_LAND_WITHIN_1_MOVE_COUNT[AIRS_COUNT] = {0};
// uint8_t AIR_TO_LAND_WITHIN_2_MOVES[AIRS_COUNT][LANDS_COUNT - 1] = {0};
// uint8_t AIR_TO_LAND_WITHIN_2_MOVES_COUNT[AIRS_COUNT] = {0};
// uint8_t AIR_TO_LAND_WITHIN_3_MOVES[AIRS_COUNT][LANDS_COUNT - 1] = {0};
// uint8_t AIR_TO_LAND_WITHIN_3_MOVES_COUNT[AIRS_COUNT] = {0};
// uint8_t AIR_TO_LAND_WITHIN_4_MOVES[AIRS_COUNT][LANDS_COUNT - 1] = {0};
// uint8_t AIR_TO_LAND_WITHIN_4_MOVES_COUNT[AIRS_COUNT] = {0};
// uint8_t AIR_TO_LAND_WITHIN_5_MOVES[AIRS_COUNT][LANDS_COUNT - 1] = {0};
// uint8_t AIR_TO_LAND_WITHIN_5_MOVES_COUNT[AIRS_COUNT] = {0};
uint8_t SEA_DIST[CANAL_STATES][SEAS_COUNT][SEAS_COUNT] = {INFINITY};
uint8_t sea_dist[SEAS_COUNT][AIRS_COUNT] = {
    INFINITY}; // TODO optimize sea_count rather than air_count
uint8_t SEA_PATH[SEA_MOVE_SIZE][CANAL_STATES][SEAS_COUNT][AIRS_COUNT] = {INFINITY};
uint8_t SEA_PATH_ALT[CANAL_STATES][SEAS_COUNT][AIRS_COUNT] = {INFINITY};
uint8_t sea_path1[SEAS_COUNT][AIRS_COUNT] = {INFINITY};
uint8_t sea_path2[SEAS_COUNT][AIRS_COUNT] = {INFINITY};
uint8_t sea_path_alt1[SEAS_COUNT][AIRS_COUNT] = {INFINITY};
uint8_t AIR_PATH[AIR_MOVE_SIZE][AIRS_COUNT][AIRS_COUNT] = {INFINITY};
uint8_t air_path2[AIRS_COUNT][AIRS_COUNT] = {INFINITY};
uint8_t air_path3[AIRS_COUNT][AIRS_COUNT] = {INFINITY};
uint8_t air_path4[AIRS_COUNT][AIRS_COUNT] = {INFINITY};
uint8_t air_path5[AIRS_COUNT][AIRS_COUNT] = {INFINITY};
uint8_t air_path6[AIRS_COUNT][AIRS_COUNT] = {INFINITY};
uint8_t LAND_TO_LAND_COUNT[LANDS_COUNT] = {0};
uint8_t LAND_TO_LAND_CONN[LANDS_COUNT][MAX_LAND_TO_LAND_CONNECTIONS] = {0};
uint8_t LAND_TO_SEA_COUNT[LANDS_COUNT] = {0};
uint8_t LAND_TO_SEA_CONN[LANDS_COUNT][MAX_LAND_TO_SEA_CONNECTIONS] = {0};
uint8_t SEA_TO_SEA_COUNT[SEAS_COUNT] = {0};
uint8_t SEA_TO_SEA_CONN[SEAS_COUNT][MAX_SEA_TO_SEA_CONNECTIONS] = {0};
uint8_t SEA_TO_LAND_COUNT[SEAS_COUNT] = {0};
uint8_t SEA_TO_LAND_CONN[SEAS_COUNT][MAX_SEA_TO_LAND_CONNECTIONS] = {0};

#define DEFENDER_LAND_UNIT_TYPES 6
uint8_t ORDER_OF_LAND_DEFENDERS[DEFENDER_LAND_UNIT_TYPES] = {
    AA_GUNS, BOMBERS_LAND_AIR, INFANTRY, ARTILLERY, TANKS, FIGHTERS};

#define ATTACKER_LAND_UNIT_TYPES_1 3
uint8_t ORDER_OF_LAND_ATTACKERS_1[ATTACKER_LAND_UNIT_TYPES_1] = {INFANTRY, ARTILLERY, TANKS};
#define ATTACKER_LAND_UNIT_TYPES_2 2
uint8_t ORDER_OF_LAND_ATTACKERS_2[ATTACKER_LAND_UNIT_TYPES_2] = {FIGHTERS, BOMBERS_LAND_AIR};

#define DEFENDER_SEA_UNIT_TYPES 13
#define ATTACKER_SEA_UNIT_TYPES_1 2
#define ATTACKER_SEA_UNIT_TYPES_2 2
#define ATTACKER_SEA_UNIT_TYPES_3 8
uint8_t ORDER_OF_SEA_DEFENDERS[DEFENDER_SEA_UNIT_TYPES] = {
    SUBMARINES, DESTROYERS, CARRIERS, CRUISERS, FIGHTERS,    BS_DAMAGED, TRANS_EMPTY,
    TRANS_1I,   TRANS_1A,   TRANS_1T, TRANS_2I, TRANS_1I_1A, TRANS_1I_1T};
uint8_t ORDER_OF_SEA_ATTACKERS_1[ATTACKER_SEA_UNIT_TYPES_1] = {SUBMARINES, DESTROYERS};
uint8_t ORDER_OF_SEA_ATTACKERS_2[ATTACKER_SEA_UNIT_TYPES_2] = {FIGHTERS, BOMBERS_SEA};
uint8_t ORDER_OF_SEA_ATTACKERS_3[ATTACKER_SEA_UNIT_TYPES_3] = {
    BS_DAMAGED, TRANS_EMPTY, TRANS_1I, TRANS_1A, TRANS_1T, TRANS_2I, TRANS_1I_1A, TRANS_1I_1T};
//#define SHIPS_UNIT_TYPES_COUNT 13
// uint8_t SHIPS_UNIT_TYPES[SHIPS_UNIT_TYPES_COUNT] = {TRANS_EMPTY, TRANS_1I, TRANS_1A, TRANS_1T,
// TRANS_2I, TRANS_1I_1A, TRANS_1I_1T, SUBMARINES, DESTROYERS, CARRIERS, CRUISERS, BATTLESHIPS,
// BS_DAMAGED};
uint8_t allied_carriers[SEAS_COUNT] = {0};
uint8_t canal_state;
uint8_t owner_idx[LANDS_COUNT];
uint8_t builds_left[LANDS_COUNT];
uint8_t factory_max[LANDS_COUNT];
int8_t factory_hp[LANDS_COUNT]; // allow negative
uint8_t bombard_max[LANDS_COUNT];
bool flagged_for_combat[AIRS_COUNT];
uint8_t other_land_units[LANDS_COUNT][PLAYERS_COUNT][LAND_UNIT_TYPES];
uint8_t other_sea_units[SEAS_COUNT][PLAYERS_COUNT][SEA_UNIT_TYPES - 1];
bool transports_with_large_cargo_space[SEAS_COUNT];
uint8_t zeros_transports_with_large_cargo_space[TRANS_EMPTY_STATES + TRANS_1I_STATES] = {0};
bool transports_with_small_cargo_space[SEAS_COUNT];
uint8_t zeros_transports_with_small_cargo_space[TRANS_EMPTY_STATES + TRANS_1I_STATES +
                                                TRANS_1A_STATES + TRANS_1T_STATES] = {0};
uint8_t skipped_airs[AIRS_COUNT][AIRS_COUNT] = {0};
uint8_t source_territories[AIRS_COUNT][AIRS_COUNT] = {0};
uint8_t source_territories_count[AIRS_COUNT] = {0};
uint8_t enemy_units_count[AIRS_COUNT] = {0};
uint8_t total_enemy_subs[SEAS_COUNT] = {0};
uint8_t my_factory_locations[LANDS_COUNT] = {0};
uint8_t my_factory_count = 0;
uint8_t RETREAT_OPTIONS[256] = {0};
//#define BOMBARDING_UNITS_COUNT 3
// uint8_t BOMBARDING_UNITS[BOMBARDING_UNITS_COUNT] = {CRUISERS, BATTLESHIPS, BS_DAMAGED};

bool canFighterLandHere[AIRS_COUNT];
bool canFighterLandIn1Move[AIRS_COUNT];
bool canBomberLandHere[AIRS_COUNT];
bool canBomberLandIn1Move[AIRS_COUNT];
bool canBomberLandIn2Moves[AIRS_COUNT];

uint8_t RANDOM_NUMBERS[65536] = {0};
u_short random_number_index = 0;

void initializeGameData() {
  generate_total_air_distance();
  generate_total_land_distance();
  generate_total_sea_distance();
  generate_landMoveAllDestination();
  generate_seaMoveAllDestination();
  generate_airMoveAllDestination();
  generate_within_x_moves();
  generate_random_numbers();
  // set RETREAT_OPTIONS to be an array of values from 0 to 255
  for (int i = 0; i < 255; i++) {
    RETREAT_OPTIONS[i] = i;
  }

  // json = serialize_game_data_to_json(&gameData);
  // write_json_to_file("game_data_0.json", json);
  // cJSON_Delete(json);
}

void load_game_data() {
  json = read_json_from_file("game_data.json");
  deserialize_game_data_from_json(&data, json);
  cJSON_Delete(json);
  refresh_cache();
}

void play_full_turn() {
  // clear printableGameStatus
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
  move_aa_guns();
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
    LAND_NAMES[land_idx] = LANDS[land_idx].name;
    LAND_VALUE[land_idx] = LANDS[land_idx].land_value;
    LAND_TO_LAND_COUNT[land_idx] = LANDS[land_idx].land_conn_count;
    for (int conn_idx = 0; conn_idx < LANDS[land_idx].land_conn_count; conn_idx++) {
      LAND_TO_LAND_CONN[land_idx][conn_idx] = LANDS[land_idx].connected_land_index[conn_idx];
    }

    for (int j = 0; j < AIRS_COUNT; j++) {
      if (land_idx != j) {
        LAND_DIST[land_idx][j] = INFINITY;
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
  for (int canal_idx = 0; canal_idx < CANAL_STATES; canal_idx++) {
    // Initialize the total_sea_distance array
    for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      for (int dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
        if (src_sea != dst_sea) {
          SEA_DIST[canal_idx][src_sea][dst_sea] = INFINITY;
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
        AIR_DIST[src_air][dst_air] = INFINITY;
      } // else {
        // total_air_distance[i][j] = 0;
      //}
    }
  }
  int air_index;
  int conn_idx;
  // Populate initial distances based on connected_sea_index and
  // connected_land_index
  for (int src_land = 0; src_land < LANDS_COUNT; src_land++) {
    AIR_NAMES[src_land] = LAND_NAMES[src_land];
    for (conn_idx = 0; conn_idx < LAND_TO_LAND_COUNT[src_land]; conn_idx++) {
      air_index = LAND_TO_LAND_CONN[src_land][conn_idx];
      AIR_CONNECTIONS[src_land][AIR_CONN_COUNT[src_land]] = air_index;
      AIR_CONN_COUNT[src_land]++;
      AIR_DIST[src_land][air_index] = 1;
      AIR_DIST[air_index][src_land] = 1;
    }
    for (conn_idx = 0; conn_idx < LAND_TO_SEA_COUNT[src_land]; conn_idx++) {
      air_index = LAND_TO_SEA_CONN[src_land][conn_idx] + LANDS_COUNT;
      AIR_CONNECTIONS[src_land][AIR_CONN_COUNT[src_land]] = air_index;
      AIR_CONN_COUNT[src_land]++;
      AIR_DIST[src_land][air_index] = 1;
      AIR_DIST[air_index][src_land] = 1;
    }
  }

  for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    int src_air = src_sea + LANDS_COUNT;
    AIR_NAMES[src_air] = SEA_NAMES[src_sea];
    for (conn_idx = 0; conn_idx < SEA_TO_LAND_COUNT[src_sea]; conn_idx++) {
      air_index = SEA_TO_LAND_CONN[src_sea][conn_idx];
      AIR_CONNECTIONS[src_air][AIR_CONN_COUNT[src_air]] = air_index;
      AIR_CONN_COUNT[src_air]++;
      AIR_DIST[src_air][air_index] = 1;
      AIR_DIST[air_index][src_air] = 1;
    }
    for (conn_idx = 0; conn_idx < SEA_TO_SEA_COUNT[src_sea]; conn_idx++) {
      air_index = SEA_TO_SEA_CONN[src_sea][conn_idx] + LANDS_COUNT;
      AIR_CONNECTIONS[src_air][AIR_CONN_COUNT[src_air]] = air_index;
      AIR_CONN_COUNT[src_air]++;
      AIR_DIST[src_air][air_index] = 1;
      AIR_DIST[air_index][src_air] = 1;
    }
  }

  // Floyd-Warshall algorithm to compute shortest paths
  for (air_index = 0; air_index < AIRS_COUNT; air_index++) {
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
  for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    SEA_TO_SEA_COUNT[src_sea] = SEAS[src_sea].sea_conn_count;
    for (int conn_idx = 0; conn_idx < SEAS[src_sea].sea_conn_count; conn_idx++) {
      SEA_TO_SEA_CONN[src_sea][conn_idx] = SEAS[src_sea].connected_sea_index[conn_idx];
    }
    SEA_TO_LAND_COUNT[src_sea] = SEAS[src_sea].land_conn_count;
    for (int conn_idx = 0; conn_idx < SEAS[src_sea].land_conn_count; conn_idx++) {
      SEA_TO_LAND_CONN[src_sea][conn_idx] = SEAS[src_sea].connected_land_index[conn_idx];
    }
    SEA_NAMES[src_sea] = SEAS[src_sea].name;
  }
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
void refresh_cache() {
  current_player_index = data.player_index;
  current_player_index_plus_one = current_player_index + 1;
  current_player = PLAYERS[current_player_index];
  current_player_name = current_player.name;
  current_player_color = current_player.color;
  current_player_is_human = current_player.is_human;
  printableGameStatus[0] = '\0';

  for (int player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    gamestate_player_to_mapstate_player[player_idx] =
        (current_player_index + player_idx) % PLAYERS_COUNT;
    mapstate_player_to_gamestate_player[gamestate_player_to_mapstate_player[player_idx]] =
        player_idx;
    is_allied[player_idx] =
        current_player.is_allied[gamestate_player_to_mapstate_player[player_idx]];
    enemies_count = 0;
    if (current_player.is_allied[player_idx] == false) {
      enemies[enemies_count] = player_idx;
      enemies_count++;
    }

    // OPTIONAL FOR AI PLAY current_player_is_human==true
    player_names[player_idx] = PLAYERS[gamestate_player_to_mapstate_player[player_idx]].name;
    player_colors[player_idx] = PLAYERS[gamestate_player_to_mapstate_player[player_idx]].color;
    money[player_idx] = data.money[player_idx];
    // END OPTIONAL FOR AI PLAY
  }
  uint8_t unit_count;
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    original_owner_idx[land_idx] =
        mapstate_player_to_gamestate_player[LANDS[land_idx].original_owner_index];
    owner_idx[land_idx] = data.land_state[land_idx].owner_idx;
    income_per_turn[owner_idx[land_idx]] += LANDS[land_idx].land_value;
    builds_left[land_idx] = data.builds_left[land_idx];
    factory_max[land_idx] = data.land_state[land_idx].factory_max;
    my_factory_locations[my_factory_count++] = land_idx;
    factory_hp[land_idx] = data.land_state[land_idx].factory_hp;
    bombard_max[land_idx] = data.land_state[land_idx].bombard_max;
    flagged_for_combat[land_idx] = data.flagged_for_combat[land_idx];
    units_air_ptr[land_idx][FIGHTERS] = (uint8_t*)data.land_state[land_idx].fighters;
    units_air_ptr[land_idx][BOMBERS_LAND_AIR] = (uint8_t*)data.land_state[land_idx].bombers;
    for (int unit_type = 0; unit_type < AIR_UNIT_TYPES_COUNT; unit_type++) {
      for (int cur_state = 0; cur_state < STATES_MOVE_SEA[unit_type]; cur_state++) {
        air_total[land_idx][unit_type] += units_sea_ptr[land_idx][unit_type][cur_state];
      }
    }
    units_land_ptr[land_idx][FIGHTERS] = (uint8_t*)data.land_state[land_idx].fighters;
    units_land_ptr[land_idx][BOMBERS_LAND_AIR] = (uint8_t*)data.land_state[land_idx].bombers;
    units_land_ptr[land_idx][INFANTRY] = (uint8_t*)data.land_state[land_idx].infantry;
    units_land_ptr[land_idx][ARTILLERY] = (uint8_t*)data.land_state[land_idx].artillery;
    units_land_ptr[land_idx][TANKS] = (uint8_t*)data.land_state[land_idx].tanks;
    units_land_ptr[land_idx][AA_GUNS] = (uint8_t*)data.land_state[land_idx].aa_guns;
    units_land_player_total[land_idx][0] = 0;
    for (int unit_type = 0; unit_type < LAND_UNIT_TYPES; unit_type++) {
      for (int cur_state = 0; cur_state < STATES_MOVE_LAND[unit_type]; cur_state++) {
        units_land_player_total[land_idx][0] += units_land_ptr[land_idx][unit_type][cur_state];
      }
    }
    enemy_units_count[land_idx] = 0;
    units_land_grand_total[land_idx] = units_land_player_total[land_idx][0];
    for (int player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
      units_land_player_total[land_idx][player_idx] = 0;
      for (int land_unit_type = 0; land_unit_type < LAND_UNIT_TYPES; land_unit_type++) {
        unit_count = data.land_state[land_idx].other_units[player_idx][land_unit_type];
        other_land_units[land_idx][player_idx][land_unit_type] = unit_count;
        units_land_player_total[land_idx][player_idx] += unit_count;
      }
      if (!is_allied[player_idx]) {
        enemy_units_count[land_idx] += units_land_player_total[land_idx][player_idx];
      }
      units_land_grand_total[land_idx] += units_land_player_total[land_idx][player_idx];
    }
  }
  for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    uint8_t air_idx = sea_idx + LANDS_COUNT;
    flagged_for_combat[air_idx] = data.flagged_for_combat[air_idx];
    units_air_ptr[air_idx][FIGHTERS] = (uint8_t*)data.units_sea[sea_idx].fighters;
    units_air_ptr[air_idx][BOMBERS_LAND_AIR] = (uint8_t*)data.units_sea[sea_idx].bombers;
    for (int unit_type = 0; unit_type < AIR_UNIT_TYPES_COUNT; unit_type++) {
      for (int cur_state = 0; cur_state < STATES_MOVE_SEA[unit_type]; cur_state++) {
        air_total[air_idx][unit_type] += units_sea_ptr[air_idx][unit_type][cur_state];
      }
    }
    units_sea_ptr[sea_idx][FIGHTERS] = (uint8_t*)data.units_sea[sea_idx].fighters;
    units_sea_ptr[sea_idx][TRANS_EMPTY] = (uint8_t*)data.units_sea[sea_idx].trans_empty;
    units_sea_ptr[sea_idx][TRANS_1I] = (uint8_t*)data.units_sea[sea_idx].trans_1i;
    units_sea_ptr[sea_idx][TRANS_1A] = (uint8_t*)data.units_sea[sea_idx].trans_1a;
    units_sea_ptr[sea_idx][TRANS_1T] = (uint8_t*)data.units_sea[sea_idx].trans_1t;
    units_sea_ptr[sea_idx][TRANS_2I] = (uint8_t*)data.units_sea[sea_idx].trans_2i;
    units_sea_ptr[sea_idx][TRANS_1I_1A] = (uint8_t*)data.units_sea[sea_idx].trans_1i_1a;
    units_sea_ptr[sea_idx][TRANS_1I_1T] = (uint8_t*)data.units_sea[sea_idx].trans_1i_1t;
    units_sea_ptr[sea_idx][SUBMARINES] = (uint8_t*)data.units_sea[sea_idx].submarines;
    units_sea_ptr[sea_idx][DESTROYERS] = (uint8_t*)data.units_sea[sea_idx].destroyers;
    units_sea_ptr[sea_idx][CARRIERS] = (uint8_t*)data.units_sea[sea_idx].carriers;
    allied_carriers[sea_idx] =
        data.units_sea[sea_idx].carriers[0] + data.units_sea[sea_idx].carriers[1];
    units_sea_ptr[sea_idx][CRUISERS] = (uint8_t*)data.units_sea[sea_idx].cruisers;
    units_sea_ptr[sea_idx][BATTLESHIPS] = (uint8_t*)data.units_sea[sea_idx].battleships;
    units_sea_ptr[sea_idx][BS_DAMAGED] = (uint8_t*)data.units_sea[sea_idx].bs_damaged;
    units_sea_ptr[sea_idx][BOMBERS_SEA] = (uint8_t*)data.units_sea[sea_idx].bombers;
    units_sea_player_total[sea_idx][0] = 0;
    for (int unit_type = 0; unit_type < SEA_UNIT_TYPES; unit_type++) {
      for (int cur_state = 0; cur_state < STATES_MOVE_SEA[unit_type]; cur_state++) {
        units_sea_player_total[sea_idx][0] += units_sea_ptr[sea_idx][unit_type][cur_state];
      }
    }
    transports_with_large_cargo_space[sea_idx] =
        memchr(&units_sea_ptr[sea_idx][TRANS_EMPTY][0], 0,
               (TRANS_EMPTY_STATES + TRANS_1I_STATES) * sizeof(uint8_t)) != NULL;
    // memcmp(&units_sea_ptr[sea_idx][TRANS_EMPTY][0], zeros_transports_with_large_cargo_space,
    //        (TRANS_EMPTY_STATES + TRANS_1I_STATES) * sizeof(uint8_t)) != 0;
    transports_with_small_cargo_space[sea_idx] =
        transports_with_large_cargo_space[sea_idx] ||
        memchr(&units_sea_ptr[sea_idx][TRANS_1A][0], 0,
               (TRANS_1A_STATES + TRANS_1T_STATES) * sizeof(uint8_t)) != NULL;
    // memcmp(&units_sea_ptr[sea_idx][TRANS_EMPTY][0], zeros_transports_with_small_cargo_space,
    //        (TRANS_EMPTY_STATES + TRANS_1I_STATES + TRANS_1A_STATES + TRANS_1T_STATES) *
    //            sizeof(uint8_t)) != 0;

    enemy_units_count[air_idx] = 0;
    units_sea_grand_total[sea_idx] = units_sea_player_total[sea_idx][0];
    for (int player_idx = 0; player_idx < PLAYERS_COUNT - 1; player_idx++) {
      for (int sea_unit_type = 0; sea_unit_type < SEA_UNIT_TYPES; sea_unit_type++) {
        unit_count = data.units_sea[sea_idx].other_units[player_idx][sea_unit_type];
        other_sea_units[sea_idx][player_idx][sea_unit_type] = unit_count;
        units_sea_player_total[sea_idx][player_idx] += unit_count;
      }
      total_enemy_subs[sea_idx] +=
          other_sea_units[sea_idx][player_idx][SUBMARINES] * !is_allied[player_idx];
      allied_carriers[sea_idx] +=
          other_sea_units[sea_idx][player_idx][CARRIERS] * is_allied[player_idx];
      if (!is_allied[player_idx]) {
        enemy_units_count[air_idx] += units_sea_player_total[sea_idx][player_idx];
      }
      units_sea_grand_total[sea_idx] += units_sea_player_total[sea_idx][player_idx];
    }
  }
  canal_state = 0;
  for (int canal_idx = 0; canal_idx < CANALS_COUNT; canal_idx++) {
    if (is_allied[owner_idx[CANALS[canal_idx].lands[0]]] &&
        is_allied[owner_idx[CANALS[canal_idx].lands[1]]]) {
      canal_state += 1 << canal_idx;
    }
  }
  memcpy(sea_dist, SEA_DIST[canal_state], sizeof(sea_dist));
  memcpy(sea_path1, SEA_PATH[0][canal_state], sizeof(sea_path1));
  memcpy(sea_path2, SEA_PATH[1][canal_state], sizeof(sea_path2));
  memcpy(sea_path_alt1, SEA_PATH_ALT[canal_state], sizeof(sea_path_alt1));
}
void refresh_canBomberLandHere() {
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    // is allied owned and not recently conquered?
    canBomberLandHere[land_idx] = (is_allied[owner_idx[land_idx]] && !flagged_for_combat[land_idx]);
  }
}
void refresh_canBomberLandIn1Move() {
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    canBomberLandIn1Move[land_idx] = false;
    for (int conn_idx = 0; conn_idx < LANDS[land_idx].land_conn_count; conn_idx++) {
      if (canBomberLandHere[LANDS[land_idx].connected_land_index[conn_idx]]) {
        canBomberLandIn1Move[land_idx] = true;
        break;
      }
    }
  }
  for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    canBomberLandIn1Move[LANDS_COUNT + sea_idx] = false;
    for (int conn_idx = 0; conn_idx < SEAS[sea_idx].land_conn_count; conn_idx++) {
      if (canBomberLandHere[SEAS[sea_idx].connected_land_index[conn_idx]]) {
        canBomberLandIn1Move[LANDS_COUNT + sea_idx] = true;
        break;
      }
    }
  }
}
void refresh_canBomberLandIn2Moves() {
  for (int air_idx = 0; air_idx < AIRS_COUNT; air_idx++) {
    canBomberLandIn2Moves[air_idx] = false;
    for (int conn_idx = 0; conn_idx < AIR_CONN_COUNT[air_idx]; conn_idx++) {
      if (canBomberLandIn1Move[AIR_CONNECTIONS[air_idx][conn_idx]]) {
        canBomberLandIn2Moves[air_idx] = true;
        break;
      }
    }
  }
}
void refresh_canFighterLandHere() {
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    // is allied owned and not recently conquered?
    canFighterLandHere[land_idx] =
        (is_allied[owner_idx[land_idx]] && !flagged_for_combat[land_idx]);
    // check for possiblity to build carrier under fighter
    if (factory_max[land_idx] > 0 && owner_idx[land_idx] == current_player_index) {
      for (int conn_idx = 0; conn_idx < LAND_TO_SEA_COUNT[land_idx]; conn_idx++) {
        canFighterLandHere[LANDS_COUNT + LAND_TO_SEA_CONN[land_idx][conn_idx]] = true;
      }
    }
  }
  for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    if (allied_carriers[sea_idx] > 0) {
      canFighterLandHere[sea_idx] = true;
      // if player owns these carriers, then landing area is 2 spaces away
      if (units_sea_ptr[sea_idx][CARRIERS][CARRIER_MOVES_MAX] > 0) {
        for (int conn_idx = 0; conn_idx < SEA_TO_SEA_COUNT[sea_idx]; conn_idx++) {
          uint8_t connected_sea1 = SEA_TO_SEA_CONN[sea_idx][conn_idx];
          canFighterLandHere[LANDS_COUNT + connected_sea1] = true;
          for (int conn2_idx = 0; conn2_idx < SEA_TO_SEA_COUNT[connected_sea1]; conn2_idx++) {
            canFighterLandHere[LANDS_COUNT + SEA_TO_SEA_CONN[connected_sea1][conn2_idx]] = true;
          }
        }
      }
    }
  }
}
void refresh_canFighterLandIn1Move() {
  for (int air_idx = 0; air_idx < AIRS_COUNT; air_idx++) {
    canFighterLandIn1Move[air_idx] = false;
    for (int conn_idx = 0; conn_idx < AIR_CONN_COUNT[air_idx]; conn_idx++) {
      if (canFighterLandHere[AIR_CONNECTIONS[air_idx][conn_idx]]) {
        canFighterLandIn1Move[air_idx] = true;
        break;
      }
    }
  }
}
void refresh_canFighterLandHere_final() {
  for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    // if (allied_carriers[sea_idx] > 0) {
    canFighterLandHere[sea_idx] = allied_carriers[sea_idx] > 0;
    //}
  }
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    // is allied owned and not recently conquered?
    // canFighterLandHere[land_idx] =
    //    (is_allied[owner_idx[land_idx]] && !flagged_for_combat[land_idx]);
    // check for possiblity to build carrier under fighter
    if (factory_max[land_idx] > 0 && owner_idx[land_idx] == current_player_index) {
      for (int conn_idx = 0; conn_idx < LAND_TO_SEA_COUNT[land_idx]; conn_idx++) {
        canFighterLandHere[LANDS_COUNT + LAND_TO_SEA_CONN[land_idx][conn_idx]] = true;
      }
    }
  }
}
void setPrintableStatus() {
  strcat(printableGameStatus, current_player_color);
  strcat(printableGameStatus, current_player_name);
  strcat(printableGameStatus, "\033[0m");
  strcat(printableGameStatus, " with money ");
  sprintf(threeCharStr, "%d", current_player_money);
  strcat(printableGameStatus, threeCharStr);
  strcat(printableGameStatus, "\n");
  setPrintableStatusLands();
  setPrintableStatusSeas();
}
void setPrintableStatusLands() {
  for (int land_index = 0; land_index < LANDS_COUNT; land_index++) {
    //    LandState land_state = gameData.land_state[i];
    strcat(printableGameStatus, player_colors[owner_idx[land_index]]);
    sprintf(threeCharStr, "%d ", land_index);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, LAND_NAMES[land_index]);
    strcat(printableGameStatus, ": ");
    strcat(printableGameStatus, player_names[owner_idx[land_index]]);
    strcat(printableGameStatus, " ");
    sprintf(threeCharStr, "%d", builds_left[land_index]);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, "/");
    sprintf(threeCharStr, "%d", factory_hp[land_index]);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, "/");
    sprintf(threeCharStr, "%d", factory_max[land_index]);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, " Combat: ");
    if (flagged_for_combat[land_index]) {
      strcat(printableGameStatus, "true\n");
    } else {
      strcat(printableGameStatus, "false\n");
    }
    if (units_land_grand_total[land_index] == 0) {
      strcat(printableGameStatus, "\033[0m");
      continue;
    }
    uint8_t land_unit_count;
    strcat(printableGameStatus, "             |Tot| 0| 1| 2| 3| 4| 5| 6|\n");
    if (units_land_player_total[land_index][0] > 0) {
      strcat(printableGameStatus, current_player_color);
      for (int land_unit_idx = 0; land_unit_idx < LAND_UNIT_TYPES; land_unit_idx++) {
        bool unit_type_present = false;
        for (int cur_state = 0; cur_state < STATES_MOVE_LAND[land_unit_idx]; cur_state++) {
          if (units_land_ptr[land_index][land_unit_idx][cur_state] > 0) {
            unit_type_present = true;
            break;
          }
        }
        if (unit_type_present) {
          for (int cur_state = 0; cur_state < STATES_MOVE_LAND[land_unit_idx]; cur_state++) {
            strcat(printableGameStatus, current_player_name);
            strcat(printableGameStatus, NAMES_UNIT_LAND[land_unit_idx]);
            for (int cur_state = 0; cur_state < STATES_MOVE_LAND[land_unit_idx]; cur_state++) {
              sprintf(threeCharStr, "%3d", units_land_ptr[land_index][land_unit_idx][cur_state]);
              strcat(printableGameStatus, threeCharStr);
            }
            strcat(printableGameStatus, "\n");
          }
        }
      }
    }
    strcat(printableGameStatus, "\033[0m");
    for (int player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      if (units_land_player_total[land_index][player_idx] > 0) {
        strcat(printableGameStatus, player_colors[player_idx]);
        for (int unit_idx = 0; unit_idx < LAND_UNIT_TYPES; unit_idx++) {
          uint8_t unit_count = other_land_units[land_index][player_idx][unit_idx];
          if (unit_count > 0) {
            strcat(printableGameStatus, player_names[player_idx]);
            strcat(printableGameStatus, NAMES_UNIT_LAND[unit_idx]);
            sprintf(threeCharStr, "%3d", unit_count);
            strcat(printableGameStatus, threeCharStr);
            strcat(printableGameStatus, "\n");
          }
        }
        strcat(printableGameStatus, "\033[0m");
      }
    }
  }
}
void setPrintableStatusSeas() {
  for (int sea_index = 0; sea_index < SEAS_COUNT; sea_index++) {
    if (units_sea_grand_total[sea_index] == 0) {
      continue;
    }
    UnitsSea units_sea = data.units_sea[sea_index];
    sprintf(threeCharStr, "%d ", LANDS_COUNT + sea_index);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, SEA_NAMES[sea_index]);
    strcat(printableGameStatus, "             |Tot| 0| 1| 2| 3| 4| 5| 6|\n");
    strcat(printableGameStatus, current_player_color);
    if (units_sea_player_total[sea_index][0] > 0) {
      for (int sea_unit_idx = 0; sea_unit_idx < SEA_UNIT_TYPES; sea_unit_idx++) {
        bool units_present = false;
        for (int cur_state = 0; cur_state < STATES_MOVE_SEA[sea_unit_idx]; cur_state++) {
          if (units_sea_ptr[sea_index][sea_unit_idx][cur_state] > 0) {
            units_present = true;
            break;
          }
        }
        if (units_present) {
          for (int cur_state = 0; cur_state < STATES_MOVE_SEA[sea_unit_idx]; cur_state++) {
            strcat(printableGameStatus, current_player_name);
            strcat(printableGameStatus, NAMES_UNIT_SEA[sea_unit_idx]);
            for (int cur_state = 0; cur_state < STATES_MOVE_SEA[sea_unit_idx]; cur_state++) {
              sprintf(threeCharStr, "%3d", units_sea_ptr[sea_index][sea_unit_idx][cur_state]);
              strcat(printableGameStatus, threeCharStr);
            }
            strcat(printableGameStatus, "\n");
          }
        }
      }
      strcat(printableGameStatus, "\033[0m");
      for (int player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
        strcat(printableGameStatus, player_colors[player_idx]);
        for (int unit_idx = 0; unit_idx < SEA_UNIT_TYPES; unit_idx++) {
          uint8_t unit_count = other_sea_units[sea_index][player_idx][unit_idx];
          if (unit_count > 0) {
            strcat(printableGameStatus, player_names[player_idx]);
            strcat(printableGameStatus, NAMES_UNIT_SEA[unit_idx]);
            sprintf(threeCharStr, "%3d", unit_count);
            strcat(printableGameStatus, threeCharStr);
            strcat(printableGameStatus, "\n");
          }
        }
        strcat(printableGameStatus, "\033[0m");
      }
    }
  }
}
uint8_t getUserInput(uint8_t* valid_moves, int valid_moves_count) {
  char buffer[4]; // Buffer to hold input string (3 digits + null terminator)
  int user_input;

  while (true) {
    // 0-valid_moves_count
    printf("Enter a number between 0 and 255: ");
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
      if (sscanf(buffer, "%d", &user_input) == 1 && user_input >= 0) {
        return (uint8_t)user_input;
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
  for (int land_idx = 0; land_idx < LAND_TO_LAND_COUNT[src_land]; land_idx++) {
    uint8_t shared_dst = LAND_TO_LAND_CONN[src_land][land_idx];
    for (int i = 0; i < source_territories_count[shared_dst]; i++) {
      uint8_t src_air = source_territories[shared_dst][i];
      if (has_checked_territory[src_air]) {
        continue;
      }
      has_checked_territory[src_air] = true;
      if (skipped_airs[src_air][dst_air]) {
        return;
      }
    }
  }
  for (int sea_idx = 0; sea_idx < LAND_TO_SEA_COUNT[src_land]; sea_idx++) {
    uint8_t dst_sea = LAND_TO_SEA_CONN[src_land][sea_idx];
    uint8_t shared_dst = dst_sea + LANDS_COUNT;
    for (int i = 0; i < source_territories_count[shared_dst]; i++) {
      uint8_t src_air = source_territories[shared_dst][i];
      if (has_checked_territory[src_air]) {
        continue;
      }
      has_checked_territory[src_air] = true;
      if (skipped_airs[src_air][dst_air]) {
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
  for (int land_idx = 0; land_idx < LANDS_WITHIN_2_MOVES_COUNT[src_land]; land_idx++) {
    uint8_t shared_dst = LANDS_WITHIN_2_MOVES[src_land][land_idx];
    for (int i = 0; i < source_territories_count[shared_dst]; i++) {
      uint8_t src_air = source_territories[shared_dst][i];
      if (has_checked_territory[src_air]) {
        continue;
      }
      has_checked_territory[src_air] = true;
      if (skipped_airs[src_air][dst_air] && !is_land_path_blocked(src_land, shared_dst)) {

        return;
      }
    }
  }
  for (int sea_idx = 0; sea_idx < LOAD_WITHIN_2_MOVES_COUNT[src_land]; sea_idx++) {
    uint8_t dst_sea = LOAD_WITHIN_2_MOVES[src_land][sea_idx];
    uint8_t shared_dst = dst_sea + LANDS_COUNT;
    for (int i = 0; i < source_territories_count[shared_dst]; i++) {
      uint8_t src_air = source_territories[shared_dst][i];
      if (has_checked_territory[src_air]) {
        continue;
      }
      has_checked_territory[src_air] = true;
      if (skipped_airs[src_air][dst_air] && !is_land_path_blocked(src_land, shared_dst)) {
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
  for (int sea_idx = 0; sea_idx < SEAS_WITHIN_1_MOVE_COUNT[canal_state][src_sea]; sea_idx++) {
    uint8_t shared_dst = SEAS_WITHIN_1_MOVE[canal_state][src_sea][sea_idx];
    for (int i = 0; i < source_territories_count[shared_dst]; i++) {
      uint8_t src_air = source_territories[shared_dst][i];
      if (has_checked_air[src_air]) {
        continue;
      }
      has_checked_air[src_air] = true;
      if (skipped_airs[src_air][dst_sea + LANDS_COUNT]) {
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
  for (int sea_idx = 0; sea_idx < SEAS_WITHIN_2_MOVES_COUNT[canal_state][src_sea]; sea_idx++) {
    uint8_t shared_dst = SEAS_WITHIN_2_MOVES[canal_state][src_sea][sea_idx];
    for (int i = 0; i < source_territories_count[shared_dst]; i++) {
      uint8_t src_air = source_territories[shared_dst][i];
      if (has_checked_territory[src_air]) {
        continue;
      }
      has_checked_territory[src_air] = true;
      if (skipped_airs[src_air][dst_sea + LANDS_COUNT]) {
        if (!is_sea_path_blocked(src_sea, dst_sea))
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
  for (int sea_idx = 0; sea_idx < SEAS_WITHIN_2_MOVES_COUNT[canal_state][src_sea]; sea_idx++) {
    uint8_t shared_dst = SEAS_WITHIN_2_MOVES[canal_state][src_sea][sea_idx];
    for (int i = 0; i < source_territories_count[shared_dst]; i++) {
      uint8_t src_air = source_territories[shared_dst][i];
      if (has_checked_territory[src_air]) {
        continue;
      }
      has_checked_territory[src_air] = true;
      if (skipped_airs[src_air][dst_sea + LANDS_COUNT]) {
        if (!is_sub_path_blocked(src_sea, dst_sea))
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
  for (int land_idx = 0; land_idx < AIR_WITHIN_X_MOVES_COUNT[moves][src_air]; land_idx++) {
    uint8_t shared_dst = AIR_WITHIN_X_MOVES[moves][src_air][land_idx];
    for (int i = 0; i < source_territories_count[shared_dst]; i++) {
      uint8_t src_air = source_territories[shared_dst][i];
      if (has_checked_territory[src_air]) {
        continue;
      }
      has_checked_territory[src_air] = true;
      if (skipped_airs[src_air][dst_air]) {
        return;
      }
    }
  }
  valid_moves[*valid_moves_count] = dst_air;
  (*valid_moves_count)++;
}

bool is_land_path_blocked(uint8_t src_land, uint8_t dst_land) {
  uint8_t nextLandMovement = land_path1[src_land][dst_land];
  if (factory_max[nextLandMovement] > 0) {
    return true;
  }
  for (int enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
    if (units_land_player_total[nextLandMovement][enemies[enemy_idx]] > 0) {
      return true;
    }
  }
  nextLandMovement = LAND_PATH_ALT[src_land][dst_land];
  if (factory_max[nextLandMovement] > 0) {
    return true;
  }
  for (int enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
    if (units_land_player_total[nextLandMovement][enemies[enemy_idx]] > 0) {
      return true;
    }
  }
  return false;
}

void clear_move_history() {
  memset(skipped_airs, 0, sizeof(skipped_airs));
  // memset(source_territories, 0, sizeof(source_territories));
  memset(source_territories_count, 0, sizeof(source_territories_count));
}

uint8_t get_user_purchase_input(uint8_t src_air, uint8_t* valid_purchases,
                                uint8_t valid_purchases_count) {
  uint8_t user_input;
  if (valid_purchases_count == 1) {
    return valid_purchases[0];
  }
  if (current_player_is_human) {
    setPrintableStatus();
    strcat(printableGameStatus, "Purchasing ");
    if (src_air < LANDS_COUNT) {
      strcat(printableGameStatus, NAMES_UNIT_LAND[src_air]);
      for (int i = 1; i < valid_purchases_count; i++) {
        sprintf(threeCharStr, "%d %s ", valid_purchases[i],
                NAMES_UNIT_LAND[valid_purchases[i] - 1]);
        strcat(printableGameStatus, threeCharStr);
      }
    } else {
      strcat(printableGameStatus, NAMES_UNIT_SEA[src_air - LANDS_COUNT]);
      for (int i = 1; i < valid_purchases_count; i++) {
        sprintf(threeCharStr, "%d %s ", valid_purchases[i], NAMES_UNIT_SEA[valid_purchases[i] - 1]);
        strcat(printableGameStatus, threeCharStr);
      }
    }
    strcat(printableGameStatus, " Valid Purchases: ");
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
  if (current_player_is_human) {
    setPrintableStatus();
    strcat(printableGameStatus, "Moving ");
    if (src_air < LANDS_COUNT) {
      strcat(printableGameStatus, NAMES_UNIT_LAND[unit_type]);
      strcat(printableGameStatus, " From: ");
      strcat(printableGameStatus, LAND_NAMES[src_air]);
    } else {
      strcat(printableGameStatus, NAMES_UNIT_SEA[unit_type]);
      strcat(printableGameStatus, " From: ");
      strcat(printableGameStatus, SEA_NAMES[src_air - LANDS_COUNT]);
    }
    strcat(printableGameStatus, " Valid Moves: ");
    for (int i = 0; i < valid_moves_count; i++) {
      sprintf(threeCharStr, "%d ", valid_moves[i]);
      strcat(printableGameStatus, threeCharStr);
    }
    printf("%s\n", printableGameStatus);
    return getUserInput(valid_moves, valid_moves_count);
  }
  return getAIInput(valid_moves, valid_moves_count);
}

void update_move_history(uint8_t user_input, uint8_t src_air, uint8_t* valid_moves,
                         uint8_t valid_moves_count) {
  source_territories[user_input][source_territories_count[user_input]] = src_air;
  source_territories_count[user_input]++;
  for (int i = valid_moves_count - 1; i >= 0; i--) {
    if (valid_moves[i] == user_input) {
      break;
    } else {
      skipped_airs[src_air][valid_moves[i]] = true;
      valid_moves_count--;
    }
  }
}

bool load_transport(uint8_t unit_type, uint8_t src_land, uint8_t dst_sea) {
  uint8_t trans_type = TRANS_1T;
  if (UNIT_WEIGHTS[unit_type] > 2) {
    trans_type = TRANS_1I;
  }
  while (trans_type >= TRANS_EMPTY) {
    uint8_t trans_state = STATES_MOVE_SEA[trans_type] - STATES_STAGING[trans_type];
    while (trans_state >= STATES_UNLOADING[trans_type]) {
      if (units_sea_ptr[dst_sea][trans_type][trans_state] > 0) {
        units_sea_ptr[dst_sea][LOAD_UNIT_TYPE[unit_type][trans_type]][trans_state]++;
        units_sea_player_total[dst_sea][0]++;
        units_sea_grand_total[dst_sea]++;
        units_sea_ptr[dst_sea][trans_type][trans_state]--;
        units_land_player_total[src_land][0]--;
        units_land_grand_total[src_land]--;
        units_land_ptr[src_land][unit_type][trans_type]--;
        transports_with_large_cargo_space[dst_sea] =
            memchr(&units_sea_ptr[dst_sea][TRANS_EMPTY][0], 0,
                   (TRANS_EMPTY_STATES + TRANS_1I_STATES) * sizeof(uint8_t)) != NULL;
        // memcmp(&units_sea_ptr[sea_idx][TRANS_EMPTY][0], zeros_transports_with_large_cargo_space,
        //        (TRANS_EMPTY_STATES + TRANS_1I_STATES) * sizeof(uint8_t)) != 0;
        transports_with_small_cargo_space[dst_sea] =
            transports_with_large_cargo_space[dst_sea] ||
            memchr(&units_sea_ptr[dst_sea][TRANS_1A][0], 0,
                   (TRANS_1A_STATES + TRANS_1T_STATES) * sizeof(uint8_t)) != NULL;
        // memcmp(&units_sea_ptr[sea_idx][TRANS_EMPTY][0], zeros_transports_with_small_cargo_space,
        //        (TRANS_EMPTY_STATES + TRANS_1I_STATES + TRANS_1A_STATES + TRANS_1T_STATES) *
        //            sizeof(uint8_t)) != 0;
        return true;
      }
      trans_state--;
    }
    trans_type--;
  }
  printf("Error: Failed to load tank onto transport\n");
  return false;
}

bool is_sea_path_blocked(uint8_t src_sea, uint8_t dst_sea) {
  uint8_t nextSeaMovement = sea_path1[src_sea][dst_sea];
  for (int enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
    if (units_sea_blockade_total[nextSeaMovement][enemies[enemy_idx]] >
        0) { // TODO optimize blockade
      return true;
    }
  }
  nextSeaMovement = sea_path_alt1[src_sea][dst_sea];
  for (int enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
    if (units_sea_blockade_total[nextSeaMovement][enemies[enemy_idx]] > 0) {
      return true;
    }
  }
  return false;
}

bool is_sub_path_blocked(uint8_t src_sea, uint8_t dst_sea) {
  uint8_t nextSeaMovement = sea_path1[src_sea][dst_sea];
  for (int enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
    if (other_sea_units[nextSeaMovement][enemies[enemy_idx]][DESTROYERS] >
        0) { // TODO optimize destroyers
      return true;
    }
  }
  nextSeaMovement = sea_path_alt1[src_sea][dst_sea];
  for (int enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
    if (other_sea_units[nextSeaMovement][enemies[enemy_idx]][DESTROYERS] > 0) {
      return true;
    }
  }
  return false;
}

void add_valid_land_moves(uint8_t* valid_moves, uint8_t* valid_moves_count, uint8_t src_land,
                          uint8_t moves_remaining, uint8_t unit_type) {
  if (moves_remaining == 2) {
    // check for moving from land to land (two moves away)
    for (int land_idx = 0; land_idx < LANDS_WITHIN_2_MOVES_COUNT[src_land]; land_idx++) {
      uint8_t dst_land = LANDS_WITHIN_2_MOVES[src_land][land_idx];
      if (LAND_DIST[src_land][dst_land] == 1) {
        add_valid_land_move_if_history_allows_2(valid_moves, valid_moves_count, dst_land, src_land);
        continue;
      }
      if (is_land_path_blocked(src_land, dst_land)) {
        continue;
      }
      add_valid_land_move_if_history_allows_2(valid_moves, valid_moves_count, dst_land, src_land);
    }
    // check for moving from land to sea (two moves away)
    for (int sea_idx = 0; sea_idx < LOAD_WITHIN_2_MOVES_COUNT[src_land]; sea_idx++) {
      uint8_t dst_sea = LOAD_WITHIN_2_MOVES[src_land][sea_idx];
      if (!transports_with_large_cargo_space[dst_sea]) { // assume large, only tanks move 2
        continue;
      }
      uint8_t dst_air = dst_sea + LANDS_COUNT;
      if (LAND_DIST[src_land][dst_air] == 1) {
        add_valid_land_move_if_history_allows_2(valid_moves, valid_moves_count, dst_air, src_land);
        continue;
      }
      if (is_land_path_blocked(src_land, dst_air)) {
        continue;
      }
      add_valid_land_move_if_history_allows_2(valid_moves, valid_moves_count, dst_air, src_land);
    }
  } else {
    // check for moving from land to land (one move away)
    for (int land_idx = 0; land_idx < LANDS_WITHIN_1_MOVE_COUNT[src_land]; land_idx++) {
      uint8_t dst_land = LANDS_WITHIN_1_MOVE[src_land][land_idx];
      add_valid_land_move_if_history_allows_1(valid_moves, valid_moves_count, dst_land, src_land);
    }
    // check for moving from land to sea (one move away)
    if (UNIT_WEIGHTS[unit_type] > 5) {
      return;
    }
    for (int sea_idx = 0; sea_idx < LAND_TO_SEA_COUNT[src_land]; sea_idx++) {
      uint8_t dst_sea = LAND_TO_SEA_CONN[src_land][sea_idx];
      if (!transports_with_small_cargo_space[dst_sea]) {
        continue;
      }
      if (UNIT_WEIGHTS[unit_type] > 2 && !transports_with_large_cargo_space[dst_sea]) {
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
    for (uint8_t sea_idx = 0; sea_idx < SEAS_WITHIN_2_MOVES_COUNT[canal_state][src_sea];
         sea_idx++) {
      uint8_t dst_sea =
          SEAS_WITHIN_2_MOVES[canal_state][src_sea][sea_idx]; // TODO optimize canal_state
      if (is_sea_path_blocked(src_sea, dst_sea)) {
        continue;
      }
      add_valid_sea_move_if_history_allows_2(valid_moves, valid_moves_count, dst_sea, src_sea);
    }
  } else {
    // check for moving from sea to sea (one move away)
    for (uint8_t sea_idx = 0; sea_idx < SEAS_WITHIN_1_MOVE_COUNT[canal_state][src_sea]; sea_idx++) {
      uint8_t dst_sea = SEAS_WITHIN_1_MOVE[canal_state][src_sea][sea_idx];
      add_valid_sea_move_if_history_allows_1(valid_moves, valid_moves_count, dst_sea, src_sea);
    }
  }
}

void add_valid_sub_moves(uint8_t* valid_moves, uint8_t* valid_moves_count, uint8_t src_sea,
                         uint8_t moves_remaining) {
  if (moves_remaining == 2) {
    // check for moving from sea to sea (two moves away)
    for (uint8_t sea_idx = 0; sea_idx < SEAS_WITHIN_2_MOVES_COUNT[canal_state][src_sea];
         sea_idx++) {
      uint8_t dst_sea =
          SEAS_WITHIN_2_MOVES[canal_state][src_sea][sea_idx]; // TODO optimize canal_state
      if (is_sub_path_blocked(src_sea, dst_sea)) {
        continue;
      }
      add_valid_sub_move_if_history_allows_2(valid_moves, valid_moves_count, dst_sea, src_sea);
    }
  } else {
    // check for moving from sea to sea (one move away)
    for (uint8_t sea_idx = 0; sea_idx < SEAS_WITHIN_1_MOVE_COUNT[canal_state][src_sea]; sea_idx++) {
      uint8_t dst_sea = SEAS_WITHIN_1_MOVE[canal_state][src_sea][sea_idx];
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
      uint8_t valid_moves[AIRS_COUNT];
      valid_moves[0] = src_sea;
      uint8_t valid_moves_count = 1;
      add_valid_sea_moves(valid_moves, &valid_moves_count, src_sea, 2);
      while (units_sea_ptr[src_sea][unit_type][staging_state] > 0) {
        uint8_t src_air = src_sea + LANDS_COUNT;
        uint8_t dst_air = get_user_move_input(unit_type, src_air, valid_moves, valid_moves_count);
        update_move_history(dst_air, src_sea, valid_moves, valid_moves_count);
        if (src_air == dst_air) {
          units_sea_ptr[src_sea][unit_type][done_staging]++;
          units_sea_ptr[src_sea][unit_type][staging_state]--;
          continue;
        }
        uint8_t dst_sea = dst_air - LANDS_COUNT;
        uint8_t sea_distance = sea_dist[src_sea][dst_air];
        if (enemy_units_count[dst_air] > 0) {
          for (uint8_t enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
            if (units_sea_blockade_total[dst_sea][enemies[enemy_idx]] > 0) {
              flagged_for_combat[dst_air] = true;
              sea_distance = MAX_MOVE_SEA[unit_type];
              break;
            }
          }
        }
        units_sea_ptr[dst_sea][unit_type][done_staging - sea_distance]++;
        units_sea_player_total[dst_sea][0]++;
        units_sea_grand_total[dst_sea]++;
        units_sea_ptr[dst_sea][unit_type][staging_state]--;
        units_sea_player_total[src_sea][0]--;
        units_sea_grand_total[src_sea]--;
      }
    }
  }
}
void move_fighter_units() {
  // check if any fighters have full moves remaining
  uint8_t* ptr = &units_air_ptr[0][FIGHTERS][FIGHTER_MOVES_MAX];
  if (memchr(ptr, 1, AIRS_COUNT * sizeof(uint8_t)) == NULL) {
    return;
  }
  clear_move_history();
  refresh_canFighterLandHere();
  refresh_canFighterLandIn1Move();
  for (int src_air = 0; src_air < AIRS_COUNT; src_air++) {
    uint8_t valid_moves[AIRS_COUNT];
    valid_moves[0] = src_air;
    uint8_t valid_moves_count = 1;
    add_valid_fighter_moves(valid_moves, &valid_moves_count, src_air, FIGHTER_MOVES_MAX);
    while (units_air_ptr[src_air][FIGHTERS][FIGHTER_MOVES_MAX] > 0) {
      uint8_t dst_air = get_user_move_input(FIGHTERS, src_air, valid_moves, valid_moves_count);
      update_move_history(dst_air, src_air, valid_moves, valid_moves_count);
      if (src_air == dst_air) {
        units_land_ptr[src_air][FIGHTERS][0]++;
        units_land_ptr[src_air][FIGHTERS][FIGHTER_MOVES_MAX]--;
        continue;
      }
      if (dst_air < LANDS_COUNT) {
        if (!is_allied[owner_idx[dst_air]]) {
          flagged_for_combat[dst_air] =
              true; // assuming enemy units are present based on valid moves
        }
      } else {
        uint8_t dst_sea = dst_air - LANDS_COUNT;
        flagged_for_combat[dst_air] = enemy_units_count[dst_air] > 0;
      }
      uint8_t airDistance = AIR_DIST[src_air][dst_air];
      /*
      // what is the actual destination that is a max of 4 air moves away?
      uint8_t air_destination = air_path4[src_air][dst_air];
      // check for bad move: if the actual destination is not water and is not
      // ally owned, and has no enemy units, then set actual destination to i
      if (air_destination < LANDS_COUNT && !is_allied[owner_idx[air_destination]] &&
          units_land_grand_total[air_destination] == 0) {
        air_destination = src_air;
      }

        // what is the actual air distance between the two?
        uint8_t airDistance = AIR_DIST[src_air][air_destination];
        if (airDistance == 4) {
          if (!canFighterLandHere[air_destination]) {
            air_destination = air_path3[src_air][user_input];
            airDistance = 3;
          }
        }
        if (airDistance == 3) {
          if (!canFighterLandIn1Move[air_destination]) {
            air_destination = air_path2[src_air][user_input];
            airDistance = 2;
          }
        }
        */
      units_air_ptr[dst_air][FIGHTERS][FIGHTER_MOVES_MAX - airDistance]++;
      if (dst_air < LANDS_COUNT) {
        units_land_player_total[dst_air][0]++;
        units_land_grand_total[dst_air]++;
        units_land_player_total[src_air][0]--;
        units_land_grand_total[src_air]--;
      } else {
        uint8_t seaDestination = dst_air - LANDS_COUNT;
        uint8_t starting_sea = src_air - LANDS_COUNT;
        units_sea_player_total[seaDestination][0]++;
        units_sea_grand_total[seaDestination]++;
        units_sea_player_total[starting_sea][0]--;
        units_sea_grand_total[starting_sea]--;
      }
      units_air_ptr[src_air][FIGHTERS][FIGHTER_MOVES_MAX]--;
    }
  }
}
void move_bomber_units() {
  // check if any bombers have full moves remaining
  clear_move_history();
  refresh_canBomberLandHere();
  refresh_canBomberLandIn1Move();
  refresh_canBomberLandIn2Moves();
  for (int src_land = 0; src_land < LANDS_COUNT; src_land++) {
    uint8_t valid_moves[AIRS_COUNT];
    valid_moves[0] = src_land;
    uint8_t valid_moves_count = 1;
    add_valid_bomber_moves(valid_moves, &valid_moves_count, src_land, BOMBER_MOVES_MAX);
    while (units_land_ptr[src_land][BOMBERS_LAND_AIR][BOMBER_MOVES_MAX] > 0) {
      uint8_t dst_air = get_user_move_input(FIGHTERS, src_land, valid_moves, valid_moves_count);
      update_move_history(dst_air, src_land, valid_moves, valid_moves_count);
      if (src_land == dst_air) {
        units_land_ptr[src_land][BOMBERS_LAND_AIR][0]++;
        units_land_ptr[src_land][BOMBERS_LAND_AIR][BOMBER_MOVES_MAX]--;
        continue;
      }
      if (dst_air < LANDS_COUNT) {
        if (!is_allied[owner_idx[dst_air]]) {
          flagged_for_combat[dst_air] =
              true; // assuming enemy units are present based on valid moves
        }
      } else {
        uint8_t dst_sea = dst_air - LANDS_COUNT;
        flagged_for_combat[dst_air] = enemy_units_count[dst_air] > 0;
      }
      uint8_t airDistance = AIR_DIST[src_land][dst_air];

      // check for bad move: if the actual destination has no enemy units and
      // is water or is not ally owned, then set actual destination to i
      /*
      if (actualDestination < LANDS_COUNT && !is_allied[owner_idx[actualDestination]] &&
          factory_hp[actualDestination] == 0 && units_land_grand_total[actualDestination] == 0) {
        actualDestination = src_land;
      } else if (actualDestination >= LANDS_COUNT) {
        if (units_sea_grand_total[actualDestination] == 0) {
          actualDestination = src_land;
        } else {
          for (int enemy_index = 0; enemy_index < enemies_count; enemy_index++) {
            if (units_sea_blockade_total[actualDestination][enemies[enemy_index]] > 0) {
              actualDestination = src_land;
              break;
            }
          }
        }
      }
      if (src_land == actualDestination) {
        units_air_ptr[src_land][BOMBERS_LAND_AIR][0]++;
        units_air_ptr[src_land][BOMBERS_LAND_AIR][BOMBER_MOVES_MAX]--;
        continue;
      } else {
        // what is the actual air distance between the two?
        uint8_t airDistance = AIR_DIST[src_land][actualDestination];
        if (airDistance == 6) {
          if (!canBomberLandHere[actualDestination]) {
            actualDestination = air_path5[src_land][user_input];
            airDistance = 5;
          }
        }
        if (airDistance == 5) {
          if (!canBomberLandIn1Move[actualDestination]) {
            actualDestination = air_path4[src_land][user_input];
            airDistance = 4;
          }
        }
        if (airDistance == 4) {
          if (!canBomberLandIn2Moves[actualDestination]) {
            actualDestination = air_path3[src_land][user_input];
            airDistance = 3;
          }
        }
        */
      if (dst_air < LANDS_COUNT) {
        units_land_ptr[dst_air][BOMBERS_LAND_AIR][BOMBER_MOVES_MAX - airDistance]++;
        units_land_player_total[dst_air][0]++;
        units_land_grand_total[dst_air]++;
        units_land_player_total[src_land][0]--;
        units_land_grand_total[src_land]--;
      } else {
        uint8_t seaDestination = dst_air - LANDS_COUNT;
        units_sea_ptr[seaDestination][BOMBERS_SEA][BOMBER_MOVES_MAX - 1 - airDistance]++;
        units_sea_player_total[seaDestination][0]++;
        units_sea_grand_total[seaDestination]++;
        units_sea_player_total[src_land][0]--;
        units_sea_grand_total[src_land]--;
      }
      units_land_ptr[src_land][BOMBERS_LAND_AIR][BOMBER_MOVES_MAX]--;
    }
  }
}

void conquer_land(uint8_t dst_land) {
  uint8_t old_owner_id = owner_idx[dst_land];
  if (PLAYERS[gamestate_player_to_mapstate_player[old_owner_id]].capital_territory_index ==
      dst_land) {
    money[0] += money[old_owner_id];
    money[old_owner_id] = 0;
  }
  income_per_turn[old_owner_id] -= LAND_VALUE[dst_land];
  uint8_t new_owner_id = 0;
  if (is_allied[original_owner_idx[dst_land]]) {
    new_owner_id = original_owner_idx[dst_land];
  }
  owner_idx[dst_land] = new_owner_id;
  income_per_turn[new_owner_id] += LAND_VALUE[dst_land];
}

void move_land_unit_type(uint8_t unit_type) {
  clear_move_history();
  for (int src_land = 0; src_land < LANDS_COUNT; src_land++) {
    for (int moves_remaining = MAX_MOVE_LAND[unit_type]; moves_remaining > 0; moves_remaining--) {
      uint8_t valid_moves[AIRS_COUNT];
      valid_moves[0] = src_land;
      uint8_t valid_moves_count = 1;
      add_valid_land_moves(valid_moves, &valid_moves_count, src_land, moves_remaining, unit_type);
      while (units_land_ptr[src_land][unit_type][moves_remaining] > 0) {
        uint8_t dst_air = get_user_move_input(unit_type, src_land, valid_moves, valid_moves_count);
        update_move_history(dst_air, src_land, valid_moves, valid_moves_count);
        if (src_land == dst_air) {
          units_land_ptr[src_land][unit_type][0]++;
          units_land_ptr[src_land][unit_type][moves_remaining]--;
          continue;
        }
        if (dst_air >= LANDS_COUNT) {
          load_transport(unit_type, src_land, dst_air - LANDS_COUNT);
          continue;
        }
        flagged_for_combat[dst_air] = enemy_units_count[dst_air] > 0;
        // if the destination is not blitzable, then end unit turn
        uint8_t landDistance;
        if (is_allied[owner_idx[dst_air]] || enemy_units_count[dst_air] > 0) {
          landDistance = MAX_MOVE_LAND[unit_type];
        } else {
          landDistance = LAND_DIST[src_land][dst_air];
          conquer_land(dst_air);
          flagged_for_combat[dst_air] = true;
        }
        units_land_ptr[dst_air][unit_type][moves_remaining - landDistance]++;
        units_land_player_total[dst_air][0]++;
        units_land_grand_total[dst_air]++;
        units_land_player_total[src_land][0]--;
        units_land_grand_total[src_land]--;
        units_land_ptr[src_land][unit_type][moves_remaining]--;
      }
    }
  }
}

void move_transport_units() {
  for (int unit_type = TRANS_1I; unit_type <= TRANS_1I_1T;
       unit_type++) { // there should be no TRANS_EMPTY
    int max_state = STATES_MOVE_SEA[unit_type] - STATES_STAGING[unit_type];
    int done_moving = STATES_UNLOADING[unit_type];
    int min_state = STATES_UNLOADING[unit_type] + 1;
    clear_move_history();
    // TODO CHECKPOINT
    for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      for (int cur_state = max_state; cur_state >= min_state; cur_state--) {
        int moves_remaining = cur_state - STATES_UNLOADING[unit_type];
        uint8_t valid_moves[AIRS_COUNT];
        valid_moves[0] = src_sea;
        uint8_t valid_moves_count = 1;
        add_valid_sea_moves(valid_moves, &valid_moves_count, src_sea, moves_remaining);
        while (units_sea_ptr[src_sea][unit_type][cur_state] > 0) {
          uint8_t src_air = src_sea + LANDS_COUNT;
          uint8_t dst_air = get_user_move_input(unit_type, src_air, valid_moves, valid_moves_count);
          update_move_history(dst_air, src_air, valid_moves, valid_moves_count);
          uint8_t dst_sea = dst_air - LANDS_COUNT;
          for (uint8_t enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
            if (units_sea_player_total[dst_sea][enemies[enemy_idx]] > 0) {
              flagged_for_combat[dst_sea] = true;
              break;
            }
          }
          if (src_air == dst_air) {
            units_sea_ptr[src_sea][unit_type][done_moving]++;
            units_sea_ptr[src_sea][unit_type][cur_state]--;
            continue;
          }
          /*
            uint8_t actualDestination;
            if (moves_remaining == 2) {
              // what is the actual destination that is a max of 2 sea moves away?
              actualDestination = sea_path2[src_sea][user_input];
            } else {
              // what is the actual destination that is a max of 1 sea moves away?
              actualDestination = sea_path1[src_sea][user_input];
            }
            if (src_sea == actualDestination || actualDestination == INFINITY) {
              units_sea_ptr[actualDestination][unit_type][done_moving]++;
              units_sea_ptr[src_sea][unit_type][cur_state]--;
              continue;
            }
            // what is the actual sea distance between the two?
            uint8_t seaDistance = SEA_DIST[canal_state][src_sea][actualDestination];
            // if the distance is 2, is the primary path blocked?
            if (seaDistance == 2) {
              uint8_t nextSeaMovement = sea_path1[src_sea][actualDestination];
              uint8_t nextSeaMovementAlt = nextSeaMovement;
              // check if the next sea movement has enemy ships
              bool hasEnemyShips = false;
              for (int enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
                if (units_sea_blockade_total[nextSeaMovement][enemies[enemy_idx]] > 0) {
                  hasEnemyShips = true;
                  nextSeaMovement = sea_path_alt1[src_sea][actualDestination];
                  break;
                }
              }
              if (hasEnemyShips && nextSeaMovementAlt != nextSeaMovement) {
                // check if the next sea movement has enemy ships
                hasEnemyShips = false;
                for (int enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
                  if (units_sea_blockade_total[nextSeaMovement][enemies[enemy_idx]] > 0) {
                    hasEnemyShips = true;
                    break;
                  }
                }
              }
              // if both paths are blocked, or dest is land
              // then move 1 space closer (where enemies are)
              if (hasEnemyShips || actualDestination < LANDS_COUNT) {
                actualDestination = nextSeaMovement;
              }
            }
            if (actualDestination < LANDS_COUNT) {
              units_sea_ptr[src_sea][unit_type][done_moving]++;
              units_sea_ptr[src_sea][unit_type][cur_state]--;
              continue;
            }
            */
          units_sea_ptr[dst_sea][unit_type][done_moving]++;
          // units_sea_type_total[actualDestination][i]++;
          units_sea_player_total[dst_sea][0]++;
          units_sea_grand_total[dst_sea]++;
          units_sea_ptr[dst_sea][unit_type][cur_state]--;
          // units_sea_type_total[j][i]--;
          units_sea_player_total[src_sea][0]--;
          units_sea_grand_total[src_sea]--;
        }
      }
    }
  }
}
void move_subs() {
  clear_move_history();
  for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    uint8_t valid_moves[AIRS_COUNT];
    valid_moves[0] = src_sea;
    uint8_t valid_moves_count = 1;
    add_valid_sub_moves(valid_moves, &valid_moves_count, src_sea, SUB_MOVES_MAX);
    while (units_sea_ptr[src_sea][SUBMARINES][SUB_UNMOVED] > 0) {
      uint8_t src_air = src_sea + LANDS_COUNT;
      uint8_t dst_air = get_user_move_input(SUBMARINES, src_air, valid_moves, valid_moves_count);
      update_move_history(dst_air, src_air, valid_moves, valid_moves_count);
      uint8_t dst_sea = dst_air - LANDS_COUNT;
      for (uint8_t enemy_idx = 0; enemy_idx < enemies_count;
           enemy_idx++) { // TODO optimize sea enemies present
        if (units_sea_player_total[dst_sea][enemies[enemy_idx]] > 0) {
          flagged_for_combat[dst_sea] = true;
          break;
        }
      }
      if (src_air == dst_air) {
        units_sea_ptr[src_sea][SUBMARINES][SUB_DONE_MOVING]++;
        units_sea_ptr[src_sea][SUBMARINES][SUB_UNMOVED]--;
        continue;
      }
      /*
    // what is the actual sea distance between the two?
    uint8_t seaDistance = sea_dist[src_sea][actualDestination];
    // if the distance is 2, is the primary path blocked?
    if (seaDistance == 2) {
      uint8_t nextSeaMovement = sea_path1[src_sea][actualDestination];
      uint8_t nextSeaMovementAlt = nextSeaMovement;
      // check if the next sea movement has enemy ships
      bool hasEnemyShips = false;
      for (int enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
        if (other_sea_units[nextSeaMovement][enemies[enemy_idx]][DESTROYERS] > 0) {
          hasEnemyShips = true;
          nextSeaMovement = SEA_PATH_ALT[canal_state][src_sea][actualDestination];
          break;
        }
      }
      if (hasEnemyShips && nextSeaMovementAlt != nextSeaMovement) {
        // check if the next sea movement has enemy ships
        hasEnemyShips = false;
        for (int l = 0; l < enemies_count; l++) {
          if (other_sea_units[nextSeaMovement][enemies[l]][DESTROYERS] > 0) {
            hasEnemyShips = true;
            break;
          }
        }
      }
      // if both paths are blocked, or dest is land
      // then move 1 space closer (where enemies are)
      if (hasEnemyShips || actualDestination < LANDS_COUNT) {
        actualDestination = nextSeaMovement;
      }
    }
    if (actualDestination < LANDS_COUNT) {
      units_sea_ptr[src_sea][SUBMARINES][SUB_DONE_MOVING]++;
      units_sea_ptr[src_sea][SUBMARINES][SUB_UNMOVED]--;
      continue;
    }
    */
      units_sea_ptr[dst_sea][SUBMARINES][SUB_DONE_MOVING]++;
      units_sea_player_total[dst_sea][0]++;
      units_sea_grand_total[dst_sea]++;
      units_sea_ptr[dst_sea][SUBMARINES][SUB_UNMOVED]--;
      units_sea_player_total[src_sea][0]--;
      units_sea_grand_total[src_sea]--;
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
      uint8_t valid_moves[AIRS_COUNT];
      valid_moves[0] = src_sea;
      uint8_t valid_moves_count = 1;
      add_valid_sea_moves(valid_moves, &valid_moves_count, src_sea, moves_remaining);
      while (units_sea_ptr[src_sea][unit_type][unmoved] > 0) {
        uint8_t src_air = src_sea + LANDS_COUNT;
        uint8_t dst_air = get_user_move_input(SUBMARINES, src_air, valid_moves, valid_moves_count);
        update_move_history(dst_air, src_air, valid_moves, valid_moves_count);
        uint8_t dst_sea = dst_air - LANDS_COUNT;
        for (uint8_t enemy_idx = 0; enemy_idx < enemies_count;
             enemy_idx++) { // TODO optimize sea enemies present
          if (units_sea_player_total[dst_sea][enemies[enemy_idx]] > 0) {
            flagged_for_combat[dst_sea] = true;
            break;
          }
        }
        if (src_air == dst_air) {
          units_sea_ptr[src_sea][unit_type][done_moving]++;
          units_sea_ptr[src_sea][unit_type][unmoved]--;
          continue;
        }
        /*
        // what is the actual sea distance between the two?
        uint8_t seaDistance = sea_dist[src_sea][actualDestination];
        // if the distance is 2, is the primary path blocked?
        if (seaDistance == 2) {
          uint8_t nextSeaMovement = sea_path2[src_sea][actualDestination];
          uint8_t nextSeaMovementAlt = nextSeaMovement;
          // check if the next sea movement has enemy ships
          bool hasEnemyShips = false;
          for (int enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
            if (units_sea_blockade_total[nextSeaMovement][enemies[enemy_idx]] > 0) {
              hasEnemyShips = true;
              nextSeaMovement = sea_path_alt1[src_sea][actualDestination];
              break;
            }
          }
          if (hasEnemyShips && nextSeaMovementAlt != nextSeaMovement) {
            // check if the next sea movement has enemy ships
            hasEnemyShips = false;
            for (int enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
              if (units_sea_blockade_total[nextSeaMovement][enemies[enemy_idx]] > 0) {
                hasEnemyShips = true;
                break;
              }
            }
          }
          // if both paths are blocked, or dest is land
          // then move 1 space closer (where enemies are)
          if (hasEnemyShips || actualDestination < LANDS_COUNT) {
            actualDestination = nextSeaMovement;
          }
        }
        if (actualDestination < LANDS_COUNT) {
          units_sea_ptr[src_sea][unit_type][DONE_MOVING_SEA[unit_type]]++;
          units_sea_ptr[src_sea][unit_type][unmoved]--;
          continue;
        }
        */

        units_sea_ptr[dst_sea][unit_type][done_moving]++;
        units_sea_player_total[dst_sea][0]++;
        units_sea_grand_total[dst_sea]++;
        units_sea_ptr[dst_sea][unit_type][unmoved]--;
        units_sea_player_total[src_sea][0]--;
        units_sea_grand_total[src_sea]--;
      }
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
    if (!flagged_for_combat[src_air]) {
      continue;
    }
    if (units_sea_player_total[src_sea][0] == 0) {
      continue;
    }
    bool defender_submerged = units_sea_ptr[src_sea][DESTROYERS][0] == 0;
    if (defender_submerged && units_sea_player_total[src_sea][0] == total_enemy_subs[src_sea]) {
      continue;
    }
    // combat is proceeding, so disable bombardment capabilities of ships
    for (uint8_t unit_type = CRUISERS; unit_type <= BS_DAMAGED; unit_type++) {
      units_sea_ptr[src_sea][unit_type][0] += units_sea_ptr[src_sea][unit_type][1];
      units_sea_ptr[src_sea][unit_type][1] = 0;
    }

    while (true) {
      bool targets_exist = false;
      if (units_sea_blockade_total[src_sea][0] > 0) {
        for (int enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
          if (units_sea_player_total[src_sea][enemies[enemy_idx]] > 0) {
            targets_exist = true;
            break;
          }
        }
      } else if (units_sea_ptr[src_sea][SUBMARINES][0] > 0) { // no blockade ships, only subs
        for (int enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
          if (units_sea_player_total[src_sea][enemies[enemy_idx]] -
                  other_sea_units[src_air][enemies[enemy_idx]][FIGHTERS] >
              0) {
            targets_exist = true;
            break;
          }
        }
      } else if (air_total[FIGHTERS][src_air] + air_total[BOMBERS_LAND_AIR][src_air] >
                 0) { // no ships, only air
        for (int enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
          if (units_sea_player_total[src_sea][enemies[enemy_idx]] -
                  other_sea_units[src_air][enemies[enemy_idx]][SUBMARINES] >
              0) {
            targets_exist = true;
            break;
          }
        }
      } else if (units_sea_player_total[src_sea][0] > 0) { // attacker only has transports
        for (int enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
          if (units_sea_player_total[src_sea][enemies[enemy_idx]] > 0) {
            targets_exist = true;
            break;
          }
        }
      }
      // untargetable battle
      if (!targets_exist) {
        allied_carriers[src_sea] =
            data.units_sea[src_sea].carriers[0] + data.units_sea[src_sea].carriers[1];
        for (int player_idx = 0; player_idx < PLAYERS_COUNT - 1; player_idx++) {
          allied_carriers[src_sea] +=
              other_sea_units[src_sea][player_idx][CARRIERS] * is_allied[player_idx];
        }
        continue;
      }
      // fire subs (defender always submerges if possible)
      int attacker_damage = units_sea_ptr[src_sea][SUBMARINES][0] * SUB_ATTACK;
      uint8_t attacker_hits =
          (attacker_damage / 6) +
          (RANDOM_NUMBERS[random_number_index++] % 6 < attacker_damage % 6 ? 1 : 0);

      int defender_damage;
      int defender_hits;
      if (!defender_submerged) {
        defender_damage = 0;
        for (int enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
          defender_damage += other_sea_units[src_air][enemies[enemy_idx]][SUBMARINES];
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
      for (uint8_t unit_type = 0; unit_type < BLOCKADE_UNIT_TYPES_COUNT; unit_type++) {
        attacker_damage += units_sea_ptr[src_sea][unit_type][0] * ATTACK_UNIT_SEA[unit_type];
      }
      for (uint8_t unit_type = 0; unit_type < AIR_UNIT_TYPES_COUNT; unit_type++) {
        attacker_damage += air_total[src_air][unit_type] * ATTACK_UNIT_LAND[unit_type];
      }
      attacker_hits = (attacker_damage / 6) +
                      (RANDOM_NUMBERS[random_number_index++] % 6 < attacker_damage % 6 ? 1 : 0);

      defender_damage = 0;
      for (uint8_t enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
        uint8_t* enemy_units = other_sea_units[src_sea][enemies[enemy_idx]];
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

      if (enemy_units_count[src_air] == 0 || units_sea_player_total[src_sea][0] == 0) {
        continue;
      }

      // ask to retreat (0-255, any non valid retreat zone is considered a no)
      uint8_t retreat = ask_to_retreat();
      // if retreat, move units to retreat zone immediately and end battle
      if (sea_dist[src_sea][retreat] == 1 && enemy_units_count[retreat] == 0 &&
          !flagged_for_combat[retreat]) {
        for (uint8_t unit_type = TRANS_EMPTY; unit_type <= BS_DAMAGED; unit_type++) {
          units_sea_ptr[retreat][unit_type][0] +=
              units_sea_ptr[src_sea][unit_type][STATES_UNLOADING[unit_type]];
          units_sea_ptr[src_sea][unit_type][STATES_UNLOADING[unit_type]] = 0;
          flagged_for_combat[src_air] = false;
        }
        continue;
      }
      // loop
    }
  }
}

uint8_t ask_to_retreat() {
  if (current_player_is_human) {
    printf("To where do you want to retreat (255 for no)? ");
    return getUserInput(RETREAT_OPTIONS, 256);
  }
  return getAIInput(RETREAT_OPTIONS, 256);
}
void remove_land_defenders(uint8_t src_land, uint8_t hits) {
  for (uint8_t unit_idx = 0; unit_idx < DEFENDER_LAND_UNIT_TYPES; unit_idx++) {
    for (uint8_t enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
      uint8_t enemy = enemies[enemy_idx];
      uint8_t* total_units = &other_land_units[src_land][enemy][ORDER_OF_LAND_DEFENDERS[unit_idx]];
      if (*total_units > 0) {
        if (*total_units < hits) {
          hits -= *total_units;
          units_land_player_total[src_land][enemy] -= *total_units;
          enemy_units_count[src_land] -= *total_units;
          units_land_grand_total[src_land] -= *total_units;
          *total_units = 0;
        } else {
          *total_units -= hits;
          units_land_player_total[src_land][enemy] -= hits;
          enemy_units_count[src_land] -= hits;
          units_land_grand_total[src_land] -= hits;
          return;
        }
      }
    }
  }
}
void remove_land_attackers(uint8_t src_land, uint8_t hits) {
  uint8_t* total_units;
  for (uint8_t unit_idx; unit_idx < ATTACKER_LAND_UNIT_TYPES_1; unit_idx++) {    
    uint8_t unit_type = ORDER_OF_LAND_ATTACKERS_1[unit_idx];
    total_units = &units_sea_ptr[src_land][unit_type][0];
    if (*total_units > 0) {
      if (*total_units < hits) {
        hits -= *total_units;
        units_sea_player_total[src_land][0] -= *total_units;
        units_sea_grand_total[src_land] -= *total_units;
        *total_units = 0;
      } else {
        *total_units -= hits;
        units_sea_player_total[src_land][0] -= hits;
        units_sea_grand_total[src_land] -= hits;
        return;
      }
    }
  }
  for (uint8_t unit_idx = 0; unit_idx < ATTACKER_LAND_UNIT_TYPES_2; unit_idx++) {
    uint8_t unit_type = ORDER_OF_LAND_ATTACKERS_2[unit_idx];
    if (air_total[src_land][unit_type] == 0)
      continue;
    for (uint8_t cur_state = 1; cur_state < STATES_MOVE_LAND[unit_type] - 1; cur_state++) {
      total_units = &units_land_ptr[src_land][unit_type][cur_state];
      if (*total_units > 0) {
        if (*total_units < hits) {
          hits -= *total_units;
          air_total[src_land][unit_type] -= *total_units;
          units_land_player_total[src_land][0] -= *total_units;
          units_land_grand_total[src_land] -= *total_units;
          *total_units = 0;
        } else {
          *total_units -= hits;
          air_total[src_land][unit_type] -= hits;
          units_land_player_total[src_land][0] -= hits;
          units_land_grand_total[src_land] -= hits;
          return;
        }
      }
    }
  }

}
void remove_sea_defenders(uint8_t src_sea, uint8_t hits, bool defender_submerged) {
  for (uint8_t enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
    uint8_t enemy = enemies[enemy_idx];
    uint8_t* total_battleships = &other_sea_units[src_sea][enemy][BATTLESHIPS];
    uint8_t* total_bs_damaged = &other_sea_units[src_sea][enemy][BS_DAMAGED];
    if (*total_battleships > 0) {
      if (*total_battleships < hits) {
        hits -= *total_battleships;
        *total_bs_damaged = *total_battleships;
        *total_battleships = 0;
      } else {
        *total_bs_damaged += hits;
        *total_battleships -= hits;
        return;
      }
    }
  }
  if (!defender_submerged && total_enemy_subs > 0) {
    for (uint8_t enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
      uint8_t enemy = enemies[enemy_idx];
      uint8_t* total_units = &other_sea_units[src_sea][enemy][SUBMARINES];
      if (*total_units > 0) {
        if (*total_units < hits) {
          hits -= *total_units;
          units_sea_player_total[src_sea][enemy] -= *total_units;
          total_enemy_subs[src_sea] -= *total_units;
          enemy_units_count[src_sea] -= *total_units;
          units_sea_grand_total[src_sea] -= *total_units;
          *total_units = 0;
        } else {
          *total_units -= hits;
          units_sea_player_total[src_sea][enemy] -= hits;
          total_enemy_subs[src_sea] -= hits;
          enemy_units_count[src_sea] -= hits;
          units_sea_grand_total[src_sea] -= hits;
          return;
        }
      }
    }
  }

  // skipping submarines
  for (uint8_t unit_idx = 1; unit_idx < DEFENDER_SEA_UNIT_TYPES; unit_idx++) {
    for (uint8_t enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
      uint8_t enemy = enemies[enemy_idx];
      uint8_t* total_units = &other_sea_units[src_sea][enemy][ORDER_OF_SEA_DEFENDERS[unit_idx]];
      if (*total_units > 0) {
        if (*total_units < hits) {
          hits -= *total_units;
          units_sea_player_total[src_sea][enemy] -= *total_units;
          enemy_units_count[src_sea] -= *total_units;
          units_sea_grand_total[src_sea] -= *total_units;
          *total_units = 0;
        } else {
          *total_units -= hits;
          units_sea_player_total[src_sea][enemy] -= hits;
          enemy_units_count[src_sea] -= hits;
          units_sea_grand_total[src_sea] -= hits;
          return;
        }
      }
    }
  }
}

void remove_sea_attackers(uint8_t src_sea, uint8_t hits) {
  uint8_t* total_battleships = &units_sea_ptr[src_sea][BATTLESHIPS][0];
  uint8_t* total_bs_damaged = &units_sea_ptr[src_sea][BS_DAMAGED][0];
  if (*total_battleships > 0) {
    if (*total_battleships < hits) {
      hits -= *total_battleships;
      *total_bs_damaged = *total_battleships;
      *total_battleships = 0;
    } else {
      *total_bs_damaged += hits;
      *total_battleships -= hits;
      return;
    }
  }
  uint8_t* total_units;
  for (uint8_t unit_idx; unit_idx < ATTACKER_SEA_UNIT_TYPES_1; unit_idx++) {
    uint8_t* total_units = &units_sea_ptr[src_sea][ORDER_OF_SEA_ATTACKERS_1[unit_idx]][0];
    if (*total_units > 0) {
      if (*total_units < hits) {
        hits -= *total_units;
        units_sea_player_total[src_sea][0] -= *total_units;
        units_sea_grand_total[src_sea] -= *total_units;
        *total_units = 0;
      } else {
        *total_units -= hits;
        units_sea_player_total[src_sea][0] -= hits;
        units_sea_grand_total[src_sea] -= hits;
        return;
      }
    }
  }
  total_units = &units_sea_ptr[src_sea][CARRIERS][0];
  if (*total_units > 0) {
    if (*total_units < hits) {
      hits -= *total_units;
      allied_carriers[src_sea] -= *total_units;
      units_sea_player_total[src_sea][0] -= *total_units;
      units_sea_grand_total[src_sea] -= *total_units;
      *total_units = 0;
    } else {
      *total_units -= hits;
      allied_carriers[src_sea] -= hits;
      units_sea_player_total[src_sea][0] -= hits;
      units_sea_grand_total[src_sea] -= hits;
      return;
    }
  }
  total_units = &units_sea_ptr[src_sea][CRUISERS][0];
  if (*total_units > 0) {
    if (*total_units < hits) {
      hits -= *total_units;
      units_sea_player_total[src_sea][0] -= *total_units;
      units_sea_grand_total[src_sea] -= *total_units;
      *total_units = 0;
    } else {
      *total_units -= hits;
      units_sea_player_total[src_sea][0] -= hits;
      units_sea_grand_total[src_sea] -= hits;
      return;
    }
  }
  uint8_t src_air = src_sea + LANDS_COUNT;
  for (uint8_t unit_idx = 0; unit_idx < ATTACKER_SEA_UNIT_TYPES_2; unit_idx++) {
    uint8_t unit_type = ORDER_OF_SEA_ATTACKERS_2[unit_idx];
    if (air_total[src_air][unit_type] == 0)
      continue;
    for (uint8_t cur_state = 0; cur_state < STATES_MOVE_SEA[unit_type]; cur_state++) {
      total_units = &units_sea_ptr[src_sea][unit_type][cur_state];
      if (*total_units > 0) {
        if (*total_units < hits) {
          hits -= *total_units;
          air_total[src_air][unit_type] -= *total_units;
          units_sea_player_total[src_sea][0] -= *total_units;
          units_sea_grand_total[src_sea] -= *total_units;
          *total_units = 0;
        } else {
          *total_units -= hits;
          air_total[src_air][unit_type] -= hits;
          units_sea_player_total[src_sea][0] -= hits;
          units_sea_grand_total[src_sea] -= hits;
          return;
        }
      }
    }
  }
  for (uint8_t unit_idx; unit_idx < ATTACKER_SEA_UNIT_TYPES_3; unit_idx++) {
    uint8_t* total_units = &units_sea_ptr[src_sea][ORDER_OF_SEA_ATTACKERS_3[unit_idx]][0];
    if (*total_units > 0) {
      if (*total_units < hits) {
        hits -= *total_units;
        units_sea_grand_total[src_sea] -= *total_units;
        units_sea_player_total[src_sea][0] -= *total_units;
        *total_units = 0;
      } else {
        *total_units -= hits;
        units_sea_player_total[src_sea][0] -= hits;
        units_sea_grand_total[src_sea] -= hits;
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
      uint8_t valid_moves[AIRS_COUNT];
      valid_moves[0] = src_sea;
      uint8_t valid_moves_count = 1;
      add_valid_unload_moves(valid_moves, &valid_moves_count, src_sea);
      while (units_sea_ptr[src_sea][unit_type][unloading_state] > 0) {
        uint8_t src_air = src_sea + LANDS_COUNT;
        uint8_t dst_air = get_user_move_input(unit_type, src_air, valid_moves, valid_moves_count);
        update_move_history(dst_air, src_sea, valid_moves, valid_moves_count);
        if (src_air == dst_air) {
          units_sea_ptr[src_sea][unit_type][0]++;
          units_sea_ptr[src_sea][unit_type][unloading_state]--;
          continue;
        }
        flagged_for_combat[dst_air] = enemy_units_count[dst_air] > 0;
        bombard_max[dst_air]++;
        units_land_ptr[dst_air][unload_cargo1][0]++;
        units_land_player_total[dst_air][0]++;
        units_land_grand_total[dst_air]++;
        units_sea_ptr[src_sea][TRANS_EMPTY][0]++;
        units_sea_ptr[src_sea][unit_type][unloading_state]--;
        if (unit_type > TRANS_1T) {
          bombard_max[dst_air]++;
          units_land_ptr[dst_air][unload_cargo2][0]++;
          units_land_player_total[dst_air][0]++;
          units_land_grand_total[dst_air]++;
        }
      }
    }
  }
}
void resolve_land_battles() {
  for (uint8_t src_land = 0; src_land < LANDS_COUNT; src_land++) {
    // check if battle is over
    if (!flagged_for_combat[src_land]) {
      continue;
    }
    // check if no friendlies remain
    if (units_land_player_total[src_land][0] == 0) {
      continue;
    }
    uint8_t attacker_damage;
    uint8_t attacker_hits;
    // only bombers exist
    if (air_total[src_land][BOMBERS_LAND_AIR] > 0 &&
        units_land_player_total[src_land][0] == air_total[src_land][BOMBERS_LAND_AIR]) {
      if (factory_hp[src_land] > 0) {
        // fire_strat_aa_guns();
        uint8_t defender_damage = air_total[src_land][BOMBERS_LAND_AIR];
        uint8_t defender_hits =
            (defender_damage / 6) +
            (RANDOM_NUMBERS[random_number_index++] % 6 < defender_damage % 6 ? 1 : 0);
        if (defender_hits > 0) {
          for (uint8_t cur_state = 1; cur_state < BOMBER_LAND_STATES - 1; cur_state++) {
            uint8_t* total_units = &units_land_ptr[src_land][BOMBERS_LAND_AIR][cur_state];
            if (*total_units < defender_hits) {
              defender_hits -= *total_units;
              air_total[src_land][BOMBERS_LAND_AIR] -= *total_units;
              units_land_player_total[src_land][0] -= *total_units;
              units_land_grand_total[src_land] -= *total_units;
              *total_units = 0;
            } else {
              *total_units -= defender_hits;
              air_total[src_land][BOMBERS_LAND_AIR] -= defender_hits;
              units_land_player_total[src_land][0] -= defender_hits;
              units_land_grand_total[src_land] -= defender_hits;
              return;
            }
          }
        }
        attacker_damage = air_total[src_land][BOMBERS_LAND_AIR] * 21;
        attacker_hits = (attacker_damage / 6) +
                        (RANDOM_NUMBERS[random_number_index++] % 6 < attacker_damage % 6 ? 1 : 0);
        factory_hp[src_land] =
            factory_hp[src_land] > attacker_hits ? factory_hp[src_land] - attacker_hits : 0;
        continue;
      }
    }
    while (true) {
      if (enemy_units_count[src_land] == 0) {
        // if infantry, artillery, tanks exist then capture
        if (units_land_ptr[src_land][INFANTRY][0] + units_land_ptr[src_land][ARTILLERY][0] +
                units_land_ptr[src_land][TANKS][0] >
            0) {
          conquer_land(src_land);
        }
        continue;
      }
      // bombard_shores
      if (bombard_max[src_land] > 0) {
        attacker_damage = 0;
        for (uint8_t unit_type = BS_DAMAGED; unit_type >= CRUISERS; unit_type--) {
          for (uint8_t sea_idx = 0; sea_idx < LAND_TO_SEA_COUNT[src_land]; sea_idx++) {
            uint8_t src_sea = LAND_TO_SEA_CONN[src_land][sea_idx];
            while (units_sea_ptr[src_sea][unit_type][1] > 0 && bombard_max[src_land] > 0) {
              attacker_damage += ATTACK_UNIT_SEA[unit_type];
              units_sea_ptr[src_sea][unit_type][0]++;
              units_sea_ptr[src_sea][unit_type][1]--;
              bombard_max[src_land]--;
            }
          }
        }
        attacker_hits = (attacker_damage / 6) +
                        (RANDOM_NUMBERS[random_number_index++] % 6 < attacker_damage % 6 ? 1 : 0);
        if (attacker_hits > 0) {
          remove_land_defenders(src_land, attacker_hits);
        }
      }
      // check if can fire tactical aa_guns
      uint8_t total_air_units =
          air_total[src_land][FIGHTERS] + air_total[src_land][BOMBERS_LAND_AIR];
      uint8_t defender_damage;
      uint8_t defender_hits;
      if (total_air_units > 0) {
        int total_aa_guns = 0;
        for (uint8_t enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
          total_aa_guns += other_land_units[src_land][enemies[enemy_idx]][AA_GUNS];
        }
        if (total_aa_guns > 0) {
          // fire_tact_aa_guns();
          defender_damage = total_air_units * 3;
          defender_hits = (defender_damage / 6) +
                          (RANDOM_NUMBERS[random_number_index++] % 6 < defender_damage % 6 ? 1 : 0);
          if (defender_hits > 0) {
            for (uint8_t cur_state = 0; cur_state < FIGHTER_STATES; cur_state++) {
              uint8_t* total_units = &units_land_ptr[src_land][FIGHTERS][cur_state];
              if (*total_units < defender_hits) {
                defender_hits -= *total_units;
                air_total[src_land][FIGHTERS] -= *total_units;
                units_land_player_total[src_land][0] -= *total_units;
                units_land_grand_total[src_land] -= *total_units;
                *total_units = 0;
              } else {
                *total_units -= defender_hits;
                air_total[src_land][FIGHTERS] -= defender_hits;
                units_land_player_total[src_land][0] -= defender_hits;
                units_land_grand_total[src_land] -= defender_hits;
                break;
              }
            }
          }
          if (defender_hits > 0) {
            for (uint8_t cur_state = 0; cur_state < BOMBER_LAND_STATES; cur_state++) {
              uint8_t* total_units = &units_land_ptr[src_land][BOMBERS_LAND_AIR][cur_state];
              if (*total_units < defender_hits) {
                defender_hits -= *total_units;
                air_total[src_land][BOMBERS_LAND_AIR] -= *total_units;
                units_land_player_total[src_land][0] -= *total_units;
                units_land_grand_total[src_land] -= *total_units;
                *total_units = 0;
              } else {
                *total_units -= defender_hits;
                air_total[src_land][BOMBERS_LAND_AIR] -= defender_hits;
                units_land_player_total[src_land][0] -= defender_hits;
                units_land_grand_total[src_land] -= defender_hits;
                break;
              }
            }
          }
        }
      }
      // land_battle
      attacker_damage = 0;
      for (uint8_t cur_state = 1; cur_state < FIGHTER_STATES - 1; cur_state++) {
        attacker_damage += units_land_ptr[src_land][FIGHTERS][cur_state] * FIGHTER_ATTACK;
      }
      for (uint8_t cur_state = 1; cur_state < BOMBER_LAND_STATES - 1; cur_state++) {
        attacker_damage += units_land_ptr[src_land][BOMBERS_LAND_AIR][cur_state] * BOMBER_ATTACK;
      }
      uint8_t infantry_count = units_land_ptr[src_land][INFANTRY][0];
      uint8_t artillery_count = units_land_ptr[src_land][ARTILLERY][0];
      attacker_damage += (infantry_count * INFANTRY_ATTACK) + (artillery_count * ARTILLERY_ATTACK) +
                         (units_land_ptr[src_land][TANKS][0] * TANK_ATTACK);
      // add damage for the minimum of count of infantry/artillery
      attacker_damage += infantry_count < artillery_count ? infantry_count : artillery_count;
      attacker_hits = (attacker_damage / 6) +
                      (RANDOM_NUMBERS[random_number_index++] % 6 < attacker_damage % 6 ? 1 : 0);

      for (uint8_t enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
        uint8_t* land_units = other_land_units[src_land][enemies[enemy_idx]];
        defender_damage = (land_units[INFANTRY] * INFANTRY_DEFENSE) +
                          (land_units[ARTILLERY] * ARTILLERY_DEFENSE) +
                          (land_units[TANKS] * TANK_DEFENSE) +
                          (land_units[FIGHTERS] * FIGHTER_DEFENSE) +
                          (land_units[BOMBERS_LAND_AIR] * BOMBER_DEFENSE);
        defender_hits = (defender_damage / 6) +
                        (RANDOM_NUMBERS[random_number_index++] % 6 < defender_damage % 6 ? 1 : 0);
      }
      if (defender_hits > 0) {
        remove_land_attackers(src_land, defender_hits);
      }
      if (attacker_hits > 0) {
        remove_land_defenders(src_land, attacker_hits);
      }
      // ask to retreat (0-255, any non valid retreat zone is considered a no)
      uint8_t retreat = ask_to_retreat();
      // if retreat, move units to retreat zone immediately and end battle
      if (LAND_DIST[src_land][retreat] == 1 && enemy_units_count[retreat] == 0 &&
          !flagged_for_combat[retreat]) {
        for (uint8_t unit_type = INFANTRY; unit_type <= TANKS; unit_type++) {
          units_land_ptr[retreat][unit_type][0] += units_land_ptr[src_land][unit_type][0];
          units_land_ptr[src_land][unit_type][0] = 0;
          flagged_for_combat[src_land] = false;
        }
        continue;
      }
    }
  }
}
void move_aa_guns() {
  move_land_unit_type(AA_GUNS);
  // clear_move_history();
  // for (uint8_t src_land = 0; src_land < LANDS_COUNT; src_land++) {
  //   uint8_t valid_moves[AIRS_COUNT];
  //   valid_moves[0] = src_land;
  //   uint8_t valid_moves_count = 1;
  //   add_valid_land_moves(valid_moves, &valid_moves_count, src_land, 1, AA_GUNS);
  //   while (units_land_ptr[src_land][AA_GUNS][1] > 0) {
  //     uint8_t src_air = src_land + LANDS_COUNT;
  //     uint8_t dst_air = get_user_move_input(AA_GUNS, src_air, valid_moves, valid_moves_count);
  //     update_move_history(dst_air, src_land, valid_moves, valid_moves_count);
  //     if (src_air == dst_air) {
  //       units_land_ptr[src_land][AA_GUNS][0]++;
  //       units_land_ptr[src_land][AA_GUNS][1]--;
  //       continue;
  //     }
  //     units_land_ptr[dst_air][AA_GUNS][0]++;
  //     units_land_player_total[dst_air][0]++;
  //     units_land_grand_total[dst_air]++;
  //     units_land_ptr[dst_air][AA_GUNS][1]--;
  //     units_land_player_total[src_land][0]--;
  //     units_land_grand_total[src_land]--;
  //   }
  // }
}

void add_valid_unload_moves(uint8_t* valid_moves, uint8_t* valid_moves_count, uint8_t src_sea) {
  uint8_t* near_land = SEA_TO_LAND_CONN[src_sea];
  for (int land_idx = 0; land_idx < SEA_TO_LAND_COUNT[src_sea]; land_idx++) {
    uint8_t dst_land = near_land[land_idx];
    add_valid_air_move_if_history_allows_X(valid_moves, valid_moves_count, dst_land,
                                           src_sea + LANDS_COUNT, 1);
  }
}

void add_valid_fighter_moves(uint8_t* valid_moves, uint8_t* valid_moves_count, uint8_t src_air,
                             uint8_t remaining_moves) {
  uint8_t* near_air = AIR_WITHIN_X_MOVES[remaining_moves - 1][src_air];
  for (int i = 0; i < AIR_WITHIN_X_MOVES_COUNT[remaining_moves - 1][src_air]; i++) {
    uint8_t dst_air = near_air[i];
    uint8_t air_dist = AIR_DIST[src_air][dst_air];
    if (air_dist <= 2 || canFighterLandHere[dst_air] ||
        (air_dist == 3 && canFighterLandIn1Move[dst_air])) {
      add_valid_air_move_if_history_allows_X(valid_moves, valid_moves_count, dst_air, src_air,
                                             air_dist);
    }
  }
}

void add_valid_fighter_landing(uint8_t* valid_moves, uint8_t* valid_moves_count, uint8_t src_air,
                               uint8_t remaining_moves) {
  uint8_t* near_land = AIR_WITHIN_X_MOVES[remaining_moves - 1][src_air];
  for (int i = 0; i < AIR_WITHIN_X_MOVES_COUNT[remaining_moves - 1][src_air]; i++) {
    uint8_t dst_air = near_land[i];
    if (canFighterLandHere[dst_air]) {
      add_valid_air_move_if_history_allows_X(valid_moves, valid_moves_count, dst_air, src_air,
                                             remaining_moves);
    }
  }
}

void add_valid_bomber_moves(uint8_t* valid_moves, uint8_t* valid_moves_count, uint8_t src_air,
                            uint8_t remaining_moves) {
  uint8_t* near_land = AIR_WITHIN_X_MOVES[remaining_moves - 1][src_air];
  for (int i = 0; i < AIR_WITHIN_X_MOVES_COUNT[remaining_moves - 1][src_air]; i++) {
    uint8_t dst_air = near_land[i];
    uint8_t air_dist = AIR_DIST[src_air][dst_air];
    if (air_dist <= 3 || canBomberLandHere[dst_air] ||
        (air_dist == 4 && canBomberLandIn2Moves[dst_air]) ||
        (air_dist == 5 && canBomberLandIn1Move[dst_air])) {
      add_valid_air_move_if_history_allows_X(valid_moves, valid_moves_count, dst_air, src_air,
                                             air_dist);
    }
  }
}

void land_fighter_units() {
  refresh_canFighterLandHere_final();
  //  check if any fighters have moves remaining
  for (uint8_t cur_state = 1; cur_state < FIGHTER_STATES - 1;
       cur_state++) { // TODO optimize to find next fighter faster
    uint8_t* total_fighter_count = &units_air_ptr[0][FIGHTERS][cur_state];
    clear_move_history();
    for (int src_air = 0; src_air < AIRS_COUNT; src_air++) {
      uint8_t valid_moves[AIRS_COUNT];
      valid_moves[0] = src_air;
      uint8_t valid_moves_count = 1;
      add_valid_fighter_landing(valid_moves, &valid_moves_count, src_air, cur_state);
      while (*total_fighter_count > 0) {
        uint8_t dst_air = get_user_move_input(FIGHTERS, src_air, valid_moves, valid_moves_count);
        update_move_history(dst_air, src_air, valid_moves, valid_moves_count);
        if (src_air == dst_air) {
          units_land_ptr[src_air][FIGHTERS][0]++;
          *total_fighter_count -= 1;
          continue;
        }
        units_air_ptr[dst_air][FIGHTERS][0]++;
        if (dst_air < LANDS_COUNT) {
          units_land_player_total[dst_air][0]++;
          units_land_grand_total[dst_air]++;
          units_land_player_total[src_air][0]--;
          units_land_grand_total[src_air]--;
        } else {
          uint8_t dst_sea = dst_air - LANDS_COUNT;
          units_sea_player_total[dst_sea][0]++;
          units_sea_grand_total[dst_sea]++;
          uint8_t src_sea = src_air - LANDS_COUNT;
          units_sea_player_total[src_sea][0]--;
          units_sea_grand_total[src_sea]--;
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
    uint8_t* total_bomber_count = &units_air_ptr[0][BOMBERS_LAND_AIR][cur_state];
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
        update_move_history(dst_air, src_air, valid_moves, valid_moves_count);
        if (src_air == dst_air) {
          units_air_ptr[src_air][BOMBERS_LAND_AIR][0]++;
          *total_bomber_count -= 1;
          continue;
        }
        units_air_ptr[dst_air][BOMBERS_LAND_AIR][0]++;
        if (dst_air < LANDS_COUNT) {
          units_land_player_total[dst_air][0]++;
          units_land_grand_total[dst_air]++;
          units_land_player_total[src_air][0]--;
          units_land_grand_total[src_air]--;
        } else {
          uint8_t dst_sea = dst_air - LANDS_COUNT;
          units_sea_player_total[dst_sea][0]++;
          units_sea_grand_total[dst_sea]++;
          uint8_t src_sea = src_air - LANDS_COUNT;
          units_sea_player_total[src_sea][0]--;
          units_sea_grand_total[src_sea]--;
        }
        *total_bomber_count -= 1;
      }
    }
  }
}
void buy_units() {
  for (uint8_t land_idx = 0; land_idx < my_factory_count; land_idx++) {
    uint8_t dst_land = my_factory_locations[land_idx];
    if (builds_left[dst_land] == 0) {
      continue;
    }
    uint8_t additional_cost = 0;
    // buy sea units
    for (uint8_t sea_idx = 0; sea_idx < LAND_TO_SEA_COUNT[dst_land]; sea_idx++) {
      uint8_t dst_sea = LAND_TO_SEA_CONN[dst_land][sea_idx];
      uint8_t valid_purchases[COST_UNIT_SEA_COUNT + 1];
      valid_purchases[0] = 0; // pass all units
      uint8_t valid_purchases_count = 1;
      uint8_t unit_type_idx = 0;
      uint8_t last_purchased = INFINITY;
      while (true) {
        if (builds_left[dst_land] == 0) {
          if (factory_hp[dst_land] >= factory_max[dst_land])
            break;
          additional_cost = 1;
        }
        while (unit_type_idx < COST_UNIT_SEA_COUNT &&
               BUY_UNIT_SEA[unit_type_idx] <= last_purchased) {
          if (money[0] >= COST_UNIT_SEA[unit_type_idx] + additional_cost) {
            valid_purchases[valid_purchases_count++] = BUY_UNIT_SEA[unit_type_idx] + 1;
          }
          unit_type_idx++;
        }
        if (valid_purchases_count == 1) {
          builds_left[dst_sea] = 0;
          break;
        }
        uint8_t dst_air = dst_land + LANDS_COUNT;
        uint8_t purchase = get_user_purchase_input(dst_air, valid_purchases, valid_purchases_count);
        if (purchase == 0) {
          builds_left[dst_sea] = 0;
          break;
        }
        uint8_t unit_type_purchased = purchase - 1;
        factory_hp[dst_land] += additional_cost;
        for (uint8_t sea_idx2 = 0; sea_idx2 < LAND_TO_SEA_COUNT[dst_land]; sea_idx2++) {
          builds_left[LAND_TO_SEA_CONN[dst_land][sea_idx2]] += additional_cost - 1;
        }
        builds_left[dst_land] += additional_cost - 1;
        money[0] -= COST_UNIT_SEA[unit_type_purchased];
        units_sea_ptr[dst_sea][unit_type_purchased][0]++;
        units_sea_player_total[dst_sea][0]++;
        units_sea_grand_total[dst_sea]++;
        last_purchased = unit_type_purchased;
      }
    }
    // buy land units
    uint8_t valid_purchases[LAND_UNIT_TYPES + 1];
    valid_purchases[0] = 0; // pass all units
    uint8_t valid_purchases_count = 1;
    uint8_t unit_type = 0;
    uint8_t last_purchased = INFINITY;
    while (true) {
      if (builds_left[dst_land] == 0) {
        if (factory_hp[dst_land] >= factory_max[dst_land])
          break;
        additional_cost = 1;
        while (unit_type < LAND_UNIT_TYPES && unit_type <= last_purchased) {
          if (money[0] >= COST_UNIT_LAND[unit_type] + additional_cost) {
            valid_purchases[valid_purchases_count++] = unit_type + 1;
          }
          unit_type++;
        }
        uint8_t purchase =
            get_user_purchase_input(dst_land, valid_purchases, valid_purchases_count);
        if (purchase == 0) {
          builds_left[dst_land] = 0;
          break;
        }
        uint8_t unit_type_purchased = purchase - 1;
        factory_hp[dst_land] += additional_cost;
        builds_left[dst_land] += additional_cost - 1;
        money[0] -= COST_UNIT_LAND[unit_type_purchased];
        units_land_ptr[dst_land][unit_type_purchased][0]++;
        units_land_player_total[dst_land][0]++;
        units_land_grand_total[dst_land]++;
        last_purchased = unit_type_purchased;
      }
    }
  }
}
void crash_air_units() {
  // crash planes not on friendly land
  for (uint8_t air_idx = 0; air_idx < AIRS_COUNT; air_idx++) {
    uint8_t* total_fighter_count = &units_air_ptr[0][FIGHTERS][0];
    if (canFighterLandHere[air_idx]) {
      continue;
    }
    units_air_ptr[air_idx][FIGHTERS][0] = 0;
    continue;
  }
  // crash planes not on allied carriers
  for (uint8_t air_idx = LANDS_COUNT; air_idx < AIRS_COUNT; air_idx++) {
    // units_air_ptr[air_idx][BOMBERS_LAND_AIR][0] = 0;
    uint8_t sea_idx = air_idx - LANDS_COUNT;
    uint8_t free_space = allied_carriers[sea_idx] * 2;
    for (uint8_t player_idx = 0; player_idx < PLAYERS_COUNT - 1; player_idx++) {
      free_space -= other_sea_units[sea_idx][player_idx][FIGHTERS];
    }
    units_air_ptr[air_idx][FIGHTERS][0] = (units_air_ptr[air_idx][FIGHTERS][0] > free_space)
                                              ? free_space
                                              : units_air_ptr[air_idx][FIGHTERS][0];
  }
}
void reset_units_fully() {
  // reset battleship health
  for (uint8_t sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    units_sea_ptr[sea_idx][BATTLESHIPS][0] += units_sea_ptr[sea_idx][BS_DAMAGED][0];
    units_sea_ptr[sea_idx][BS_DAMAGED][0] = 0;
  }
}
// TODO BUY FACTORY
void buy_factory() {}
void collect_money() {
  // if player still owns their capital, collect income
  current_player_money +=
      (income_per_turn[0] * (owner_idx[current_player.capital_territory_index] == 0));
}
void rotate_turns() {
  // save cache back to arrays
  // current_player = (current_player + 1) % PLAYERS_COUNT;
  // current_player_is_human = is_human[current_player];
  // rotate arrays
  // set movement to full
  // set contruction remaining to full
}
