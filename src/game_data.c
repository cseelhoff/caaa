#include "game_data.h"
#include "canal.h"
#include "land.h"
#include "player.h"
#include "sea.h"
#include "serialize_data.h"
#include "units/units.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_AIR_HOPS 2
#define MAX_AIR_HOPS 6
#define MIN_SEA_HOPS 1
#define MAX_SEA_HOPS 2
#define MIN_LAND_HOPS 1
#define MAX_LAND_HOPS 2

char buffer[STRING_BUFFER_SIZE];
char threeCharStr[5];
GameData gameData = {0};
cJSON* json;
// GameCache cache = {0};
uint8_t current_player_index;
uint8_t current_player_index_plus_one;
Player current_player;
char* current_player_name;
char printableGameStatus[5000] = "";
char* player_names[PLAYERS_COUNT];
char* player_colors[PLAYERS_COUNT];
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
bool current_player_is_allied[PLAYERS_COUNT];
UnitsSea units_sea;
LandState land_state;
int user_input;
Land land;
char* AIR_NAMES[AIRS_COUNT] = {0};
uint8_t AIR_CONN_COUNT[AIRS_COUNT] = {0};
uint8_t AIR_CONNECTIONS[AIRS_COUNT][MAX_AIR_TO_AIR_CONNECTIONS] = {0};
uint8_t total_air_distance[AIRS_COUNT][AIRS_COUNT] = {INFINITY};
uint8_t total_land_distance[LANDS_COUNT][AIRS_COUNT] = {INFINITY};
uint8_t total_sea_distance[CANAL_STATES][SEAS_COUNT][AIRS_COUNT] = {INFINITY};
uint8_t seaMoveXDestination[1 + MAX_SEA_HOPS - MIN_SEA_HOPS][CANAL_STATES]
                           [SEAS_COUNT][AIRS_COUNT] = {INFINITY};
uint8_t seaMove1DestinationAlt[CANAL_STATES][SEAS_COUNT][AIRS_COUNT] = {
    INFINITY};
uint8_t airMoveXDestination[1 + MAX_AIR_HOPS - MIN_AIR_HOPS][AIRS_COUNT]
                           [AIRS_COUNT] = {INFINITY};
uint8_t landMoveXDestination[LANDS_COUNT][AIRS_COUNT] = {INFINITY};
uint8_t landMove1DestinationAlt[LANDS_COUNT][AIRS_COUNT] = {INFINITY};
uint8_t allied_carriers[SEAS_COUNT] = {0};
uint8_t canal_state;
bool canFighterLandHere[AIRS_COUNT];
bool canFighterLandIn1Move[AIRS_COUNT];
bool canBomberLandHere[AIRS_COUNT];
bool canBomberLandIn1Move[AIRS_COUNT];
bool canBomberLandIn2Moves[AIRS_COUNT];

void initializeGameData() {
  // json = serialize_game_data_to_json(&gameData);
  // write_json_to_file("game_data_0.json", json);
  // cJSON_Delete(json);

  json = read_json_from_file("game_data.json");
  deserialize_game_data_from_json(&gameData, json);
  cJSON_Delete(json);

  // clear printableGameStatus
  current_player_index = gameData.player_index;
  current_player_index_plus_one = current_player_index + 1;
  current_player = Players[current_player_index];
  current_player_name = current_player.name;
  for (int player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    gamestate_player_to_player_id[player_idx] =
        (current_player_index + player_idx) % PLAYERS_COUNT;
    current_player_is_allied[player_idx] =
        current_player.is_allied[gamestate_player_to_player_id[player_idx]];
  }
  printableGameStatus[0] = '\0';
  build_total_air_distance();
  build_total_land_distance();
  build_total_sea_distance();
  build_seaMoveAllDestination();
  build_airMoveAllDestination();
  buildCache();
  move_fighter_units();
  move_bomber_units();
  stage_transport_units();
  move_tanks();
  move_artillery();
  move_infantry();
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

void updateAirMoveDst(int hop, int src_air, int dst_air, int cur_air,
                      int min_dist) {
  if (hop > MAX_AIR_HOPS)
    return;
  if (min_dist <= hop)
    airMoveXDestination[hop - MIN_AIR_HOPS][src_air][dst_air] = cur_air;
  for (int conn_idx = 0; conn_idx < AIR_CONN_COUNT[cur_air]; conn_idx++) {
    int next_air = AIR_CONNECTIONS[cur_air][conn_idx];
    int next_dist = total_air_distance[next_air][dst_air];
    if (next_dist < min_dist) {
      updateAirMoveDst(hop + 1, src_air, dst_air, next_air, next_dist);
    }
  }
}
void build_airMoveAllDestination() {
  for (int src_air = 0; src_air < AIRS_COUNT; src_air++) {
    for (int dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
      int min_dist = total_air_distance[src_air][dst_air];
      updateAirMoveDst(2, src_air, dst_air, src_air, min_dist);
    }
  }
}
void updateSeaMoveDst(int hop, int src_sea, int dst_air, int cur_sea,
                      int min_dist) {
  if (hop > MAX_SEA_HOPS)
    return;
  if (min_dist <= hop)
    seaMoveXDestination[hop - MIN_SEA_HOPS][canal_state][src_sea][dst_air] =
        cur_sea;
  for (int conn_idx = 0; conn_idx < SEAS[cur_sea].sea_conn_count; conn_idx++) {
    int next_sea = SEAS[cur_sea].connected_sea_index[conn_idx];
    int next_dist = total_sea_distance[canal_state][next_sea][dst_air];
    if (next_dist < min_dist) {
      updateSeaMoveDst(hop + 1, src_sea, dst_air, next_sea, next_dist);
    }
  }
}
void build_seaMoveAllDestination() {
  for (int canal_state = 0; canal_state < CANAL_STATES; canal_state++) {
    for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      for (int dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
        int min_dist = total_sea_distance[canal_state][src_sea][dst_air];
        updateSeaMoveDst(1, src_sea, dst_air, src_sea, min_dist);
        if (min_dist <= 1)
          seaMove1DestinationAlt[canal_state][src_sea][dst_air] = src_sea;
        for (int conn_idx = SEAS[src_sea].sea_conn_count; conn_idx >= 0;
             conn_idx--) {
          int next_sea = SEAS[src_sea].connected_sea_index[conn_idx];
          int next_dist = total_sea_distance[canal_state][next_sea][dst_air];
        }
      }
    }
  }
}
void updateLandMoveDst(int hop, int src_land, int dst_air, int cur_land,
                       int min_dist) {
  if (hop > MAX_LAND_HOPS)
    return;

  if (min_dist <= hop)
    landMoveXDestination[hop - MIN_LAND_HOPS][canal_state][src_land][dst_air] =
        cur_land;

  for (int conn_idx = 0; conn_idx < LANDS[cur_land].land_conn_count;
       conn_idx++) {
    int next_land = LANDS[cur_land].connected_land_index[conn_idx];
    int next_dist = total_land_distance[next_land][dst_air];
    if (next_dist < min_dist) {
      updateLandMoveDst(hop + 1, src_land, dst_air, next_land, next_dist);
    }
  }
}
void build_landMoveAllDestination() {
  for (int src_land = 0; src_land < LANDS_COUNT; src_land++) {
    for (int dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
      int min_dist = total_land_distance[src_land][dst_air];
      updateLandMoveDst(1, src_land, dst_air, src_land, min_dist);
      if (min_dist <= 1)
        landMove1DestinationAlt[src_land][dst_air] = src_land;
      for (int conn_idx = LANDS[src_land].land_conn_count; conn_idx >= 0;
           conn_idx--) {
        int next_land = LANDS[src_land].connected_land_index[conn_idx];
        int next_dist = total_land_distance[next_land][dst_air];
      }
    }
  }
}
void build_total_air_distance() {
  // Initialize the total_air_distance array
  for (int i = 0; i < AIRS_COUNT; i++) {
    for (int j = 0; j < AIRS_COUNT; j++) {
      if (i != j) {
        total_air_distance[i][j] = INFINITY;
      } // else {
        // total_air_distance[i][j] = 0;
      //}
    }
  }
  int air_index;
  // Populate initial distances based on connected_sea_index and
  // connected_land_index
  for (int i = 0; i < LANDS_COUNT; i++) {
    AIR_NAMES[i] = LANDS[i].name;
    for (int j = 0; j < LANDS[i].land_conn_count; j++) {
      air_index = LANDS[i].connected_land_index[j];
      AIR_CONNECTIONS[i][AIR_CONN_COUNT[i]] = air_index;
      AIR_CONN_COUNT[i]++;
      total_air_distance[i][air_index] = 1;
      total_air_distance[air_index][i] = 1;
    }
    for (int j = 0; j < LANDS[i].sea_conn_count; j++) {
      air_index = LANDS[i].connected_sea_index[j] + LANDS_COUNT;
      AIR_CONNECTIONS[i][AIR_CONN_COUNT[i]] = air_index;
      AIR_CONN_COUNT[i]++;
      total_air_distance[i][air_index] = 1;
      total_air_distance[air_index][i] = 1;
    }
  }

  for (int i = 0; i < SEAS_COUNT; i++) {
    AIR_NAMES[i + LANDS_COUNT] = SEAS[i].name;
    for (int j = 0; j < SEAS[i].land_conn_count; j++) {
      air_index = SEAS[i].connected_land_index[j];
      AIR_CONNECTIONS[i + LANDS_COUNT][AIR_CONN_COUNT[i + LANDS_COUNT]] =
          air_index;
      AIR_CONN_COUNT[i + LANDS_COUNT]++;
      total_air_distance[i + LANDS_COUNT][air_index] = 1;
      total_air_distance[air_index][i + LANDS_COUNT] = 1;
    }
    for (int j = 0; j < SEAS[i].sea_conn_count; j++) {
      air_index = SEAS[i].connected_sea_index[j] + LANDS_COUNT;
      AIR_CONNECTIONS[i + LANDS_COUNT][AIR_CONN_COUNT[i + LANDS_COUNT]] =
          air_index;
      AIR_CONN_COUNT[i + LANDS_COUNT]++;
      total_air_distance[i + LANDS_COUNT][air_index] = 1;
      total_air_distance[air_index][i + LANDS_COUNT] = 1;
    }
  }

  // Floyd-Warshall algorithm to compute shortest paths
  for (int k = 0; k < AIRS_COUNT; k++) {
    for (int i = 0; i < AIRS_COUNT; i++) {
      for (int j = 0; j < AIRS_COUNT; j++) {
        if (total_air_distance[i][k] + total_air_distance[k][j] <
            total_air_distance[i][j]) {
          total_air_distance[i][j] =
              total_air_distance[i][k] + total_air_distance[k][j];
        }
      }
    }
  }
}
void build_total_land_distance() {
  // Initialize the total_land_distance array
  for (int i = 0; i < LANDS_COUNT; i++) {
    for (int j = 0; j < AIRS_COUNT; j++) {
      if (i != j) {
        total_land_distance[i][j] = INFINITY;
      }
    }
  }

  // Populate initial distances based on connected_sea_index
  for (int i = 0; i < LANDS_COUNT; i++) {
    for (int j = 0; j < LANDS[i].land_conn_count; j++) {
      int land_index = LANDS[i].connected_land_index[j];
      total_land_distance[i][land_index] = 1;
      total_land_distance[land_index][i] = 1;
    }
    for (int j = 0; j < LANDS[i].sea_conn_count; j++) {
      int air_index = LANDS[i].connected_sea_index[j] + LANDS_COUNT;
      total_land_distance[i][air_index] = 1;
      // total_land_distance[sea_index][i] = 1;
    }
  }

  // Floyd-Warshall algorithm to compute shortest paths
  for (int i = 0; i < LANDS_COUNT; i++) {
    for (int j = 0; j < LANDS_COUNT; j++) {
      for (int k = 0; k < AIRS_COUNT; k++) {
        if (total_land_distance[j][i] + total_land_distance[i][k] <
            total_land_distance[j][k]) {
          total_land_distance[j][k] =
              total_land_distance[j][i] + total_land_distance[i][k];
        }
      }
    }
  }
}
void build_total_sea_distance() {
  for (int i = 0; i < CANAL_STATES; i++) {
    // Initialize the total_sea_distance array
    for (int j = 0; j < SEAS_COUNT; j++) {
      for (int k = 0; k < AIRS_COUNT; k++) {
        if (j != k) {
          total_sea_distance[i][j][k] = INFINITY;
        } // else {
          // total_sea_distance[i][j] = 0;
        //}
      }
    }
    int air_index;
    // Populate initial distances based on connected_sea_index
    for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      for (int conn_idx = 0; conn_idx < SEAS[src_sea].sea_conn_count;
           conn_idx++) {
        int sea_index = SEAS[src_sea].connected_sea_index[conn_idx];
        total_sea_distance[i][src_sea][sea_index + LANDS_COUNT] = 1;
        total_sea_distance[i][sea_index][src_sea + LANDS_COUNT] = 1;
      }
      for (int conn_idx = 0; conn_idx < SEAS[src_sea].land_conn_count;
           conn_idx++) {
        int air_index = SEAS[src_sea].connected_land_index[conn_idx];
        total_sea_distance[i][src_sea][air_index] = 1;
        // total_sea_distance[i][land_index][j] = 1;
      }
    }

    // convert canal_state to a bitmask and loop through CANALS for those
    // enabled for example if canal_state is 0, do not process any items in
    // CANALS, if canal_state is 1, process the first item in CANALS, if
    // canal_state is 2, process the second item in CANALS, if canal_state is
    // 3, process the first and second items in CANALS, etc.
    for (int j = 0; j < CANALS_COUNT; j++) {
      if ((i & (1 << j)) == 0) {
        continue;
      }
      total_sea_distance[i][CANALS[j].seas[0]][CANALS[j].seas[1]] = 1;
      total_sea_distance[i][CANALS[j].seas[1]][CANALS[j].seas[0]] = 1;
    }

    // Floyd-Warshall algorithm to compute shortest paths
    for (int j = 0; j < SEAS_COUNT; j++) {
      for (int k = 0; k < SEAS_COUNT; k++) {
        for (int l = 0; l < AIRS_COUNT; l++) {
          if (total_sea_distance[i][k][j + LANDS_COUNT] +
                  total_sea_distance[i][j][l] <
              total_sea_distance[i][k][l]) {
            total_sea_distance[i][k][l] =
                total_sea_distance[i][k][j + LANDS_COUNT] +
                total_sea_distance[i][j][l];
          }
        }
      }
    }
  }
}
void build_canBomberLandHere() {
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    // is allied owned and not recently conquered?
    canBomberLandHere[land_idx] =
        (current_player_is_allied[gameData.land_state[land_idx].owner_index] &&
         !gameData.land_state[land_idx].recently_conquered);
  }
}
void build_canBomberLandIn1Move() {
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    canBomberLandIn1Move[land_idx] = false;
    for (int conn_idx = 0; conn_idx < LANDS[land_idx].land_conn_count;
         conn_idx++) {
      if (canBomberLandHere[LANDS[land_idx].connected_land_index[conn_idx]]) {
        canBomberLandIn1Move[land_idx] = true;
        break;
      }
    }
  }
  for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    canBomberLandIn1Move[LANDS_COUNT + sea_idx] = false;
    for (int conn_idx = 0; conn_idx < SEAS[sea_idx].land_conn_count;
         conn_idx++) {
      if (canBomberLandHere[SEAS[sea_idx].connected_land_index[conn_idx]]) {
        canBomberLandIn1Move[LANDS_COUNT + sea_idx] = true;
        break;
      }
    }
  }
}
void build_canBomberLandIn2Moves() {
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
void build_canFighterLandHere() {
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    // is allied owned and not recently conquered?
    canFighterLandHere[land_idx] =
        (current_player_is_allied[gameData.land_state[land_idx].owner_index] &&
         !gameData.land_state[land_idx].recently_conquered);
    // check for possiblity to build carrier under fighter
    if (gameData.land_state[land_idx].factory_max > 0 &&
        gameData.land_state[land_idx].owner_index == current_player_index) {
      for (int conn_idx = 0; conn_idx < LANDS[land_idx].sea_conn_count;
           conn_idx++) {
        canFighterLandHere[LANDS_COUNT +
                           LANDS[land_idx].connected_sea_index[conn_idx]] =
            true;
      }
    }
  }
  for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    if (allied_carriers[sea_idx] > 0) {
      canFighterLandHere[sea_idx] = true;
      // if player owns these carriers, then landing area is 2 spaces away
      if (units_sea_ptr[sea_idx][CARRIERS][CARRIER_MOVES_MAX] > 0) {
        for (int conn_idx = 0; conn_idx < SEAS[sea_idx].sea_conn_count;
             conn_idx++) {
          uint8_t connected_sea1 = SEAS[sea_idx].connected_sea_index[conn_idx];
          canFighterLandHere[LANDS_COUNT + connected_sea1] = true;
          for (int conn2_idx = 0;
               conn2_idx < SEAS[connected_sea1].sea_conn_count; conn2_idx++) {
            canFighterLandHere[LANDS_COUNT +
                               SEAS[connected_sea1]
                                   .connected_sea_index[conn2_idx]] = true;
          }
        }
      }
    }
  }
}
void build_canFighterLandIn1Move() {
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
void buildCache() {
  for (int player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    enemies_count = 0;
    player_names[player_idx] =
        Players[gamestate_player_to_player_id[player_idx]].name;
    player_colors[player_idx] =
        Players[gamestate_player_to_player_id[player_idx]].color;
    if (current_player.is_allied[player_idx] == false) {
      enemies[enemies_count] = player_idx;
      enemies_count++;
    }
  }
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    income_per_turn[gameData.land_state[land_idx].owner_index] +=
        LANDS[land_idx].land_value;
    units_air_ptr[land_idx][FIGHTERS] =
        (uint8_t*)gameData.land_state[land_idx].fighters;
    units_air_ptr[land_idx][BOMBERS_LAND_AIR] =
        (uint8_t*)gameData.land_state[land_idx].bombers;
    units_land_ptr[land_idx][FIGHTERS] =
        (uint8_t*)gameData.land_state[land_idx].fighters;
    units_land_ptr[land_idx][BOMBERS_LAND_AIR] =
        (uint8_t*)gameData.land_state[land_idx].bombers;
    units_land_ptr[land_idx][INFANTRY] =
        (uint8_t*)gameData.land_state[land_idx].infantry;
    units_land_ptr[land_idx][ARTILLERY] =
        (uint8_t*)gameData.land_state[land_idx].artillery;
    units_land_ptr[land_idx][TANKS] =
        (uint8_t*)gameData.land_state[land_idx].tanks;
    units_land_ptr[land_idx][AA_GUNS] =
        (uint8_t*)gameData.land_state[land_idx].aa_guns;
    units_land_grand_total[land_idx] = units_land_player_total[land_idx][0];
    units_land_grand_total[land_idx] = units_land_player_total[land_idx][0];
    for (int player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      for (int land_unit_type = 0; land_unit_type < LAND_UNIT_TYPES;
           land_unit_type++) {
        units_land_player_total[land_idx][player_idx] +=
            gameData.land_state[land_idx]
                .other_units[player_idx][land_unit_type];
      }
      units_land_grand_total[land_idx] +=
          units_land_player_total[land_idx][player_idx];
    }
  }
  for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    units_air_ptr[sea_idx + LANDS_COUNT][FIGHTERS] =
        (uint8_t*)gameData.units_sea[sea_idx].fighters;
    units_air_ptr[sea_idx + LANDS_COUNT][BOMBERS_LAND_AIR] =
        (uint8_t*)gameData.units_sea[sea_idx].bombers;
    units_sea_ptr[sea_idx][FIGHTERS] =
        (uint8_t*)gameData.units_sea[sea_idx].fighters;
    units_sea_ptr[sea_idx][TRANS_EMPTY] =
        (uint8_t*)gameData.units_sea[sea_idx].trans_empty;
    units_sea_ptr[sea_idx][TRANS_1I] =
        (uint8_t*)gameData.units_sea[sea_idx].trans_1i;
    units_sea_ptr[sea_idx][TRANS_1A] =
        (uint8_t*)gameData.units_sea[sea_idx].trans_1a;
    units_sea_ptr[sea_idx][TRANS_1T] =
        (uint8_t*)gameData.units_sea[sea_idx].trans_1t;
    units_sea_ptr[sea_idx][TRANS_2I] =
        (uint8_t*)gameData.units_sea[sea_idx].trans_2i;
    units_sea_ptr[sea_idx][TRANS_1I_1A] =
        (uint8_t*)gameData.units_sea[sea_idx].trans_1i_1a;
    units_sea_ptr[sea_idx][TRANS_1I_1T] =
        (uint8_t*)gameData.units_sea[sea_idx].trans_1i_1t;
    units_sea_ptr[sea_idx][SUBMARINES] =
        (uint8_t*)gameData.units_sea[sea_idx].submarines;
    units_sea_ptr[sea_idx][DESTROYERS] =
        (uint8_t*)gameData.units_sea[sea_idx].destroyers;
    units_sea_ptr[sea_idx][CARRIERS] =
        (uint8_t*)gameData.units_sea[sea_idx].carriers;
    allied_carriers[sea_idx] = gameData.units_sea[sea_idx].carriers[0] +
                               gameData.units_sea[sea_idx].carriers[1];
    units_sea_ptr[sea_idx][CRUISERS] =
        (uint8_t*)gameData.units_sea[sea_idx].cruisers;
    units_sea_ptr[sea_idx][BATTLESHIPS] =
        (uint8_t*)gameData.units_sea[sea_idx].battleships;
    units_sea_ptr[sea_idx][BS_DAMAGED] =
        (uint8_t*)gameData.units_sea[sea_idx].bs_damaged;
    units_sea_ptr[sea_idx][BOMBERS_SEA] =
        (uint8_t*)gameData.units_sea[sea_idx].bombers;
    units_sea_grand_total[sea_idx] = units_sea_player_total[sea_idx][0];
    for (int player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      for (int sea_unit_type = 0; sea_unit_type < SEA_UNIT_TYPES;
           sea_unit_type++) {
        units_sea_player_total[sea_idx][player_idx] +=
            gameData.units_sea[sea_idx].other_units[player_idx][sea_unit_type];
      }
      allied_carriers[sea_idx] +=
          (gameData.units_sea[sea_idx].other_units[player_idx][CARRIERS] *
           current_player_is_allied[player_idx]);
      units_sea_grand_total[sea_idx] +=
          units_sea_player_total[sea_idx][player_idx];
    }
  }
  canal_state = 0;
  for (int canal_idx = 0; canal_idx < CANALS_COUNT; canal_idx++) {
    if (current_player.is_allied[gameData.land_state[CANALS[canal_idx].lands[0]]
                                     .owner_index] &&
        current_player.is_allied[gameData.land_state[CANALS[canal_idx].lands[1]]
                                     .owner_index]) {
      canal_state += 1 << canal_idx;
    }
  }
}
void setPrintableStatus() {
  strcat(printableGameStatus, current_player.color);
  strcat(printableGameStatus, current_player_name);
  strcat(printableGameStatus, "\033[0m");
  strcat(printableGameStatus, " is in phase ");
  //  strcat(printableGameStatus, PHASES[gameData.phase]);
  strcat(printableGameStatus, " with money ");
  sprintf(threeCharStr, "%d", gameData.money[current_player_index]);
  strcat(printableGameStatus, threeCharStr);
  strcat(printableGameStatus, "\n");
  setPrintableStatusLands();
  setPrintableStatusSeas();
}
void setPrintableStatusLands() {
  for (int land_index = 0; land_index < LANDS_COUNT; land_index++) {
    //    LandState land_state = gameData.land_state[i];
    strcat(printableGameStatus,
           Players[gameData.land_state[land_index].owner_index].color);
    sprintf(threeCharStr, "%d ", land_index);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, LANDS[land_index].name);
    strcat(printableGameStatus, ": ");
    strcat(printableGameStatus,
           Players[gameData.land_state[land_index].owner_index].name);
    strcat(printableGameStatus, " ");
    sprintf(threeCharStr, "%d", gameData.land_state[land_index].builds_left);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, "/");
    sprintf(threeCharStr, "%d", gameData.land_state[land_index].factory_hp);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, "/");
    sprintf(threeCharStr, "%d", gameData.land_state[land_index].factory_max);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, " Bombarded: ");
    if (gameData.land_state[land_index].bombard_max) {
      strcat(printableGameStatus, "true\n");
    } else {
      strcat(printableGameStatus, "false\n");
    }
    if (units_land_grand_total[land_index] == 0) {
      strcat(printableGameStatus, "\033[0m");
      continue;
    }
    strcat(printableGameStatus, "             |Tot| 0| 1| 2| 3| 4| 5| 6|\n");
    for (int land_unit_idx = 0; land_unit_idx < LAND_UNIT_TYPES;
         land_unit_idx++) {
      bool units_present = false;
      for (int unit_state = 0; unit_state < STATES_MOVE_LAND[land_unit_idx];
           unit_state++) {
        if (units_land_ptr[land_index][land_unit_idx][unit_state] > 0) {
          units_present = true;
          break;
        }
      }
      if (units_present) {
        strcat(printableGameStatus, current_player.name);
        strcat(printableGameStatus, NAMES_UNIT_LAND[land_unit_idx]);
        for (int k = 0; k < STATES_MOVE_LAND[land_unit_idx]; k++) {
          sprintf(threeCharStr, "%3d",
                  units_land_ptr[land_index][land_unit_idx][k]);
          strcat(printableGameStatus, threeCharStr);
        }
        strcat(printableGameStatus, "\n");
      }
    }
    strcat(printableGameStatus, "\033[0m");
    for (int player_idx = 0; player_idx < PLAYERS_COUNT - 1; player_idx++) {
      if (units_land_player_total[land_index][player_idx] > 0) {
        strcat(printableGameStatus, player_colors[player_idx]);
        for (int land_unit_idx = 0; land_unit_idx < LAND_UNIT_TYPES;
             land_unit_idx++) {
          if (gameData.land_state[land_index]
                  .other_units[player_idx][land_unit_idx] > 0) {
            strcat(printableGameStatus, player_names[player_idx]);
            strcat(printableGameStatus, NAMES_UNIT_LAND[land_unit_idx]);
            sprintf(threeCharStr, "%3d",
                    gameData.land_state[land_index]
                        .other_units[player_idx][land_unit_idx]);
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
    units_sea = gameData.units_sea[sea_index];
    sprintf(threeCharStr, "%d ", LANDS_COUNT + sea_index);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, SEAS[sea_index].name);
    strcat(printableGameStatus, "             |Tot| 0| 1| 2| 3| 4| 5| 6|\n");
    strcat(printableGameStatus, current_player.color);
    if (units_sea_player_total[sea_index][0] > 0) {
      //      uint8_t* units_sea_mobile_total = units_sea_type_total[i];
      for (int j = 0; j < SEA_UNIT_TYPES; j++) {
        bool units_present = false;
        for (int k = 0; k < STATES_MOVE_SEA[j]; k++) {
          if (units_sea_ptr[sea_index][j][k] > 0) {
            units_present = true;
            break;
          }
        }
        if (units_present) {
          strcat(printableGameStatus, current_player.name);
          strcat(printableGameStatus, NAMES_UNIT_SEA[j]);
          for (int k = 0; k < STATES_MOVE_SEA[j]; k++) {
            sprintf(threeCharStr, "%3d", units_sea_ptr[sea_index][j][k]);
            strcat(printableGameStatus, threeCharStr);
          }
          strcat(printableGameStatus, "\n");
        }
      }
      strcat(printableGameStatus, "\033[0m");
      for (int player_idx = 0; player_idx < PLAYERS_COUNT - 1; player_idx++) {
        strcat(printableGameStatus, player_colors[player_idx]);
        for (int sea_unit_idx = 0; sea_unit_idx < SEA_UNIT_TYPES;
             sea_unit_idx++) {
          if (units_sea.other_units[player_idx][sea_unit_idx] > 0) {
            strcat(printableGameStatus, player_names[player_idx]);
            strcat(printableGameStatus, NAMES_UNIT_SEA[sea_unit_idx]);
            sprintf(threeCharStr, "%3d",
                    units_sea.other_units[player_idx][sea_unit_idx]);
            strcat(printableGameStatus, threeCharStr);
            strcat(printableGameStatus, "\n");
          }
        }
        strcat(printableGameStatus, "\033[0m");
      }
    }
  }
}
void getUserInput() {
  while (true) {
    if (fgets(threeCharStr, 3, stdin) != NULL) {
      if (sscanf(buffer, "%d", &user_input) == 1) {
        break;
      }
    }
  }
}
void getAIInput() {
  user_input = 0;
  return;
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
        if (current_player.is_human) {
          setPrintableStatus();
          strcat(printableGameStatus, "Staging ");
          strcat(printableGameStatus, NAMES_UNIT_SEA[trans_type]);
          strcat(printableGameStatus, " From: ");
          strcat(printableGameStatus, SEAS[starting_sea].name);
          strcat(printableGameStatus, " To: ");
          printf("%s\n", printableGameStatus);
          getUserInput();
        } else {
          // AI
          getAIInput();
        }
        // what is the actual destination that is a max of 2 sea moves away?
        uint8_t sea_destination =
            seaMoveXDestination[2 - MIN_SEA_HOPS][canal_state][starting_sea]
                               [user_input];
        if (starting_sea == sea_destination || sea_destination == INFINITY) {
          units_sea_ptr[starting_sea][trans_type][done_staging]++;
          units_sea_ptr[starting_sea][trans_type][staging_state]--;
          continue;
        }
        // what is the actual sea distance between the two?
        uint8_t seaDistance =
            total_sea_distance[canal_state][starting_sea][sea_destination];
        // if the distance is 2, is the primary path blocked?
        if (seaDistance == 2) {
          uint8_t nextSeaMovement =
              seaMoveXDestination[1 - MIN_SEA_HOPS][canal_state][starting_sea]
                                 [sea_destination];
          uint8_t nextSeaMovementAlt = nextSeaMovement;
          // check if the next sea movement has enemy ships
          bool hasEnemyShips = false;
          for (int enemy_index = 0; enemy_index < enemies_count;
               enemy_index++) {
            if (units_sea_blockade_total[nextSeaMovement]
                                        [enemies[enemy_index]] > 0) {
              hasEnemyShips = true;
              nextSeaMovement =
                  seaMove1DestinationAlt[canal_state][starting_sea]
                                        [sea_destination];
              break;
            }
          }
          if (hasEnemyShips && nextSeaMovementAlt != nextSeaMovement) {
            // check if the next sea movement has enemy ships
            hasEnemyShips = false;
            for (int enemy_index = 0; enemy_index < enemies_count;
                 enemy_index++) {
              if (units_sea_blockade_total[nextSeaMovement]
                                          [enemies[enemy_index]] > 0) {
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
        units_sea_ptr[sea_destination][trans_type]
                     [done_staging - seaDistance]++;
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
  uint8_t *ptr = &units_air_ptr[0][FIGHTERS][FIGHTER_MOVES_MAX];
  if (memchr(ptr, 1, AIRS_COUNT * sizeof(uint8_t)) == NULL) {
      return;
  }
  build_canFighterLandHere();
  build_canFighterLandIn1Move();
  for (int starting_air = 0; starting_air < AIRS_COUNT; starting_air++) {
    while (units_air_ptr[starting_air][FIGHTERS][FIGHTER_MOVES_MAX] > 0) {
      if (current_player.is_human) {
        setPrintableStatus();
        strcat(printableGameStatus, "Moving Fighter From: ");
        strcat(printableGameStatus, AIR_NAMES[starting_air]);
        strcat(printableGameStatus, " To: ");
        printf("%s\n", printableGameStatus);
        getUserInput();
      } else {
        // AI
        getAIInput();
      }
      // what is the actual destination that is a max of 4 air moves away?
      uint8_t air_destination =
          airMoveXDestination[4 - MIN_AIR_HOPS][starting_air][user_input];
      // check for bad move: if the actual destination is not water and is not
      // ally owned, and has no enemy units, then set actual destination to i
      if (air_destination < LANDS_COUNT &&
          !current_player_is_allied[gameData.land_state[air_destination]
                                        .owner_index] &&
          units_land_grand_total[air_destination] == 0) {
        air_destination = starting_air;
      }
      if (starting_air == air_destination) {
        units_air_ptr[starting_air][FIGHTERS][0]++;
        units_air_ptr[starting_air][FIGHTERS][FIGHTER_MOVES_MAX]--;
        continue;
      } else {
        // what is the actual air distance between the two?
        uint8_t airDistance = total_air_distance[starting_air][air_destination];
        if (airDistance == 4) {
          if (!canFighterLandHere[air_destination]) {
            air_destination =
                airMoveXDestination[3 - MIN_AIR_HOPS][starting_air][user_input];
            airDistance = 3;
          }
        }
        if (airDistance == 3) {
          if (!canFighterLandIn1Move[air_destination]) {
            air_destination =
                airMoveXDestination[2 - MIN_AIR_HOPS][starting_air][user_input];
            airDistance = 2;
          }
        }
        units_air_ptr[air_destination][FIGHTERS]
                     [FIGHTER_MOVES_MAX - airDistance]++;
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
  uint8_t *ptr = &units_land_ptr[0][BOMBERS_LAND_AIR][BOMBER_MOVES_MAX];
  if (memchr(ptr, 1, LANDS_COUNT * sizeof(uint8_t)) == NULL) {
      return;
  }
  build_canBomberLandHere();
  build_canBomberLandIn1Move();
  build_canBomberLandIn2Moves();
  for (int starting_land = 0; starting_land < LANDS_COUNT; starting_land++) {
    while (units_land_ptr[starting_land][BOMBERS_LAND_AIR][BOMBER_MOVES_MAX] >
           0) {
      if (current_player.is_human) {
        setPrintableStatus();
        strcat(printableGameStatus, "Moving Bomber From: ");
        strcat(printableGameStatus, LANDS[starting_land].name);
        strcat(printableGameStatus, " To: ");
        printf("%s\n", printableGameStatus);
        getUserInput();
      } else {
        // AI
        getAIInput();
      }
      // what is the actual destination that is a max of 6 air moves away?
      uint8_t actualDestination =
          airMove6Destination[starting_land][user_input];
      // check for bad move: if the actual destination has no enemy units and
      // is water or is not ally owned, then set actual destination to i
      if (actualDestination < LANDS_COUNT &&
          !current_player_is_allied[gameData.land_state[actualDestination]
                                        .owner_index] &&
          gameData.land_state[actualDestination].factory_hp == 0 &&
          units_land_grand_total[actualDestination] == 0) {
        actualDestination = starting_land;
      } else if (actualDestination >= LANDS_COUNT) {
        if (units_sea_grand_total[actualDestination] == 0) {
          actualDestination = starting_land;
        } else {
          for (int enemy_index = 0; enemy_index < enemies_count;
               enemy_index++) {
            if (units_sea_blockade_total[actualDestination]
                                        [enemies[enemy_index]] > 0) {
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
        uint8_t airDistance =
            total_air_distance[starting_land][actualDestination];
        if (airDistance == 6) {
          if (!canBomberLandHere[actualDestination]) {
            actualDestination = airMove5Destination[starting_land][user_input];
            airDistance = 5;
          }
        }
        if (airDistance == 5) {
          if (!canBomberLandIn1Move[actualDestination]) {
            actualDestination = airMove4Destination[starting_land][user_input];
            airDistance = 4;
          }
        }
        if (airDistance == 4) {
          if (!canBomberLandIn2Moves[actualDestination]) {
            actualDestination = airMove3Destination[starting_land][user_input];
            airDistance = 3;
          }
        }
        if (actualDestination < LANDS_COUNT) {
          units_land_ptr[actualDestination][BOMBERS_LAND_AIR]
                        [BOMBER_MOVES_MAX - airDistance]++;
          units_land_player_total[actualDestination][0]++;
          units_land_grand_total[actualDestination]++;
          units_land_player_total[starting_land][0]--;
          units_land_grand_total[starting_land]--;
        } else {
          uint8_t seaDestination = actualDestination - LANDS_COUNT;
          units_sea_ptr[seaDestination][BOMBERS_SEA]
                       [BOMBER_MOVES_MAX - 1 - airDistance]++;
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
void move_tanks() {
  for (int starting_land = 0; starting_land < LANDS_COUNT; starting_land++) {
    for (int moves_remaining = TANK_MOVES_MAX; moves_remaining >= 0;
         moves_remaining--) {
      while (units_land_ptr[starting_land][TANKS][moves_remaining] > 0) {
        if (current_player.is_human) {
          setPrintableStatus();
          strcat(printableGameStatus, "Moving Tank From: ");
          strcat(printableGameStatus, LANDS[starting_land].name);
          strcat(printableGameStatus, " To: ");
          printf("%s\n", printableGameStatus);
          getUserInput();
        } else {
          // AI
          getAIInput();
        }
        // what is the actual destination that is a max of 2 land moves away?
        uint8_t actualDestination =
            landMove2Destination[starting_land][user_input];

        if (starting_land == actualDestination ||
            actualDestination == INFINITY) {
          units_land_ptr[starting_land][TANKS][0]++;
          units_land_ptr[starting_land][TANKS][moves_remaining]--;
          continue;
        }
        // what is the actual land distance between the two?
        uint8_t landDistance =
            total_land_distance[starting_land][actualDestination];
        uint8_t nextLandMovement;
        if (landDistance == 2) {
          // check if intermediate land blocks blitz, if it does, then move 1
          // space closer
          uint8_t nextLandMovement =
              landMove1Destination[starting_land][user_input];
          uint8_t nextLandMovementAlt = nextLandMovement;
          // check if the next sea movement has EnemyBlockers
          bool hasEnemyBlockers = false;
          for (int enemy_index = 0; enemy_index < enemies_count;
               enemy_index++) {
            if (units_land_player_total[nextLandMovement]
                                       [enemies[enemy_index]] > 0 ||
                gameData.land_state[nextLandMovement].factory_max > 0) {
              hasEnemyBlockers = true;
              nextLandMovement =
                  landMove1DestinationAlt[starting_land][actualDestination];
              break;
            }
          }
          if (hasEnemyBlockers && nextLandMovementAlt != nextLandMovement) {
            // check if the next sea movement has EnemyBlockers
            hasEnemyBlockers = false;
            for (int k = 0; k < enemies_count; k++) {
              if (units_land_player_total[nextLandMovement][enemies[k]] > 0 ||
                  gameData.land_state[nextLandMovement].factory_max > 0) {
                hasEnemyBlockers = true;
                break;
              }
            }
          }
          // if both paths are blocked, or dest is land
          // then move 1 space closer (where enemies are)
          if (hasEnemyBlockers) {
            actualDestination = nextLandMovement;
          }
        }
        if (actualDestination >= LANDS_COUNT) {
          uint8_t seaDestination = actualDestination - LANDS_COUNT;
          // try to board transport
          bool loaded_successfully = false;
          int trans_type = TRANS_1I;
          while (!loaded_successfully && trans_type >= TRANS_EMPTY) {
            uint8_t trans_state =
                STATES_MOVE_SEA[trans_type] - STATES_STAGING[trans_type];
            while (!loaded_successfully &&
                   trans_state >= STATES_UNLOADING[trans_type]) {
              if (units_sea_ptr[seaDestination][trans_type][trans_state] > 0) {
                units_sea_ptr[seaDestination][LOAD_TANK[trans_type]]
                             [trans_state]++;
                units_sea_player_total[seaDestination][0]++;
                units_sea_grand_total[seaDestination]++;
                units_sea_ptr[seaDestination][trans_type][trans_state]--;
                units_land_player_total[starting_land][0]--;
                units_land_grand_total[starting_land]--;
                units_land_ptr[starting_land][TANKS][trans_type]--;
                loaded_successfully = true;
              }
              trans_state--;
            }
            trans_type--;
          }
          if (loaded_successfully) {
            continue;
          }
          // boarding was unsuccessful, move to land
          if (landDistance == 2) {
            actualDestination = nextLandMovement;
          } else {
            // failed move distance of 1, so stay put
            units_land_ptr[starting_land][TANKS][0]++;
            units_land_ptr[starting_land][TANKS][trans_type]--;
            continue;
          }
        }
        units_land_ptr[actualDestination][TANKS]
                      [moves_remaining - landDistance]++;
        units_land_player_total[actualDestination][0]++;
        units_land_grand_total[actualDestination]++;
        units_land_player_total[starting_land][0]--;
        units_land_grand_total[starting_land]--;
        units_land_ptr[starting_land][TANKS][moves_remaining]--;
      }
    }
  }
}
void move_artillery() {
  for (int i = 0; i < LANDS_COUNT; i++) {
    while (units_land_ptr[i][ARTILLERY][ARTILLERY_MOVES_MAX] > 0) {
      if (current_player.is_human) {
        setPrintableStatus();
        strcat(printableGameStatus, "Moving Artillery From: ");
        strcat(printableGameStatus, LANDS[i].name);
        strcat(printableGameStatus, " To: ");
        printf("%s\n", printableGameStatus);
        getUserInput();
      } else {
        // AI
        getAIInput();
      }
      // what is the actual destination that is a max of 1 land move away?
      uint8_t actualDestination = landMove1Destination[i][user_input];

      if (i == actualDestination || actualDestination == INFINITY) {
        units_land_ptr[i][ARTILLERY][0]++;
        units_land_ptr[i][ARTILLERY][ARTILLERY_MOVES_MAX]--;
        continue;
      }
      if (actualDestination >= LANDS_COUNT) {
        uint8_t seaDestination = actualDestination - LANDS_COUNT;
        // try to board transport
        bool loaded_successfully = false;
        int j = TRANS_1I;
        while (!loaded_successfully && j >= TRANS_EMPTY) {
          uint8_t k = STATES_MOVE_SEA[j] - STATES_STAGING[j];
          while (!loaded_successfully && k >= STATES_UNLOADING[j]) {
            if (units_sea_ptr[seaDestination][j][k] > 0) {
              units_sea_ptr[seaDestination][LOAD_ARTILLERY[j]][k]++;
              units_sea_player_total[seaDestination][0]++;
              units_sea_grand_total[seaDestination]++;
              units_sea_ptr[seaDestination][j][k]--;
              units_land_player_total[i][0]--;
              units_land_grand_total[i]--;
              units_land_ptr[i][ARTILLERY][ARTILLERY_MOVES_MAX]--;
              loaded_successfully = true;
            }
            k--;
          }
          j--;
        }
        if (loaded_successfully) {
          continue;
        }
        // failed move distance of 1, so stay put
        units_land_ptr[i][ARTILLERY][0]++;
        units_land_ptr[i][ARTILLERY][ARTILLERY_MOVES_MAX]--;
        continue;
      }
      units_land_ptr[actualDestination][ARTILLERY][ARTILLERY_MOVES_MAX - 1]++;
      units_land_player_total[actualDestination][0]++;
      units_land_grand_total[actualDestination]++;
      units_land_player_total[i][0]--;
      units_land_grand_total[i]--;
      units_land_ptr[i][ARTILLERY][ARTILLERY_MOVES_MAX]--;
    }
  }
}
void move_infantry() {
  for (int i = 0; i < LANDS_COUNT; i++) {
    while (units_land_ptr[i][INFANTRY][INFANTRY_MOVES_MAX] > 0) {
      if (current_player.is_human) {
        setPrintableStatus();
        strcat(printableGameStatus, "Moving Infantry From: ");
        strcat(printableGameStatus, LANDS[i].name);
        strcat(printableGameStatus, " To: ");
        printf("%s\n", printableGameStatus);
        getUserInput();
      } else {
        // AI
        getAIInput();
      }
      // what is the actual destination that is a max of 1 land move away?
      uint8_t actualDestination = landMove1Destination[i][user_input];

      if (i == actualDestination || actualDestination == INFINITY) {
        units_land_ptr[i][INFANTRY][0]++;
        units_land_ptr[i][INFANTRY][INFANTRY_MOVES_MAX]--;
        continue;
      }
      if (actualDestination >= LANDS_COUNT) {
        uint8_t seaDestination = actualDestination - LANDS_COUNT;
        // try to board transport
        bool loaded_successfully = false;
        int j = TRANS_1T;
        while (!loaded_successfully && j >= TRANS_EMPTY) {
          uint8_t k = STATES_MOVE_SEA[j] - STATES_STAGING[j];
          while (!loaded_successfully && k >= STATES_UNLOADING[j]) {
            if (units_sea_ptr[seaDestination][j][k] > 0) {
              units_sea_ptr[seaDestination][LOAD_INFANTRY[j]][k]++;
              units_sea_player_total[seaDestination][0]++;
              units_sea_grand_total[seaDestination]++;
              units_sea_ptr[seaDestination][j][k]--;
              units_land_player_total[i][0]--;
              units_land_grand_total[i]--;
              units_land_ptr[i][INFANTRY][INFANTRY_MOVES_MAX]--;
              loaded_successfully = true;
            }
            k--;
          }
          j--;
        }
        if (loaded_successfully) {
          continue;
        }
        // failed move distance of 1, so stay put
        units_land_ptr[i][INFANTRY][0]++;
        units_land_ptr[i][INFANTRY][INFANTRY_MOVES_MAX]--;
        continue;
      }
      units_land_ptr[actualDestination][INFANTRY][INFANTRY_MOVES_MAX - 1]++;
      units_land_player_total[actualDestination][0]++;
      units_land_grand_total[actualDestination]++;
      units_land_player_total[i][0]--;
      units_land_grand_total[i]--;
      units_land_ptr[i][INFANTRY][INFANTRY_MOVES_MAX]--;
    }
  }
}
void move_transport_units() {
  for (int i = TRANS_EMPTY; i <= TRANS_1I_1T; i++) {
    int max_index = STATES_MOVE_SEA[i] - STATES_STAGING[i];
    int done_moving = STATES_UNLOADING[i];
    int min_index = STATES_UNLOADING[i] + 1;
    // TODO CHECKPOINT
    for (int j = 0; j < SEAS_COUNT; j++) {
      for (int k = max_index; k >= min_index; k--) {
        int moves_remaining = k - STATES_UNLOADING[i];
        while (units_sea_ptr[j][i][k] > 0) {
          if (current_player.is_human) {
            setPrintableStatus();
            strcat(printableGameStatus, "Moving ");
            strcat(printableGameStatus, NAMES_UNIT_SEA[i]);
            strcat(printableGameStatus, " From: ");
            strcat(printableGameStatus, SEAS[j].name);
            strcat(printableGameStatus, " To: ");
            printf("%s\n", printableGameStatus);
            getUserInput();
          } else {
            // AI
            getAIInput();
          }
          uint8_t actualDestination;
          if (moves_remaining == 2) {
            // what is the actual destination that is a max of 2 sea moves
            // away?
            actualDestination = seaMove2Destination[canal_state][j][user_input];
          } else {
            // what is the actual destination that is a max of 1 sea moves
            // away?
            actualDestination = seaMove1Destination[canal_state][j][user_input];
          }
          if (j == actualDestination || actualDestination == INFINITY) {
            units_sea_ptr[actualDestination][i][done_moving]++;
            units_sea_ptr[j][i][k]--;
            continue;
          }
          // what is the actual sea distance between the two?
          uint8_t seaDistance =
              total_sea_distance[canal_state][j][actualDestination];
          // if the distance is 2, is the primary path blocked?
          if (seaDistance == 2) {
            uint8_t nextSeaMovement =
                seaMove1Destination[canal_state][j][actualDestination];
            uint8_t nextSeaMovementAlt = nextSeaMovement;
            // check if the next sea movement has enemy ships
            bool hasEnemyShips = false;
            for (int l = 0; l < enemies_count; l++) {
              if (units_sea_blockade_total[nextSeaMovement][enemies[l]] > 0) {
                hasEnemyShips = true;
                nextSeaMovement =
                    seaMove1DestinationAlt[canal_state][j][actualDestination];
                break;
              }
            }
            if (hasEnemyShips && nextSeaMovementAlt != nextSeaMovement) {
              // check if the next sea movement has enemy ships
              hasEnemyShips = false;
              for (int l = 0; l < enemies_count; l++) {
                if (units_sea_blockade_total[nextSeaMovement][enemies[l]] > 0) {
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
            units_sea_ptr[j][i][done_moving]++;
            units_sea_ptr[j][i][k]--;
            continue;
          }
          units_sea_ptr[actualDestination][i][done_moving]++;
          // units_sea_type_total[actualDestination][i]++;
          units_sea_player_total[actualDestination][0]++;
          units_sea_grand_total[actualDestination]++;
          units_sea_ptr[actualDestination][i][k]--;
          // units_sea_type_total[j][i]--;
          units_sea_player_total[j][0]--;
          units_sea_grand_total[j]--;
        }
      }
    }
  }
}
void move_subs() {
  for (int j = 0; j < SEAS_COUNT; j++) {
    while (units_sea_ptr[j][SUBMARINES][SUB_UNMOVED] > 0) {
      if (current_player.is_human) {
        setPrintableStatus();
        strcat(printableGameStatus, "Moving ");
        strcat(printableGameStatus, NAMES_UNIT_SEA[i]);
        strcat(printableGameStatus, " From: ");
        strcat(printableGameStatus, SEAS[j].name);
        strcat(printableGameStatus, " To: ");
        printf("%s\n", printableGameStatus);
        getUserInput();
      } else {
        // AI
        getAIInput();
      }
      uint8_t actualDestination;
      // what is the actual destination that is a max of 2 sea moves away?
      actualDestination = seaMove2Destination[canal_state][j][user_input];

      if (j == actualDestination || actualDestination == INFINITY) {
        units_sea_ptr[actualDestination][SUBMARINES][SUB_DONE_MOVING]++;
        units_sea_ptr[j][SUBMARINES][SUB_UNMOVED]--;
        continue;
      }
      // what is the actual sea distance between the two?
      uint8_t seaDistance =
          total_sea_distance[canal_state][j][actualDestination];
      // if the distance is 2, is the primary path blocked?
      if (seaDistance == 2) {
        uint8_t nextSeaMovement =
            seaMove1Destination[canal_state][j][actualDestination];
        uint8_t nextSeaMovementAlt = nextSeaMovement;
        // check if the next sea movement has enemy ships
        bool hasEnemyShips = false;
        for (int l = 0; l < enemies_count; l++) {
          if (gameData.units_sea[nextSeaMovement]
                  .other_units[enemies[l]][DESTROYERS] > 0) {
            hasEnemyShips = true;
            nextSeaMovement =
                seaMove1DestinationAlt[canal_state][j][actualDestination];
            break;
          }
        }
        if (hasEnemyShips && nextSeaMovementAlt != nextSeaMovement) {
          // check if the next sea movement has enemy ships
          hasEnemyShips = false;
          for (int l = 0; l < enemies_count; l++) {
            if (gameData.units_sea[nextSeaMovement]
                    .other_units[enemies[l]][DESTROYERS] > 0) {
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
        units_sea_ptr[j][SUBMARINES][SUB_DONE_MOVING]++;
        units_sea_ptr[j][SUBMARINES][SUB_UNMOVED]--;
        continue;
      }
      units_sea_ptr[actualDestination][SUBMARINES][SUB_DONE_MOVING]++;
      // units_sea_type_total[actualDestination][i]++;
      units_sea_player_total[actualDestination][0]++;
      units_sea_grand_total[actualDestination]++;
      units_sea_ptr[actualDestination][SUBMARINES][SUB_UNMOVED]--;
      // units_sea_type_total[j][i]--;
      units_sea_player_total[j][0]--;
      units_sea_grand_total[j]--;
    }
  }
}

void move_destroyers_battleships() {

  for (int i = DESTROYERS; i <= BS_DAMAGED; i++) {
    for (int j = 0; j < SEAS_COUNT; j++) {
      while (units_sea_ptr[j][i][SUB_UNMOVED] > 0) {
        if (current_player.is_human) {
          setPrintableStatus();
          strcat(printableGameStatus, "Moving ");
          strcat(printableGameStatus, NAMES_UNIT_SEA[i]);
          strcat(printableGameStatus, " From: ");
          strcat(printableGameStatus, SEAS[j].name);
          strcat(printableGameStatus, " To: ");
          printf("%s\n", printableGameStatus);
          getUserInput();
        } else {
          // AI
          getAIInput();
        }
        uint8_t actualDestination;
        // what is the actual destination that is a max of 2 sea moves away?
        actualDestination = seaMove2Destination[canal_state][j][user_input];

        if (j == actualDestination || actualDestination == INFINITY) {
          units_sea_ptr[actualDestination][i][SUB_DONE_MOVING]++;
          units_sea_ptr[j][i][SUB_UNMOVED]--;
          continue;
        }
        // what is the actual sea distance between the two?
        uint8_t seaDistance =
            total_sea_distance[canal_state][j][actualDestination];
        // if the distance is 2, is the primary path blocked?
        if (seaDistance == 2) {
          uint8_t nextSeaMovement =
              seaMove1Destination[canal_state][j][actualDestination];
          uint8_t nextSeaMovementAlt = nextSeaMovement;
          // check if the next sea movement has enemy ships
          bool hasEnemyShips = false;
          for (int l = 0; l < enemies_count; l++) {
            if (units_sea_blockade_total[nextSeaMovement][enemies[l]] > 0) {
              hasEnemyShips = true;
              nextSeaMovement =
                  seaMove1DestinationAlt[canal_state][j][actualDestination];
              break;
            }
          }
          if (hasEnemyShips && nextSeaMovementAlt != nextSeaMovement) {
            // check if the next sea movement has enemy ships
            hasEnemyShips = false;
            for (int l = 0; l < enemies_count; l++) {
              if (units_sea_blockade_total[nextSeaMovement][enemies[l]] > 0) {
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
          units_sea_ptr[j][i][SUB_DONE_MOVING]++;
          units_sea_ptr[j][i][SUB_UNMOVED]--;
          continue;
        }
        units_sea_ptr[actualDestination][i][SUB_DONE_MOVING]++;
        // units_sea_type_total[actualDestination][i]++;
        units_sea_player_total[actualDestination][0]++;
        units_sea_grand_total[actualDestination]++;
        units_sea_ptr[actualDestination][i][SUB_UNMOVED]--;
        // units_sea_type_total[j][i]--;
        units_sea_player_total[j][0]--;
        units_sea_grand_total[j]--;
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