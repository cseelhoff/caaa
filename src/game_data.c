#include "game_data.h"
#include "canal.h"
#include "land.h"
#include "player.h"
#include "sea.h"
#include "serialize_data.h"
#include "units/artillery.h"
#include "units/transport.h"
#include "units/units.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define MIN_AIR_HOPS 2
#define MAX_AIR_HOPS 6
#define MIN_SEA_HOPS 1
#define MAX_SEA_HOPS 2
#define MIN_LAND_HOPS 1
#define MAX_LAND_HOPS 2
#define AIR_MOVE_SIZE 1 + MAX_AIR_HOPS - MIN_AIR_HOPS
#define SEA_MOVE_SIZE 1 + MAX_SEA_HOPS - MIN_SEA_HOPS
#define LAND_MOVE_SIZE 1 + MAX_LAND_HOPS - MIN_LAND_HOPS
#define MAX_MOVE_HISTORY AIRS_COUNT* AIRS_COUNT

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
uint8_t units_sea_blockade_total[SEAS_COUNT][PLAYERS_COUNT];
uint8_t units_sea_grand_total[SEAS_COUNT];
uint8_t* units_sea_ptr[SEAS_COUNT][SEA_UNIT_TYPES];
uint8_t* units_air_ptr[AIRS_COUNT][AIR_UNIT_TYPES];
uint8_t income_per_turn[PLAYERS_COUNT];
uint8_t enemies[PLAYERS_COUNT - 1];
uint8_t enemies_count;
uint8_t canal_state;
uint8_t gamestate_player_to_player_id[PLAYERS_COUNT];
bool is_allied[PLAYERS_COUNT];
char* LAND_NAMES[LANDS_COUNT] = {0};
char* SEA_NAMES[SEAS_COUNT] = {0};
char* AIR_NAMES[AIRS_COUNT] = {0};
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
uint8_t LOAD_WITHIN_1_MOVE[LANDS_COUNT][SEAS_COUNT] = {0};
uint8_t LOAD_WITHIN_1_MOVE_COUNT[LANDS_COUNT] = {0};
uint8_t LOAD_WITHIN_2_MOVES[LANDS_COUNT][SEAS_COUNT] = {0};
uint8_t LOAD_WITHIN_2_MOVES_COUNT[LANDS_COUNT] = {0};
uint8_t UNLOAD_WITHIN_1_MOVE[SEAS_COUNT][LANDS_COUNT] = {0};
uint8_t UNLOAD_WITHIN_1_MOVE_COUNT[SEAS_COUNT] = {0};
uint8_t SEAS_WITHIN_1_MOVE[CANALS_COUNT][SEAS_COUNT][SEAS_COUNT - 1] = {0};
uint8_t SEAS_WITHIN_1_MOVE_COUNT[CANALS_COUNT][SEAS_COUNT] = {0};
uint8_t SEAS_WITHIN_2_MOVES[CANALS_COUNT][SEAS_COUNT][SEAS_COUNT - 1] = {0};
uint8_t SEAS_WITHIN_2_MOVES_COUNT[CANALS_COUNT][SEAS_COUNT] = {0};
uint8_t AIR_WITHIN_1_MOVE[AIRS_COUNT][AIRS_COUNT - 1] = {0};
uint8_t AIR_WITHIN_1_MOVE_COUNT[AIRS_COUNT] = {0};
uint8_t AIR_WITHIN_2_MOVES[AIRS_COUNT][AIRS_COUNT - 1] = {0};
uint8_t AIR_WITHIN_2_MOVES_COUNT[AIRS_COUNT] = {0};
uint8_t AIR_WITHIN_3_MOVES[AIRS_COUNT][AIRS_COUNT - 1] = {0};
uint8_t AIR_WITHIN_3_MOVES_COUNT[AIRS_COUNT] = {0};
uint8_t AIR_WITHIN_4_MOVES[AIRS_COUNT][AIRS_COUNT - 1] = {0};
uint8_t AIR_WITHIN_4_MOVES_COUNT[AIRS_COUNT] = {0};
uint8_t AIR_WITHIN_5_MOVES[AIRS_COUNT][AIRS_COUNT - 1] = {0};
uint8_t AIR_WITHIN_5_MOVES_COUNT[AIRS_COUNT] = {0};
uint8_t AIR_WITHIN_6_MOVES[AIRS_COUNT][AIRS_COUNT - 1] = {0};
uint8_t AIR_WITHIN_6_MOVES_COUNT[AIRS_COUNT] = {0};
uint8_t SEA_DIST[CANAL_STATES][SEAS_COUNT][SEAS_COUNT] = {INFINITY};
uint8_t sea_dist[SEAS_COUNT][AIRS_COUNT] = {INFINITY};
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
uint8_t allied_carriers[SEAS_COUNT] = {0};
uint8_t canal_state;
uint8_t owner_idx[LANDS_COUNT];
uint8_t builds_left[LANDS_COUNT];
uint8_t factory_max[LANDS_COUNT];
uint8_t factory_hp[LANDS_COUNT];
uint8_t no_airstrip[LANDS_COUNT];
uint8_t other_land_units[LANDS_COUNT][PLAYERS_COUNT][LAND_UNIT_TYPES];
uint8_t other_sea_units[SEAS_COUNT][PLAYERS_COUNT][SEA_UNIT_TYPES - 1];
bool transports_with_large_cargo_space[SEAS_COUNT];
uint8_t zeros_transports_with_large_cargo_space[TRANS_EMPTY_STATES + TRANS_1I_STATES] = {0};
bool transports_with_small_cargo_space[SEAS_COUNT];
uint8_t zeros_transports_with_small_cargo_space[TRANS_EMPTY_STATES + TRANS_1I_STATES +
                                                TRANS_1A_STATES + TRANS_1T_STATES] = {0};
uint8_t skipped_territories[AIRS_COUNT][AIRS_COUNT] = {0};
uint8_t source_territories[AIRS_COUNT][AIRS_COUNT] = {0};
uint8_t source_territories_count[AIRS_COUNT] = {0};

bool canFighterLandHere[AIRS_COUNT];
bool canFighterLandIn1Move[AIRS_COUNT];
bool canBomberLandHere[AIRS_COUNT];
bool canBomberLandIn1Move[AIRS_COUNT];
bool canBomberLandIn2Moves[AIRS_COUNT];

uint8_t RANDOM_NUMBERS[10000] = {0};
int random_number_index = 0;

void initializeGameData() {
  // json = serialize_game_data_to_json(&gameData);
  // write_json_to_file("game_data_0.json", json);
  // cJSON_Delete(json);

  json = read_json_from_file("game_data.json");
  deserialize_game_data_from_json(&data, json);
  cJSON_Delete(json);

  // clear printableGameStatus
  current_player_index = data.player_index;
  current_player_index_plus_one = current_player_index + 1;
  current_player = Players[current_player_index];
  current_player_name = current_player.name;
  current_player_color = current_player.color;
  current_player_is_human = current_player.is_human;
  current_player_money = data.money[current_player_index];
  for (int player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    gamestate_player_to_player_id[player_idx] = (current_player_index + player_idx) % PLAYERS_COUNT;
    is_allied[player_idx] = current_player.is_allied[gamestate_player_to_player_id[player_idx]];
  }
  printableGameStatus[0] = '\0';
  generate_total_air_distance();
  generate_total_land_distance();
  generate_total_sea_distance();
  generate_seaMoveAllDestination();
  generate_airMoveAllDestination();
  refresh_cache();
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
  bombard_shores();
  // fire_strat_aa_guns();
  bomb_factories();
  // fire_tactical_aa_guns();
  resolve_land_battles();
  move_aa_guns();
  land_air_units();
  buy_units();
  crash_air_units();
  reset_units_fully();
  collect_money();
}

void generate_total_land_distance() {
  // Initialize the total_land_distance array
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {

    LAND_TO_LAND_COUNT[land_idx] = LANDS[land_idx].land_conn_count;
    for (int conn_idx = 0; conn_idx < LANDS[land_idx].land_conn_count; conn_idx++) {
      LAND_TO_LAND_CONN[land_idx][conn_idx] = LANDS[land_idx].connected_land_index[conn_idx];
    }
    LAND_NAMES[land_idx] = LANDS[land_idx].name;

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
        LANDS_WITHIN_1_MOVE[src_land][LANDS_WITHIN_1_MOVE_COUNT[src_land]] = dst_land;
        LANDS_WITHIN_1_MOVE_COUNT[src_land]++;
      }
      if (LAND_DIST[src_land][dst_land] <= 2) {
        LANDS_WITHIN_2_MOVES[src_land][LANDS_WITHIN_2_MOVES_COUNT[src_land]] = dst_land;
        LANDS_WITHIN_2_MOVES_COUNT[src_land]++;
      }
    }
    for (int dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
      if (LAND_DIST[src_land][dst_sea] <= 1) {
        LOAD_WITHIN_1_MOVE[src_land][LOAD_WITHIN_1_MOVE_COUNT[src_land]] = dst_sea;
        LOAD_WITHIN_1_MOVE_COUNT[src_land]++;
      }
      if (LAND_DIST[src_land][dst_sea] <= 2) {
        LOAD_WITHIN_2_MOVES[src_land][LOAD_WITHIN_2_MOVES_COUNT[src_land]] = dst_sea;
        LOAD_WITHIN_2_MOVES_COUNT[src_land]++;
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
                            [SEAS_WITHIN_1_MOVE_COUNT[canal_state_idx][src_sea]] = dst_sea;
          SEAS_WITHIN_1_MOVE_COUNT[canal_state_idx][src_sea]++;
        }
        if (SEA_DIST[canal_state_idx][src_sea][dst_sea] <= 2) {
          SEAS_WITHIN_2_MOVES[canal_state_idx][src_sea]
                             [SEAS_WITHIN_2_MOVES_COUNT[canal_state_idx][src_sea]] = dst_sea;
          SEAS_WITHIN_2_MOVES_COUNT[canal_state_idx][src_sea]++;
        }
      }
      for (int dst_land = 0; dst_land < LANDS_COUNT; dst_land++) {
        if (SEA_DIST[canal_state_idx][src_sea][dst_land] <= 1) {
          UNLOAD_WITHIN_1_MOVE[src_sea][UNLOAD_WITHIN_1_MOVE_COUNT[src_sea]] = dst_land;
          UNLOAD_WITHIN_1_MOVE_COUNT[src_sea]++;
        }
      }
    }
  }
  for (int src_air = 0; src_air < AIRS_COUNT; src_air++) {
    for (int dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
      if (src_air == dst_air) {
        continue;
      }
      if (AIR_DIST[src_air][dst_air] <= 1) {
        AIR_WITHIN_1_MOVE[src_air][AIR_WITHIN_1_MOVE_COUNT[src_air]] = dst_air;
        AIR_WITHIN_1_MOVE_COUNT[src_air]++;
      }
      if (AIR_DIST[src_air][dst_air] <= 2) {
        AIR_WITHIN_2_MOVES[src_air][AIR_WITHIN_2_MOVES_COUNT[src_air]] = dst_air;
        AIR_WITHIN_2_MOVES_COUNT[src_air]++;
      }
      if (AIR_DIST[src_air][dst_air] <= 3) {
        AIR_WITHIN_3_MOVES[src_air][AIR_WITHIN_3_MOVES_COUNT[src_air]] = dst_air;
        AIR_WITHIN_3_MOVES_COUNT[src_air]++;
      }
      if (AIR_DIST[src_air][dst_air] <= 4) {
        AIR_WITHIN_4_MOVES[src_air][AIR_WITHIN_4_MOVES_COUNT[src_air]] = dst_air;
        AIR_WITHIN_4_MOVES_COUNT[src_air]++;
      }
      if (AIR_DIST[src_air][dst_air] <= 5) {
        AIR_WITHIN_5_MOVES[src_air][AIR_WITHIN_5_MOVES_COUNT[src_air]] = dst_air;
        AIR_WITHIN_5_MOVES_COUNT[src_air]++;
      }
      if (AIR_DIST[src_air][dst_air] <= 6) {
        AIR_WITHIN_6_MOVES[src_air][AIR_WITHIN_6_MOVES_COUNT[src_air]] = dst_air;
        AIR_WITHIN_6_MOVES_COUNT[src_air]++;
      }
    }
  }
}
void generate_random_numbers() {
  for (int i = 0; i < 10000; i++) {
    RANDOM_NUMBERS[i] = rand() % 256;
  }
}
void refresh_canBomberLandHere() {
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    // is allied owned and not recently conquered?
    canBomberLandHere[land_idx] =
        (is_allied[data.land_state[land_idx].owner_idx] && !data.land_state[land_idx].no_airstrip);
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
        (is_allied[data.land_state[land_idx].owner_idx] && !data.land_state[land_idx].no_airstrip);
    // check for possiblity to build carrier under fighter
    if (data.land_state[land_idx].factory_max > 0 &&
        data.land_state[land_idx].owner_idx == current_player_index) {
      for (int conn_idx = 0; conn_idx < LANDS[land_idx].sea_conn_count; conn_idx++) {
        canFighterLandHere[LANDS_COUNT + LANDS[land_idx].connected_sea_index[conn_idx]] = true;
      }
    }
  }
  for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    if (allied_carriers[sea_idx] > 0) {
      canFighterLandHere[sea_idx] = true;
      // if player owns these carriers, then landing area is 2 spaces away
      if (units_sea_ptr[sea_idx][CARRIERS][CARRIER_MOVES_MAX] > 0) {
        for (int conn_idx = 0; conn_idx < SEAS[sea_idx].sea_conn_count; conn_idx++) {
          uint8_t connected_sea1 = SEAS[sea_idx].connected_sea_index[conn_idx];
          canFighterLandHere[LANDS_COUNT + connected_sea1] = true;
          for (int conn2_idx = 0; conn2_idx < SEAS[connected_sea1].sea_conn_count; conn2_idx++) {
            canFighterLandHere[LANDS_COUNT + SEAS[connected_sea1].connected_sea_index[conn2_idx]] =
                true;
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
void refresh_cache() {
  for (int player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    enemies_count = 0;
    if (current_player.is_allied[player_idx] == false) {
      enemies[enemies_count] = player_idx;
      enemies_count++;
    }

    // OPTIONAL FOR AI PLAY current_player_is_human==true
    player_names[player_idx] = Players[gamestate_player_to_player_id[player_idx]].name;
    player_colors[player_idx] = Players[gamestate_player_to_player_id[player_idx]].color;
    money[player_idx] = data.money[player_idx];
    // END OPTIONAL FOR AI PLAY
  }
  uint8_t unit_count;
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    owner_idx[land_idx] = data.land_state[land_idx].owner_idx;
    income_per_turn[owner_idx[land_idx]] += LANDS[land_idx].land_value;
    units_air_ptr[land_idx][FIGHTERS] = (uint8_t*)data.land_state[land_idx].fighters;
    units_air_ptr[land_idx][BOMBERS_LAND_AIR] = (uint8_t*)data.land_state[land_idx].bombers;
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
    units_land_grand_total[land_idx] = units_land_player_total[land_idx][0];
    for (int player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
      units_land_player_total[land_idx][player_idx] = 0;
      for (int land_unit_type = 0; land_unit_type < LAND_UNIT_TYPES; land_unit_type++) {
        unit_count = data.land_state[land_idx].other_units[player_idx][land_unit_type];
        other_land_units[land_idx][player_idx][land_unit_type] = unit_count;
        units_land_player_total[land_idx][player_idx] += unit_count;
      }
      units_land_grand_total[land_idx] += units_land_player_total[land_idx][player_idx];
    }
    builds_left[land_idx] = data.land_state[land_idx].builds_left;
    factory_max[land_idx] = data.land_state[land_idx].factory_max;
    factory_hp[land_idx] = data.land_state[land_idx].factory_hp;
    no_airstrip[land_idx] = data.land_state[land_idx].no_airstrip;
  }
  for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    uint8_t air_idx = sea_idx + LANDS_COUNT;
    units_air_ptr[air_idx][FIGHTERS] = (uint8_t*)data.units_sea[sea_idx].fighters;
    units_air_ptr[air_idx][BOMBERS_LAND_AIR] = (uint8_t*)data.units_sea[sea_idx].bombers;
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
        memcmp(&units_sea_ptr[sea_idx][TRANS_EMPTY][0], zeros_transports_with_large_cargo_space,
               (TRANS_EMPTY_STATES + TRANS_1I_STATES) * sizeof(uint8_t)) != 0;
    transports_with_small_cargo_space[sea_idx] =
        memcmp(&units_sea_ptr[sea_idx][TRANS_EMPTY][0], zeros_transports_with_small_cargo_space,
               (TRANS_EMPTY_STATES + TRANS_1I_STATES + TRANS_1A_STATES + TRANS_1T_STATES) *
                   sizeof(uint8_t)) != 0;

    units_sea_grand_total[sea_idx] = units_sea_player_total[sea_idx][0];
    for (int player_idx = 0; player_idx < PLAYERS_COUNT - 1; player_idx++) {
      for (int sea_unit_type = 0; sea_unit_type < SEA_UNIT_TYPES; sea_unit_type++) {
        unit_count = data.units_sea[sea_idx].other_units[player_idx][sea_unit_type];
        other_sea_units[sea_idx][player_idx][sea_unit_type] = unit_count;
        units_sea_player_total[sea_idx][player_idx] += unit_count;
      }
      allied_carriers[sea_idx] +=
          (other_sea_units[sea_idx][player_idx][CARRIERS] * is_allied[player_idx]);
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
    strcat(printableGameStatus, " No Airstrip: ");
    if (no_airstrip[land_index]) {
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
    printf("Enter a number between 0 and %d: ", valid_moves_count - 1);
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
      if (sscanf(buffer, "%d", &user_input) == 1 && user_input >= 0 &&
          user_input < valid_moves_count) {
        return (uint8_t)user_input;
      }
    }
  }
}
uint8_t getAIInput(uint8_t* valid_moves, int valid_moves_count) {
  return RANDOM_NUMBERS[random_number_index++] % valid_moves_count;
}

void add_valid_move_if_history_allows_1(uint8_t* valid_moves, uint8_t* valid_moves_count,
                                        uint8_t dst_air, uint8_t starting_land) {
  // get a list of all of the source territories that moved a unit into a territory that I can also
  // reach for each source territory, get a list of possible territory moves that were purposefully
  // skipped these skipped territories are also invalid moves
  bool has_checked_territory[AIRS_COUNT] = {0};
  for (int land_idx = 0; land_idx < LANDS_WITHIN_1_MOVE_COUNT[starting_land]; land_idx++) {
    uint8_t shared_dst = LANDS_WITHIN_1_MOVE[starting_land][land_idx];
    for (int i = 0; i < source_territories_count[shared_dst]; i++) {
      uint8_t src_air = source_territories[shared_dst][i];
      if (has_checked_territory[src_air]) {
        continue;
      }
      has_checked_territory[src_air] = true;
      if (skipped_territories[src_air][dst_air]) {
        return;
      }
    }
  }
  for (int sea_idx = 0; sea_idx < LOAD_WITHIN_1_MOVE_COUNT[starting_land]; sea_idx++) {
    uint8_t dst_sea = LOAD_WITHIN_1_MOVE[starting_land][sea_idx];
    uint8_t shared_dst = dst_sea + LANDS_COUNT;
    for (int i = 0; i < source_territories_count[shared_dst]; i++) {
      uint8_t src_air = source_territories[shared_dst][i];
      if (has_checked_territory[src_air]) {
        continue;
      }
      has_checked_territory[src_air] = true;
      if (skipped_territories[src_air][dst_air]) {
        return;
      }
    }
  }
  valid_moves[*valid_moves_count] = dst_air;
  (*valid_moves_count)++;
}

void add_valid_move_if_history_allows_2(uint8_t* valid_moves, uint8_t* valid_moves_count,
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
      if (skipped_territories[src_air][dst_air]) {
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
      if (skipped_territories[src_air][dst_air]) {
        return;
      }
    }
  }
  valid_moves[*valid_moves_count] = dst_air;
  (*valid_moves_count)++;
}

bool is_path_blocked(uint8_t src_land, uint8_t dst_land) {
  uint8_t nextLandMovement = land_path1[starting_land][dst_land];
  if (factory_max[nextLandMovement] > 0) {
    return true;
  }
  for (int enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
    if (units_land_player_total[nextLandMovement][enemies[enemy_idx]] > 0) {
      return true;
    }
  }
  nextLandMovement = LAND_PATH_ALT[starting_land][dst_land];
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
  memset(skipped_territories, 0, sizeof(skipped_territories));
  // memset(source_territories, 0, sizeof(source_territories));
  memset(source_territories_count, 0, sizeof(source_territories_count));
}

void stage_transport_units() {
  // loop through transports with "3" moves remaining (that aren't full),
  // start at sea 0 to n
  // TODO: optimize with cache - only loop through regions with transports
  for (int trans_type = TRANS_EMPTY; trans_type <= TRANS_1T; trans_type++) {
    uint8_t staging_state = STATES_MOVE_SEA[trans_type] - 1;
    uint8_t done_staging = staging_state - 1;
    // TODO CHECKPOINT
    for (int starting_sea = 0; starting_sea < SEAS_COUNT; starting_sea++) {
      while (units_sea_ptr[starting_sea][trans_type][staging_state] > 0) {
        uint8_t user_input;
        if (current_player_is_human) {
          setPrintableStatus();
          strcat(printableGameStatus, "Staging ");
          strcat(printableGameStatus, NAMES_UNIT_SEA[trans_type]);
          strcat(printableGameStatus, " From: ");
          strcat(printableGameStatus, SEA_NAMES[starting_sea]);
          strcat(printableGameStatus, " To: ");
          printf("%s\n", printableGameStatus);
          user_input = getUserInput();
        } else {
          // AI
          user_input = getAIInput();
        }
        // what is the actual destination that is a max of 2 sea moves away?
        uint8_t sea_destination = sea_path2[starting_sea][user_input];
        if (starting_sea == sea_destination || sea_destination == INFINITY) {
          units_sea_ptr[starting_sea][trans_type][done_staging]++;
          units_sea_ptr[starting_sea][trans_type][staging_state]--;
          continue;
        }
        // what is the actual sea distance between the two?
        uint8_t seaDistance = sea_dist[starting_sea][sea_destination];
        // if the distance is 2, is the primary path blocked?
        if (seaDistance == 2) {
          uint8_t nextSeaMovement = sea_path1[starting_sea][sea_destination];
          uint8_t nextSeaMovementAlt = nextSeaMovement;
          // check if the next sea movement has enemy ships
          bool hasEnemyShips = false;
          for (int enemy_index = 0; enemy_index < enemies_count; enemy_index++) {
            if (units_sea_blockade_total[nextSeaMovement][enemies[enemy_index]] > 0) {
              hasEnemyShips = true;
              nextSeaMovement = sea_path_alt1[starting_sea][sea_destination];
              break;
            }
          }
          if (hasEnemyShips && nextSeaMovementAlt != nextSeaMovement) {
            // check if the next sea movement has enemy ships
            hasEnemyShips = false;
            for (int enemy_index = 0; enemy_index < enemies_count; enemy_index++) {
              if (units_sea_blockade_total[nextSeaMovement][enemies[enemy_index]] > 0) {
                hasEnemyShips = true;
                break;
              }
            }
          }
          // if both paths are blocked, or dest is land
          // then move 1 space closer (where enemies are)
          if (hasEnemyShips || sea_destination < LANDS_COUNT) {
            sea_destination = nextSeaMovement;
          }
        }
        if (sea_destination < LANDS_COUNT) {
          units_sea_ptr[starting_sea][trans_type][done_staging]++;
          units_sea_ptr[starting_sea][trans_type][staging_state]--;
          continue;
        }
        units_sea_ptr[sea_destination][trans_type][done_staging - seaDistance]++;
        // units_sea_type_total[actualDestination][i]++;
        units_sea_player_total[sea_destination][0]++;
        units_sea_grand_total[sea_destination]++;
        units_sea_ptr[sea_destination][trans_type][staging_state]--;
        // units_sea_type_total[j][i]--;
        units_sea_player_total[starting_sea][0]--;
        units_sea_grand_total[starting_sea]--;
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
  refresh_canFighterLandHere();
  refresh_canFighterLandIn1Move();
  for (int starting_air = 0; starting_air < AIRS_COUNT; starting_air++) {
    while (units_air_ptr[starting_air][FIGHTERS][FIGHTER_MOVES_MAX] > 0) {
      uint8_t user_input;
      if (current_player_is_human) {
        setPrintableStatus();
        strcat(printableGameStatus, "Moving Fighter From: ");
        strcat(printableGameStatus, AIR_NAMES[starting_air]);
        strcat(printableGameStatus, " To: ");
        printf("%s\n", printableGameStatus);
        user_input = getUserInput();
      } else {
        // AI
        user_input = getAIInput();
      }
      // what is the actual destination that is a max of 4 air moves away?
      uint8_t air_destination = air_path4[starting_air][user_input];
      // check for bad move: if the actual destination is not water and is not
      // ally owned, and has no enemy units, then set actual destination to i
      if (air_destination < LANDS_COUNT && !is_allied[owner_idx[air_destination]] &&
          units_land_grand_total[air_destination] == 0) {
        air_destination = starting_air;
      }
      if (starting_air == air_destination) {
        units_air_ptr[starting_air][FIGHTERS][0]++;
        units_air_ptr[starting_air][FIGHTERS][FIGHTER_MOVES_MAX]--;
        continue;
      } else {
        // what is the actual air distance between the two?
        uint8_t airDistance = AIR_DIST[starting_air][air_destination];
        if (airDistance == 4) {
          if (!canFighterLandHere[air_destination]) {
            air_destination = air_path3[starting_air][user_input];
            airDistance = 3;
          }
        }
        if (airDistance == 3) {
          if (!canFighterLandIn1Move[air_destination]) {
            air_destination = air_path2[starting_air][user_input];
            airDistance = 2;
          }
        }
        units_air_ptr[air_destination][FIGHTERS][FIGHTER_MOVES_MAX - airDistance]++;
        if (air_destination < LANDS_COUNT) {
          units_land_player_total[air_destination][0]++;
          units_land_grand_total[air_destination]++;
          units_land_player_total[starting_air][0]--;
          units_land_grand_total[starting_air]--;
        } else {
          uint8_t seaDestination = air_destination - LANDS_COUNT;
          uint8_t starting_sea = starting_air - LANDS_COUNT;
          units_sea_player_total[seaDestination][0]++;
          units_sea_grand_total[seaDestination]++;
          units_sea_player_total[starting_sea][0]--;
          units_sea_grand_total[starting_sea]--;
        }
        units_air_ptr[starting_air][FIGHTERS][FIGHTER_MOVES_MAX]--;
      }
    }
  }
}
void move_bomber_units() {
  // check if any bombers have full moves remaining
  uint8_t* ptr = &units_land_ptr[0][BOMBERS_LAND_AIR][BOMBER_MOVES_MAX];
  if (memchr(ptr, 1, LANDS_COUNT * sizeof(uint8_t)) == NULL) {
    return;
  }
  refresh_canBomberLandHere();
  refresh_canBomberLandIn1Move();
  refresh_canBomberLandIn2Moves();
  for (int starting_land = 0; starting_land < LANDS_COUNT; starting_land++) {
    while (units_land_ptr[starting_land][BOMBERS_LAND_AIR][BOMBER_MOVES_MAX] > 0) {
      uint8_t user_input;
      if (current_player_is_human) {
        setPrintableStatus();
        strcat(printableGameStatus, "Moving Bomber From: ");
        strcat(printableGameStatus, LAND_NAMES[starting_land]);
        strcat(printableGameStatus, " To: ");
        printf("%s\n", printableGameStatus);
        user_input = getUserInput();
      } else {
        // AI
        user_input = getAIInput();
      }
      // what is the actual destination that is a max of 6 air moves away?
      uint8_t actualDestination = air_path6[starting_land][user_input];
      // check for bad move: if the actual destination has no enemy units and
      // is water or is not ally owned, then set actual destination to i
      if (actualDestination < LANDS_COUNT && !is_allied[owner_idx[actualDestination]] &&
          factory_hp[actualDestination] == 0 && units_land_grand_total[actualDestination] == 0) {
        actualDestination = starting_land;
      } else if (actualDestination >= LANDS_COUNT) {
        if (units_sea_grand_total[actualDestination] == 0) {
          actualDestination = starting_land;
        } else {
          for (int enemy_index = 0; enemy_index < enemies_count; enemy_index++) {
            if (units_sea_blockade_total[actualDestination][enemies[enemy_index]] > 0) {
              actualDestination = starting_land;
              break;
            }
          }
        }
      }
      if (starting_land == actualDestination) {
        units_air_ptr[starting_land][BOMBERS_LAND_AIR][0]++;
        units_air_ptr[starting_land][BOMBERS_LAND_AIR][BOMBER_MOVES_MAX]--;
        continue;
      } else {
        // what is the actual air distance between the two?
        uint8_t airDistance = AIR_DIST[starting_land][actualDestination];
        if (airDistance == 6) {
          if (!canBomberLandHere[actualDestination]) {
            actualDestination = air_path5[starting_land][user_input];
            airDistance = 5;
          }
        }
        if (airDistance == 5) {
          if (!canBomberLandIn1Move[actualDestination]) {
            actualDestination = air_path4[starting_land][user_input];
            airDistance = 4;
          }
        }
        if (airDistance == 4) {
          if (!canBomberLandIn2Moves[actualDestination]) {
            actualDestination = air_path3[starting_land][user_input];
            airDistance = 3;
          }
        }
        if (actualDestination < LANDS_COUNT) {
          units_land_ptr[actualDestination][BOMBERS_LAND_AIR][BOMBER_MOVES_MAX - airDistance]++;
          units_land_player_total[actualDestination][0]++;
          units_land_grand_total[actualDestination]++;
          units_land_player_total[starting_land][0]--;
          units_land_grand_total[starting_land]--;
        } else {
          uint8_t seaDestination = actualDestination - LANDS_COUNT;
          units_sea_ptr[seaDestination][BOMBERS_SEA][BOMBER_MOVES_MAX - 1 - airDistance]++;
          units_sea_player_total[seaDestination][0]++;
          units_sea_grand_total[seaDestination]++;
          units_sea_player_total[starting_land][0]--;
          units_sea_grand_total[starting_land]--;
        }
        units_land_ptr[starting_land][BOMBERS_LAND_AIR][BOMBER_MOVES_MAX]--;
      }
    }
  }
}

uint8_t get_user_input(uint8_t unit_type, uint8_t src_land, uint8_t* valid_moves,
                       uint8_t valid_moves_count) {
  uint8_t user_input;
  if (valid_moves_count == 1) {
    return valid_moves[0];
  }
  if (current_player_is_human) {
    setPrintableStatus();
    strcat(printableGameStatus, "Moving ");
    strcat(printableGameStatus, NAMES_UNIT_LAND[unit_type]);
    strcat(printableGameStatus, " From: ");
    strcat(printableGameStatus, LAND_NAMES[src_land]);
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

void update_move_history(uint8_t user_input, uint8_t src_land, uint8_t* valid_moves,
                         uint8_t valid_moves_count) {
  source_territories[user_input][source_territories_count[user_input]] = src_land;
  source_territories_count[user_input]++;
  for (int i = valid_moves_count - 1; i >= 0; i--) {
    if (valid_moves[i] == user_input) {
      break;
    } else {
      skipped_territories[src_land][valid_moves[i]] = true;
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
            memcmp(&units_sea_ptr[dst_sea][TRANS_EMPTY][0], zeros_transports_with_large_cargo_space,
                   (TRANS_EMPTY_STATES + TRANS_1I_STATES) * sizeof(uint8_t)) != 0;
        transports_with_small_cargo_space[dst_sea] =
            memcmp(&units_sea_ptr[dst_sea][TRANS_EMPTY][0], zeros_transports_with_small_cargo_space,
                   (TRANS_EMPTY_STATES + TRANS_1I_STATES + TRANS_1A_STATES + TRANS_1T_STATES) *
                       sizeof(uint8_t)) != 0;
        return true;
      }
      trans_state--;
    }
    trans_type--;
  }
  printf("Error: Failed to load tank onto transport\n");
}

void move_land_unit(uint8_t unit_type, uint8_t src_land, uint8_t user_input,
                    uint8_t moves_remaining) {
  uint8_t landDistance = LAND_DIST[src_land][user_input];
  bool hasEnemyBlockers = false;
  for (int enemy_idx = 0; enemy_idx < enemies_count; enemy_idx++) {
    if (units_land_player_total[user_input][enemies[enemy_idx]] > 0) {
      hasEnemyBlockers = true;
      break;
    }
  }
  // if the actual destination is not blitzable, then end unit turn
  if (is_allied[owner_idx[user_input]] || !hasEnemyBlockers) {
    landDistance = 2;
  }
  units_land_ptr[user_input][unit_type][moves_remaining - landDistance]++;
  units_land_player_total[user_input][0]++;
  units_land_grand_total[user_input]++;
  units_land_player_total[src_land][0]--;
  units_land_grand_total[src_land]--;
  units_land_ptr[src_land][unit_type][moves_remaining]--;
}

void add_valid_moves(uint8_t* valid_moves, uint8_t* valid_moves_count, uint8_t src_land,
                     uint8_t moves_remaining, uint8_t unit_type) {
  if (moves_remaining == 2) {
    // check for moving from land to land (two moves away)
    for (int land_idx = 0; land_idx < LANDS_WITHIN_2_MOVES_COUNT[src_land]; land_idx++) {
      uint8_t dst_land = LANDS_WITHIN_2_MOVES[src_land][land_idx];
      if (LAND_DIST[src_land][dst_land] == 1) {
        add_valid_move_if_history_allows_2(valid_moves, valid_moves_count, dst_land, src_land);
        continue;
      }
      if (is_path_blocked(src_land, dst_land)) {
        continue;
      }
      add_valid_move_if_history_allows_2(valid_moves, valid_moves_count, dst_land, src_land);
    }
    // check for moving from land to sea (two moves away)
    for (int sea_idx = 0; sea_idx < LOAD_WITHIN_2_MOVES_COUNT[src_land]; sea_idx++) {
      uint8_t dst_sea = LOAD_WITHIN_2_MOVES[src_land][sea_idx];
      if (!transports_with_large_cargo_space[dst_sea]) { // assume large, only tanks move 2
        continue;
      }
      uint8_t dst_air = dst_sea + LANDS_COUNT;
      if (LAND_DIST[src_land][dst_air] == 1) {
        add_valid_move_if_history_allows_2(valid_moves, valid_moves_count, dst_air, src_land);
        continue;
      }
      if (is_path_blocked(src_land, dst_air)) {
        continue;
      }
      add_valid_move_if_history_allows_2(valid_moves, valid_moves_count, dst_air, src_land);
    }
  } else {
    // check for moving from land to land (one move away)
    for (int land_idx = 0; land_idx < LANDS_WITHIN_1_MOVE_COUNT[src_land]; land_idx++) {
      uint8_t dst_land = LANDS_WITHIN_1_MOVE[src_land][land_idx];
      add_valid_move_if_history_allows_1(valid_moves, valid_moves_count, dst_land, src_land);
    }
    // check for moving from land to sea (one move away)
    for (int sea_idx = 0; sea_idx < LOAD_WITHIN_1_MOVE_COUNT[src_land]; sea_idx++) {
      uint8_t dst_sea = LOAD_WITHIN_1_MOVE[src_land][sea_idx];
      if (!transports_with_small_cargo_space[dst_sea]) {
        continue;
      }
      if (UNIT_WEIGHTS[unit_type] > 2 && !transports_with_large_cargo_space[dst_sea]) {
        continue;
      }
      add_valid_move_if_history_allows_1(valid_moves, valid_moves_count, dst_sea + LANDS_COUNT,
                                         src_land);
    }
  }
}

void move_land_unit_type(uint8_t unit_type) {
  clear_move_history();
  for (int src_land = 0; src_land < LANDS_COUNT; src_land++) {
    for (int moves_remaining = MAX_MOVE_LAND[unit_type]; moves_remaining > 0; moves_remaining--) {
      uint8_t valid_moves[AIRS_COUNT];
      valid_moves[0] = src_land;
      uint8_t valid_moves_count = 1;
      add_valid_moves(valid_moves, &valid_moves_count, src_land, moves_remaining, unit_type);
      while (units_land_ptr[src_land][unit_type][moves_remaining] > 0) {
        uint8_t user_input = get_user_input(unit_type, src_land, valid_moves, valid_moves_count);
        update_move_history(user_input, src_land, valid_moves, valid_moves_count);
        if (src_land == user_input) {
          units_land_ptr[src_land][unit_type][0]++;
          units_land_ptr[src_land][unit_type][moves_remaining]--;
          continue;
        }
        if (user_input >= LANDS_COUNT) {
          load_transport(unit_type, src_land, user_input - LANDS_COUNT);
          continue;
        }
        move_land_unit(unit_type, src_land, user_input, moves_remaining);
      }
    }
  }
}

void move_transport_units() {
  for (int trans_type = TRANS_1I; trans_type <= TRANS_1I_1T;
       trans_type++) { // there should be no TRANS_EMPTY
    int max_state = STATES_MOVE_SEA[trans_type] - STATES_STAGING[trans_type];
    int done_moving = STATES_UNLOADING[trans_type];
    int min_state = STATES_UNLOADING[trans_type] + 1;
    // TODO CHECKPOINT
    for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      for (int cur_state = max_state; cur_state >= min_state; cur_state--) {
        int moves_remaining = cur_state - STATES_UNLOADING[trans_type];
        while (units_sea_ptr[src_sea][trans_type][cur_state] > 0) {
          uint8_t user_input;
          if (current_player_is_human) {
            setPrintableStatus();
            strcat(printableGameStatus, "Moving ");
            strcat(printableGameStatus, NAMES_UNIT_SEA[trans_type]);
            strcat(printableGameStatus, " From: ");
            strcat(printableGameStatus, SEA_NAMES[src_sea]);
            strcat(printableGameStatus, " To: ");
            printf("%s\n", printableGameStatus);
            user_input = getUserInput();
          } else {
            // AI
            user_input = getAIInput();
          }
          uint8_t actualDestination;
          if (moves_remaining == 2) {
            // what is the actual destination that is a max of 2 sea moves away?
            actualDestination = sea_path2[src_sea][user_input];
          } else {
            // what is the actual destination that is a max of 1 sea moves away?
            actualDestination = sea_path1[src_sea][user_input];
          }
          if (src_sea == actualDestination || actualDestination == INFINITY) {
            units_sea_ptr[actualDestination][trans_type][done_moving]++;
            units_sea_ptr[src_sea][trans_type][cur_state]--;
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
            units_sea_ptr[src_sea][trans_type][done_moving]++;
            units_sea_ptr[src_sea][trans_type][cur_state]--;
            continue;
          }
          units_sea_ptr[actualDestination][trans_type][done_moving]++;
          // units_sea_type_total[actualDestination][i]++;
          units_sea_player_total[actualDestination][0]++;
          units_sea_grand_total[actualDestination]++;
          units_sea_ptr[actualDestination][trans_type][cur_state]--;
          // units_sea_type_total[j][i]--;
          units_sea_player_total[src_sea][0]--;
          units_sea_grand_total[src_sea]--;
        }
      }
    }
  }
}
void move_subs() {
  for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    while (units_sea_ptr[src_sea][SUBMARINES][SUB_UNMOVED] > 0) {
      uint8_t user_input;
      if (current_player_is_human) {
        setPrintableStatus();
        strcat(printableGameStatus, "Moving Submarine From: ");
        strcat(printableGameStatus, SEAS[src_sea].name);
        strcat(printableGameStatus, " To: ");
        printf("%s\n", printableGameStatus);
        user_input = getUserInput();
      } else {
        // AI
        user_input = getAIInput();
      }
      uint8_t actualDestination;
      // what is the actual destination that is a max of 2 sea moves away?
      actualDestination = sea_path2[src_sea][user_input];

      if (src_sea == actualDestination || actualDestination == INFINITY) {
        units_sea_ptr[actualDestination][SUBMARINES][SUB_DONE_MOVING]++;
        units_sea_ptr[src_sea][SUBMARINES][SUB_UNMOVED]--;
        continue;
      }
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
      units_sea_ptr[actualDestination][SUBMARINES][SUB_DONE_MOVING]++;
      units_sea_player_total[actualDestination][0]++;
      units_sea_grand_total[actualDestination]++;
      units_sea_ptr[actualDestination][SUBMARINES][SUB_UNMOVED]--;
      units_sea_player_total[src_sea][0]--;
      units_sea_grand_total[src_sea]--;
    }
  }
}

void move_destroyers_battleships() {

  for (int unit_type = DESTROYERS; unit_type <= BS_DAMAGED; unit_type++) {
    // TODO CHECKPOINT
    for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      while (units_sea_ptr[src_sea][unit_type][UNMOVED_SEA[unit_type]] > 0) {
        uint8_t user_input;
        if (current_player_is_human) {
          setPrintableStatus();
          strcat(printableGameStatus, "Moving ");
          strcat(printableGameStatus, NAMES_UNIT_SEA[unit_type]);
          strcat(printableGameStatus, " From: ");
          strcat(printableGameStatus, SEA_NAMES[src_sea]);
          strcat(printableGameStatus, " To: ");
          printf("%s\n", printableGameStatus);
          user_input = getUserInput();
        } else {
          // AI
          user_input = getAIInput();
        }
        uint8_t actualDestination;
        // what is the actual destination that is a max of 2 sea moves away?
        actualDestination = sea_path2[src_sea][user_input];

        if (src_sea == actualDestination || actualDestination == INFINITY) {
          units_sea_ptr[actualDestination][unit_type][DONE_MOVING_SEA[unit_type]]++;
          units_sea_ptr[src_sea][unit_type][UNMOVED_SEA[unit_type]]--;
          continue;
        }
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
          units_sea_ptr[src_sea][unit_type][UNMOVED_SEA[unit_type]]--;
          continue;
        }
        units_sea_ptr[actualDestination][unit_type][DONE_MOVING_SEA[unit_type]]++;
        units_sea_player_total[actualDestination][0]++;
        units_sea_grand_total[actualDestination]++;
        units_sea_ptr[actualDestination][unit_type][UNMOVED_SEA[unit_type]]--;
        units_sea_player_total[src_sea][0]--;
        units_sea_grand_total[src_sea]--;
      }
    }
  }
}
void resolve_sea_battles() {}
void unload_transports() {}
void bomb_factories() {}
void bombard_shores() {}
void fire_aa_guns() {}
void resolve_land_battles() {}
void land_air_units() {}
void move_aa_guns() {}
void reset_units_fully() {}
void buy_units() {}
void crash_air_units() {}
void collect_money() {}