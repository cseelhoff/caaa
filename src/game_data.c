#include "game_data.h"
#include "land.h"
#include "player.h"
#include "sea.h"
#include "serialize_data.h"
#include "units/carrier.h"
#include "units/fighter.h"
#include "units/units.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char buffer[STRING_BUFFER_SIZE];
char threeCharStr[5];
GameData gameData = {0};
cJSON* json;
GameCache cache = {0};
uint8_t player_index;
uint8_t player_index_plus_one;
Player player;
char* playerName;
char printableGameStatus[5000] = "";
uint8_t enemies[PLAYERS_COUNT - 1];
uint8_t enemies_count;
bool is_allied[PLAYERS_COUNT];
UnitsSea units_sea;
LandState land_state;
int user_input;
Land land;
char* AIR_NAMES[AIRS_COUNT] = {0};
uint8_t AIR_CONNECTIONS_COUNT[AIRS_COUNT] = {0};
uint8_t AIR_CONNECTIONS[AIRS_COUNT][MAX_AIR_TO_AIR_CONNECTIONS] = {0};
uint8_t total_air_distance[AIRS_COUNT][AIRS_COUNT] = {0};
uint8_t total_land_distance[LANDS_COUNT][LANDS_COUNT] = {0};
uint8_t total_sea_distance[SEAS_COUNT][SEAS_COUNT] = {0};
uint8_t seaMove2Destination[SEAS_COUNT][SEAS_COUNT] = {255};
uint8_t seaMove1Destination[SEAS_COUNT][SEAS_COUNT] = {255};
uint8_t seaMove1DestinationAlt[SEAS_COUNT][SEAS_COUNT] = {255};
uint8_t airMove2Destination[AIRS_COUNT][AIRS_COUNT] = {255};
uint8_t airMove3Destination[AIRS_COUNT][AIRS_COUNT] = {255};
uint8_t airMove4Destination[AIRS_COUNT][AIRS_COUNT] = {255};
uint8_t allied_carriers[SEAS_COUNT] = {0};
bool canFighterLandHere[AIRS_COUNT];
bool canFighterLandIn1Move[AIRS_COUNT];

void initializeGameData() {
  // json = serialize_game_data_to_json(&gameData);
  // write_json_to_file("game_data_0.json", json);
  // cJSON_Delete(json);

  json = read_json_from_file("game_data.json");
  deserialize_game_data_from_json(&gameData, json);
  cJSON_Delete(json);

  // clear printableGameStatus
  player_index = gameData.player_index;
  player_index_plus_one = player_index + 1;
  player = Players[player_index];
  playerName = player.name;
  for (int i = 0; i < PLAYERS_COUNT; i++) {
    is_allied[i] = player.is_allied[i];
  }
  printableGameStatus[0] = '\0';
  build_total_air_distance();
  build_total_land_distance();
  build_total_sea_distance();
  build_seaMove2Destination();
  build_seaMove1Destination();
  build_seaMove1DestinationAlt();
  build_airMove2Destination();
  build_airMove3Destination();
  build_airMove4Destination();
  buildCache();
  stage_transport_units();
  move_fighter_units();
  move_bomber_units();
  move_land_units();
  move_transport_units();
  move_sea_units();
  resolve_sea_battles_ask_retreat();
  unload_transports();
  bombard_shores();
  //fire_strat_aa_guns();
  bomb_factories_safely_land_bombers();
  //fire_tactical_aa_guns();
  resolve_land_battles_ask_retreat();
  land_air_units();
  move_aa_guns();
  reset_units_fully();
  buy_units();
  crash_air_units();
  collect_money();
}

void build_airMove2Destination() {
  for (int i = 0; i < AIRS_COUNT; i++) {
    for (int j = 0; j < AIRS_COUNT; j++) {
      if (total_air_distance[i][j] <= 2) {
        airMove2Destination[i][j] = j;
        continue;
      }
      // If no direct path within two moves is found, return the closest air
      int min_distance = INFINITY;
      for (int k = 0; k < AIR_CONNECTIONS_COUNT[i]; k++) {
        int intermediate = AIR_CONNECTIONS[i][k];
        for (int l = 0; l < AIR_CONNECTIONS_COUNT[intermediate]; l++) {
          int intermediate2 = AIR_CONNECTIONS[intermediate][l];
          if (total_air_distance[intermediate2][j] < min_distance) {
            min_distance = total_air_distance[intermediate2][j];
            airMove2Destination[i][j] = intermediate2;
          }
        }
      }
    }
  }
}
void build_airMove3Destination() {
  for (int i = 0; i < AIRS_COUNT; i++) {
    for (int j = 0; j < AIRS_COUNT; j++) {
      if (total_air_distance[i][j] <= 3) {
        airMove3Destination[i][j] = j;
        continue;
      }
      // If no direct path within three moves is found, return the closest air
      int min_distance = INFINITY;
      for (int k = 0; k < AIR_CONNECTIONS_COUNT[i]; k++) {
        int intermediate = AIR_CONNECTIONS[i][k];
        for (int l = 0; l < AIR_CONNECTIONS_COUNT[intermediate]; l++) {
          int intermediate2 = AIR_CONNECTIONS[intermediate][l];
          for (int m = 0; m < AIR_CONNECTIONS_COUNT[intermediate2]; m++) {
            int intermediate3 = AIR_CONNECTIONS[intermediate2][m];
            if (total_air_distance[intermediate3][j] < min_distance) {
              min_distance = total_air_distance[intermediate3][j];
              airMove3Destination[i][j] = intermediate3;
            }
          }
        }
      }
    }
  }
}

void build_airMove4Destination() {
  for (int i = 0; i < AIRS_COUNT; i++) {
    for (int j = 0; j < AIRS_COUNT; j++) {
      if (total_air_distance[i][j] <= 4) {
        airMove4Destination[i][j] = j;
        continue;
      }
      // If no direct path within four moves is found, return the closest air
      int min_distance = INFINITY;
      for (int k = 0; k < AIR_CONNECTIONS_COUNT[i]; k++) {
        int intermediate = AIR_CONNECTIONS[i][k];
        for (int l = 0; l < AIR_CONNECTIONS_COUNT[intermediate]; l++) {
          int intermediate2 = AIR_CONNECTIONS[intermediate][l];
          for (int m = 0; m < AIR_CONNECTIONS_COUNT[intermediate2]; m++) {
            int intermediate3 = AIR_CONNECTIONS[intermediate2][m];
            for (int n = 0; n < AIR_CONNECTIONS_COUNT[intermediate3]; n++) {
              int intermediate4 = AIR_CONNECTIONS[intermediate3][n];
              if (total_air_distance[intermediate4][j] < min_distance) {
                min_distance = total_air_distance[intermediate4][j];
                airMove4Destination[i][j] = intermediate4;
              }
            }
          }
        }
      }
    }
  }
}

void build_seaMove2Destination() {
  for (int i = 0; i < SEAS_COUNT; i++) {
    for (int j = 0; j < SEAS_COUNT; j++) {
      if (total_sea_distance[i][j] <= 2) {
        seaMove2Destination[i][j] = j;
        continue;
      }

      // If no direct path within two moves is found, return the closest sea
      int min_distance = INFINITY;
      for (int k = 0; k < SEAS[i].sea_conn_count; k++) {
        int intermediate = SEAS[i].connected_sea_index[k];
        for (int l = 0; l < SEAS[intermediate].sea_conn_count; l++) {
          int intermediate2 = SEAS[intermediate].connected_sea_index[l];
          if (total_sea_distance[intermediate2][j] < min_distance) {
            min_distance = total_sea_distance[intermediate2][j];
            seaMove2Destination[i][j] = intermediate2;
          }
        }
      }
    }
  }
}

void build_seaMove1Destination() {
  for (int i = 0; i < SEAS_COUNT; i++) {
    for (int j = 0; j < SEAS_COUNT; j++) {
      if (total_sea_distance[i][j] <= 1) {
        seaMove1Destination[i][j] = j;
        continue;
      }

      // If no direct path within two moves is found, return the closest sea
      int min_distance = INFINITY;
      for (int k = 0; k < SEAS[i].sea_conn_count; k++) {
        int intermediate = SEAS[i].connected_sea_index[k];
        if (total_sea_distance[intermediate][j] < min_distance) {
          min_distance = total_sea_distance[intermediate][j];
          seaMove1Destination[i][j] = intermediate;
        }
      }
    }
  }
}

void build_seaMove1DestinationAlt() {
  for (int i = 0; i < SEAS_COUNT; i++) {
    for (int j = 0; j < SEAS_COUNT; j++) {
      if (total_sea_distance[i][j] <= 1) {
        seaMove1DestinationAlt[i][j] = j;
        continue;
      }

      // If no direct path within two moves is found, return the closest sea
      int min_distance = INFINITY;
      for (int k = SEAS[i].sea_conn_count - 1; k >= 0; k--) {
        int intermediate = SEAS[i].connected_sea_index[k];
        if (total_sea_distance[intermediate][j] < min_distance) {
          min_distance = total_sea_distance[intermediate][j];
          seaMove1DestinationAlt[i][j] = intermediate;
        }
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
  int land_index;
  int sea_index;
  // Populate initial distances based on connected_sea_index and
  // connected_land_index
  for (int i = 0; i < LANDS_COUNT; i++) {
    AIR_NAMES[i] = LANDS[i].name;
    for (int j = 0; j < LANDS[i].land_conn_count; j++) {
      land_index = LANDS[i].connected_land_index[j];
      AIR_CONNECTIONS[i][AIR_CONNECTIONS_COUNT[i]] = land_index;
      AIR_CONNECTIONS_COUNT[i]++;
      total_air_distance[i][land_index] = 1;
      total_air_distance[land_index][i] = 1;
    }
    for (int j = 0; j < LANDS[i].sea_conn_count; j++) {
      sea_index = LANDS[i].connected_sea_index[j] + LANDS_COUNT;
      AIR_CONNECTIONS[i][AIR_CONNECTIONS_COUNT[i]] = sea_index;
      AIR_CONNECTIONS_COUNT[i]++;
      total_air_distance[i][sea_index] = 1;
      total_air_distance[sea_index][i] = 1;
    }
  }

  for (int i = 0; i < SEAS_COUNT; i++) {
    AIR_NAMES[i + LANDS_COUNT] = SEAS[i].name;
    for (int j = 0; j < SEAS[i].land_conn_count; j++) {
      land_index = SEAS[i].connected_land_index[j];
      AIR_CONNECTIONS[i + LANDS_COUNT][AIR_CONNECTIONS_COUNT[i + LANDS_COUNT]] =
          land_index;
      AIR_CONNECTIONS_COUNT[i + LANDS_COUNT]++;
      total_air_distance[i + LANDS_COUNT][land_index] = 1;
      total_air_distance[land_index][i + LANDS_COUNT] = 1;
    }
    for (int j = 0; j < SEAS[i].sea_conn_count; j++) {
      sea_index = SEAS[i].connected_sea_index[j] + LANDS_COUNT;
      AIR_CONNECTIONS[i + LANDS_COUNT][AIR_CONNECTIONS_COUNT[i + LANDS_COUNT]] =
          sea_index;
      AIR_CONNECTIONS_COUNT[i + LANDS_COUNT]++;
      total_air_distance[i + LANDS_COUNT][sea_index] = 1;
      total_air_distance[sea_index][i + LANDS_COUNT] = 1;
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
    for (int j = 0; j < LANDS_COUNT; j++) {
      if (i != j) {
        total_land_distance[i][j] = INFINITY;
      } // else {
        // total_land_distance[i][j] = 0;
      //}
    }
  }

  // Populate initial distances based on connected_sea_index
  for (int i = 0; i < LANDS_COUNT; i++) {
    for (int j = 0; j < LANDS[i].land_conn_count; j++) {
      int land_index = LANDS[i].connected_land_index[j];
      total_land_distance[i][land_index] = 1;
      total_land_distance[land_index][i] = 1;
    }
  }

  // Floyd-Warshall algorithm to compute shortest paths
  for (int k = 0; k < LANDS_COUNT; k++) {
    for (int i = 0; i < LANDS_COUNT; i++) {
      for (int j = 0; j < LANDS_COUNT; j++) {
        if (total_land_distance[i][k] + total_land_distance[k][j] <
            total_land_distance[i][j]) {
          total_land_distance[i][j] =
              total_land_distance[i][k] + total_land_distance[k][j];
        }
      }
    }
  }
}

void build_total_sea_distance() {
  // Initialize the total_sea_distance array
  for (int i = 0; i < SEAS_COUNT; i++) {
    for (int j = 0; j < SEAS_COUNT; j++) {
      if (i != j) {
        total_sea_distance[i][j] = INFINITY;
      } // else {
        // total_sea_distance[i][j] = 0;
      //}
    }
  }

  // Populate initial distances based on connected_sea_index
  for (int i = 0; i < SEAS_COUNT; i++) {
    for (int j = 0; j < SEAS[i].sea_conn_count; j++) {
      int sea_index = SEAS[i].connected_sea_index[j];
      total_sea_distance[i][sea_index] = 1;
      total_sea_distance[sea_index][i] = 1;
    }
  }

  // Floyd-Warshall algorithm to compute shortest paths
  for (int k = 0; k < SEAS_COUNT; k++) {
    for (int i = 0; i < SEAS_COUNT; i++) {
      for (int j = 0; j < SEAS_COUNT; j++) {
        if (total_sea_distance[i][k] + total_sea_distance[k][j] <
            total_sea_distance[i][j]) {
          total_sea_distance[i][j] =
              total_sea_distance[i][k] + total_sea_distance[k][j];
        }
      }
    }
  }
}

void buildCache() {
  for (int i = 0; i < PLAYERS_COUNT; i++) {
    // cache.income_per_turn[i] = 0;
    // cache.enemies_count = 0;
    uint8_t modPlayerIndex = (player_index_plus_one + i) % PLAYERS_COUNT;
    cache.player_names[i] = Players[modPlayerIndex].name;
    cache.player_colors[i] = Players[modPlayerIndex].color;
    for (int j = 0; j < PLAYERS_COUNT - 1; j++) {
      if (player.is_allied[j] == false) {
        cache.enemies[cache.enemies_count] = j;
        cache.enemies_count++;
      }
    }
  }
  for (int i = 0; i < LANDS_COUNT; i++) {
    //    LandState land_state = gameData.land_state[i];
    cache.income_per_turn[gameData.land_state[i].owner_index] +=
        LANDS[i].land_value;
    cache.units_air_ptr[i][FIGHTERS] =
        (uint8_t*)gameData.land_state[i].fighters;
    cache.units_air_ptr[i][BOMBERS_LAND] =
        (uint8_t*)gameData.land_state[i].bombers;
    cache.units_land_ptr[i][FIGHTERS] =
        (uint8_t*)gameData.land_state[i].fighters;
    cache.units_land_ptr[i][BOMBERS_LAND] =
        (uint8_t*)gameData.land_state[i].bombers;
    cache.units_land_ptr[i][INFANTRY] =
        (uint8_t*)gameData.land_state[i].infantry;
    cache.units_land_ptr[i][ARTILLERY] =
        (uint8_t*)gameData.land_state[i].artillery;
    cache.units_land_ptr[i][TANKS] = (uint8_t*)gameData.land_state[i].tanks;
    cache.units_land_ptr[i][AA_GUNS] = (uint8_t*)gameData.land_state[i].aa_guns;
    cache.units_land_grand_total[i] = cache.units_land_player_total[i][0];
    // for (int j = 0; j < LAND_UNIT_TYPES; j++) {
    //   for (int k = 0; k < STATES_MOVE_LAND[j]; k++) {
    //     cache.units_land_type_total[i][j] += cache.units_land_ptr[i][j][k];
    //   }
    //   cache.units_land_player_total[i][0] +=
    //   cache.units_land_type_total[i][j];
    // }
    cache.units_land_grand_total[i] = cache.units_land_player_total[i][0];
    for (int j = 0; j < PLAYERS_COUNT - 1; j++) {
      for (int k = 0; k < LAND_UNIT_TYPES; k++) {
        cache.units_land_player_total[i][j] +=
            gameData.land_state[i].other_units[j][k];
      }
      cache.units_land_grand_total[i] += cache.units_land_player_total[i][j];
    }
  }
  for (int i = 0; i < SEAS_COUNT; i++) {
    cache.units_air_ptr[i + LANDS_COUNT][FIGHTERS] =
        (uint8_t*)gameData.units_sea[i].fighters;
    cache.units_air_ptr[i + LANDS_COUNT][BOMBERS_LAND] =
        (uint8_t*)gameData.units_sea[i].bombers;
    cache.units_sea_ptr[i][FIGHTERS] = (uint8_t*)gameData.units_sea[i].fighters;
    cache.units_sea_ptr[i][TRANS_EMPTY] =
        (uint8_t*)gameData.units_sea[i].trans_empty;
    cache.units_sea_ptr[i][TRANS_1I] = (uint8_t*)gameData.units_sea[i].trans_1i;
    cache.units_sea_ptr[i][TRANS_1A] = (uint8_t*)gameData.units_sea[i].trans_1a;
    cache.units_sea_ptr[i][TRANS_1T] = (uint8_t*)gameData.units_sea[i].trans_1t;
    cache.units_sea_ptr[i][TRANS_2I] = (uint8_t*)gameData.units_sea[i].trans_2i;
    cache.units_sea_ptr[i][TRANS_1I_1A] =
        (uint8_t*)gameData.units_sea[i].trans_1i_1a;
    cache.units_sea_ptr[i][TRANS_1I_1T] =
        (uint8_t*)gameData.units_sea[i].trans_1i_1t;
    cache.units_sea_ptr[i][SUBMARINES] =
        (uint8_t*)gameData.units_sea[i].submarines;
    cache.units_sea_ptr[i][DESTROYERS] =
        (uint8_t*)gameData.units_sea[i].destroyers;
    cache.units_sea_ptr[i][CRUISERS] = (uint8_t*)gameData.units_sea[i].cruisers;
    cache.units_sea_ptr[i][CARRIERS] = (uint8_t*)gameData.units_sea[i].carriers;
    allied_carriers[i] = gameData.units_sea[i].carriers[0] +
                         gameData.units_sea[i].carriers[1] +
                         gameData.units_sea[i].carriers[2];
    cache.units_sea_ptr[i][BATTLESHIPS] =
        (uint8_t*)gameData.units_sea[i].battleships;
    cache.units_sea_ptr[i][BS_DAMAGED] =
        (uint8_t*)gameData.units_sea[i].bs_damaged;
    cache.units_sea_ptr[i][BOMBERS_SEA] =
        (uint8_t*)gameData.units_sea[i].bombers;
    // for (int j = 0; j < SEA_UNIT_TYPES; j++) {
    //   cache.units_sea_type_total[i][j] = 0;
    //   for (int k = 0; k < STATES_MOVE_SEA[j]; k++) {
    //     cache.units_sea_type_total[i][j] += cache.units_sea_ptr[i][j][k];
    //   }
    //   cache.units_sea_player_total[i][0] += cache.units_sea_type_total[i][j];
    // }
    cache.units_sea_grand_total[i] = cache.units_sea_player_total[i][0];
    for (int j = 0; j < PLAYERS_COUNT - 1; j++) {
      for (int k = 0; k < SEA_UNIT_TYPES; k++) {
        cache.units_sea_player_total[i][j] +=
            gameData.units_sea[i].other_units[j][k];
      }
      allied_carriers[i] +=
          (gameData.units_sea[i].other_units[j][CARRIERS] *
           is_allied[(player_index_plus_one + j) % PLAYERS_COUNT]);
      cache.units_sea_grand_total[i] += cache.units_sea_player_total[i][j];
    }
  }
}

void setPrintableStatus() {
  strcat(printableGameStatus, player.color);
  strcat(printableGameStatus, playerName);
  strcat(printableGameStatus, "\033[0m");
  strcat(printableGameStatus, " is in phase ");
  //  strcat(printableGameStatus, PHASES[gameData.phase]);
  strcat(printableGameStatus, " with money ");
  sprintf(threeCharStr, "%d", gameData.money[player_index]);
  strcat(printableGameStatus, threeCharStr);
  strcat(printableGameStatus, "\n");
  setPrintableStatusLands(gameData, cache, printableGameStatus);
  setPrintableStatusSeas(gameData, cache, printableGameStatus);
}

void setPrintableStatusLands() {
  for (int i = 0; i < LANDS_COUNT; i++) {
    //    LandState land_state = gameData.land_state[i];
    strcat(printableGameStatus,
           Players[gameData.land_state[i].owner_index].color);
    sprintf(threeCharStr, "%d ", i);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, LANDS[i].name);
    strcat(printableGameStatus, ": ");
    strcat(printableGameStatus,
           Players[gameData.land_state[i].owner_index].name);
    strcat(printableGameStatus, " ");
    sprintf(threeCharStr, "%d", gameData.land_state[i].builds_left);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, "/");
    sprintf(threeCharStr, "%d", gameData.land_state[i].factory_hp);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, "/");
    sprintf(threeCharStr, "%d", gameData.land_state[i].factory_max);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, " Bombarded: ");
    if (gameData.land_state[i].bombard_max) {
      strcat(printableGameStatus, "true\n");
    } else {
      strcat(printableGameStatus, "false\n");
    }
    if (cache.units_land_grand_total[i] == 0) {
      strcat(printableGameStatus, "\033[0m");
      continue;
    }
    strcat(printableGameStatus, "             |Tot| 0| 1| 2| 3| 4| 5| 6|\n");
    for (int j = 0; j < LAND_UNIT_TYPES; j++) {
      if (cache.units_land_type_total[i][j] > 0) {
        strcat(printableGameStatus, player.name);
        strcat(printableGameStatus, NAMES_UNIT_LAND[j]);
        for (int k = 0; k < STATES_MOVE_LAND[j]; k++) {
          sprintf(threeCharStr, "%3d", cache.units_land_ptr[i][j][k]);
          strcat(printableGameStatus, threeCharStr);
        }
        strcat(printableGameStatus, "\n");
      }
    }
    strcat(printableGameStatus, "\033[0m");
    for (int j = 0; j < PLAYERS_COUNT - 1; j++) {
      if (cache.units_land_player_total[i][j] > 0) {
        strcat(printableGameStatus, cache.player_colors[j]);
        for (int k = 0; k < LAND_UNIT_TYPES; k++) {
          if (gameData.land_state[i].other_units[j][k] > 0) {
            strcat(printableGameStatus, cache.player_names[j]);
            strcat(printableGameStatus, NAMES_UNIT_LAND[k]);
            for (int l = 0; l < STATES_MOVE_LAND[k]; l++) {
              sprintf(threeCharStr, "%3d",
                      gameData.land_state[i].other_units[j][k]);
              strcat(printableGameStatus, threeCharStr);
            }
            strcat(printableGameStatus, "\n");
          }
        }
        strcat(printableGameStatus, "\033[0m");
      }
    }
  }
}

void setPrintableStatusSeas() {
  for (int i = 0; i < SEAS_COUNT; i++) {
    if (cache.units_sea_grand_total[i] == 0) {
      continue;
    }
    units_sea = gameData.units_sea[i];
    sprintf(threeCharStr, "%d ", LANDS_COUNT + i);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, SEAS[i].name);
    strcat(printableGameStatus, "             |Tot| 0| 1| 2| 3| 4| 5| 6|\n");
    strcat(printableGameStatus, player.color);
    if (cache.units_sea_player_total[i][0] > 0) {
      //      uint8_t* units_sea_mobile_total = cache.units_sea_type_total[i];
      for (int j = 0; j < SEA_UNIT_TYPES; j++) {
        bool units_present = false;
        for (int k = 0; k < STATES_MOVE_SEA[j]; k++) {
          if (cache.units_sea_ptr[i][j][k] > 0) {
            units_present = true;
            break;
          }
        }
        if (units_present) {
          strcat(printableGameStatus, player.name);
          strcat(printableGameStatus, NAMES_UNIT_SEA[j]);
          for (int k = 0; k < STATES_MOVE_SEA[j]; k++) {
            sprintf(threeCharStr, "%3d", cache.units_sea_ptr[i][j][k]);
            strcat(printableGameStatus, threeCharStr);
          }
          strcat(printableGameStatus, "\n");
        }
      }
      strcat(printableGameStatus, "\033[0m");
      for (int j = 0; j < PLAYERS_COUNT - 1; j++) {
        player = Players[(player_index_plus_one + j) % PLAYERS_COUNT];
        //        uint8_t* units_sea_static = units_sea.other_units[j];
        strcat(printableGameStatus, player.color);
        for (int k = 0; k < SEA_UNIT_TYPES; k++) {
          if (units_sea.other_units[j][k] > 0) {
            strcat(printableGameStatus, player.name);
            strcat(printableGameStatus, NAMES_UNIT_SEA[k]);
            sprintf(threeCharStr, "%3d", units_sea.other_units[j][k]);
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
  // loop through transports with "3" moves remaining (that aren't full), start
  // at sea 0 to n
  // TODO: optimize with cache - only loop through regions with transports
  uint8_t actualDestination;
  uint8_t nextSeaMovement;
  uint8_t nextSeaMovementAlt;
  uint8_t seaDistance;
  for (int i = TRANS_EMPTY; i <= TRANS_1T; i++) {
    // TODO CHECKPOINT
    for (int j = 0; j < SEAS_COUNT; j++) {
      while (cache.units_sea_ptr[j][i][TRANSPORT_MOVES_MAX + 1] > 0) {
        if (player.is_human) {
          setPrintableStatus();
          strcat(printableGameStatus, "Staging ");
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
        // what is the actual destination that is a max of 2 sea moves away?
        actualDestination = seaMove2Destination[j][user_input];
        if (j == actualDestination) {
          cache.units_sea_ptr[actualDestination][i][TRANSPORT_MOVES_MAX]++;
          cache.units_sea_ptr[j][i][TRANSPORT_MOVES_MAX + 1]--;
          continue;
        }
        // what is the actual sea distance between the two?
        seaDistance = total_sea_distance[j][actualDestination];
        // if the distance is 2, is the primary path blocked?
        if (seaDistance == 2) {
          nextSeaMovement = seaMove1Destination[j][actualDestination];
          // check if the next sea movement has enemy ships
          bool hasEnemyShips = false;
          for (int k = 0; k < cache.enemies_count; k++) {
            if (cache.units_sea_blockade_total[nextSeaMovement]
                                              [cache.enemies[k]] > 0) {
              hasEnemyShips = true;
              nextSeaMovementAlt =
                  seaMove1DestinationAlt[nextSeaMovement][actualDestination];
              break;
            }
          }
          if (hasEnemyShips && nextSeaMovementAlt != nextSeaMovement) {
            // check if the next sea movement has enemy ships
            hasEnemyShips = false;
            for (int k = 0; k < cache.enemies_count; k++) {
              if (cache.units_sea_blockade_total[nextSeaMovementAlt]
                                                [cache.enemies[k]] > 0) {
                hasEnemyShips = true;
                break;
              }
            }
          }
          // if both paths are blocked, move 1 space closer (where enemies are)
          if (hasEnemyShips) {
            actualDestination = nextSeaMovement;
          }
        }
        cache.units_sea_ptr[actualDestination][i]
                           [TRANSPORT_MOVES_MAX - seaDistance]++;
        // cache.units_sea_type_total[actualDestination][i]++;
        cache.units_sea_player_total[actualDestination][0]++;
        cache.units_sea_grand_total[actualDestination]++;
        cache.units_sea_ptr[actualDestination][i][TRANSPORT_MOVES_MAX + 1]--;
        // cache.units_sea_type_total[j][i]--;
        cache.units_sea_player_total[j][0]--;
        cache.units_sea_grand_total[j]--;
      }
    }
  }
}

void move_land_units() {}

void build_canFighterLandHere() {
  for (int i = 0; i < LANDS_COUNT; i++) {
    // is allied owned and not recently conquered?
    canFighterLandHere[i] = (is_allied[gameData.land_state[i].owner_index] &&
                             !gameData.land_state[i].bombard_max);
    // check for possiblity to build carrier under fighter
    if (gameData.land_state[i].factory_max > 0 &&
        gameData.land_state[i].owner_index == player_index) {
      for (int j = 0; j < LANDS[i].sea_conn_count; j++) {
        canFighterLandHere[LANDS_COUNT + LANDS[i].connected_sea_index[j]] =
            true;
      }
    }
  }
  for (int i = 0; i < SEAS_COUNT; i++) {
    if (allied_carriers[i] > 0) {
      canFighterLandHere[i] = true;
      // if player owns these carriers, then landing area is 2 spaces away
      if (cache.units_sea_ptr[i][CARRIERS][CARRIER_MOVES_MAX] > 0) {
        for (int j = 0; j < SEAS[i].sea_conn_count; j++) {
          uint8_t connected_sea1 = SEAS[i].connected_sea_index[j];
          canFighterLandHere[LANDS_COUNT + connected_sea1] = true;
          for (int k = 0; k < SEAS[connected_sea1].sea_conn_count; k++) {
            canFighterLandHere[LANDS_COUNT +
                               SEAS[connected_sea1].connected_sea_index[k]] =
                true;
          }
        }
      }
    }
  }
}

void build_canFighterLandIn1Move() {
  for (int i = 0; i < LANDS_COUNT; i++) {
    canFighterLandIn1Move[i] = false;
    for (int j = 0; j < LANDS[i].sea_conn_count; j++) {
      if (canFighterLandHere[LANDS[i].connected_land_index[j]]) {
        canFighterLandIn1Move[i] = true;
        break;
      }
    }
    for (int j = 0; j < LANDS[i].sea_conn_count; j++) {
      if (canFighterLandHere[LANDS_COUNT + LANDS[i].connected_sea_index[j]]) {
        canFighterLandIn1Move[i] = true;
        break;
      }
    }
  }
  for (int i = 0; i < SEAS_COUNT; i++) {
    canFighterLandIn1Move[LANDS_COUNT + i] = false;
    for (int j = 0; j < SEAS[i].sea_conn_count; j++) {
      if (canFighterLandHere[SEAS[i].connected_land_index[j]]) {
        canFighterLandIn1Move[LANDS_COUNT + i] = true;
        break;
      }
    }
    for (int j = 0; j < SEAS[i].sea_conn_count; j++) {
      if (canFighterLandHere[LANDS_COUNT + SEAS[i].connected_sea_index[j]]) {
        canFighterLandIn1Move[LANDS_COUNT + i] = true;
        break;
      }
    }
  }
}
void move_fighter_units() {
  build_canFighterLandHere();
  build_canFighterLandIn1Move();
  for (int i = 0; i < AIRS_COUNT; i++) {
    while (cache.units_air_ptr[i][FIGHTERS][FIGHTER_MOVES_MAX] > 0) {
      if (player.is_human) {
        setPrintableStatus();
        strcat(printableGameStatus, "Moving Fighter From: ");
        strcat(printableGameStatus, AIR_NAMES[i]);
        strcat(printableGameStatus, " To: ");
        printf("%s\n", printableGameStatus);
        getUserInput();
      } else {
        // AI
        getAIInput();
      }
      // what is the actual destination that is a max of 4 air moves away?
      uint8_t actualDestination = airMove4Destination[i][user_input];
      // check for bad move: if the actual destination is not water and is not
      // ally owned, and has no enemy units, then set actual destination to i
      if (actualDestination < LANDS_COUNT &&
          !is_allied[gameData.land_state[actualDestination].owner_index] &&
          cache.units_land_grand_total[actualDestination] == 0) {
        actualDestination = i;
      }
      if (i == actualDestination) {
        cache.units_air_ptr[i][FIGHTERS][0]++;
        cache.units_air_ptr[i][FIGHTERS][FIGHTER_MOVES_MAX]--;
        continue;
      } else {
        // what is the actual air distance between the two?
        uint8_t airDistance = total_air_distance[i][actualDestination];
        if (airDistance == 4) {
          if (!canFighterLandHere[actualDestination]) {
            actualDestination = airMove3Destination[i][user_input];
            airDistance = 3;
          }
        }
        if (airDistance == 3) {
          if (!canFighterLandIn1Move[actualDestination]) {
            actualDestination = airMove2Destination[i][user_input];
            airDistance = 2;
          }
        }
        cache.units_air_ptr[actualDestination][FIGHTERS]
                           [FIGHTER_MOVES_MAX - airDistance]++;
        if (actualDestination < LANDS_COUNT) {
          cache.units_land_player_total[actualDestination][FIGHTERS]++;
          cache.units_land_grand_total[actualDestination]++;
          cache.units_land_player_total[i][FIGHTERS]--;
          cache.units_land_grand_total[i]--;
        } else {
          cache.units_sea_player_total[actualDestination][FIGHTERS]++;
          cache.units_sea_grand_total[actualDestination]++;
          cache.units_sea_player_total[i][FIGHTERS]--;
          cache.units_sea_grand_total[i]--;
        }
        cache.units_air_ptr[i][FIGHTERS][FIGHTER_MOVES_MAX]--;
      }
    }
  }
}

void move_transport_units() {
  //---repeat until all transports done---
  // loop through all transports
  //  1. no movement
  //  2. move 1 space
  // loop through land units
  //  1. load transport
  //    a. set movement remaining to receiving transports movement remaining
  //  2. (if adj to water and transports with moves still exist) wait
  //---
}
void move_sea_units() {
  // loop through all remaining sea units
  // 1. no movement - set moves to 0
  // 2. move 1 space, reduce movement, and ask again
}
void move_bomber_units() {
  // loop through all bomber units (no kamakaze check yet)
  // 1. (if available) no more movement - bomber mode
  // 2. (if available) no more movement - attack mode
  // 3. move 1 space, reduce movement, and ask again
  // crash bomber if unsavable
}
void resolve_sea_battles() {}
void unload_transports() {
  // loop through all transports
  // 1. (if available) no more movement
  // 2. unload all units
  // 3. (if adj to water and transports with moves still exist) wait

  // old notes:
  // 1. no unload
  // 2. unload
}
void bomb_factories() {}
void bombard_shores() {}
void fire_aa_guns() {}
void resolve_land_battles() {
  // loop through all land units
  // 1. (if available) no more movement
  // 2. move 1 space, reduce movement, and ask again
  // 3. (if adj to water and transports with moves still exist) wait

  // old notes:
  // land combat
  // a. round with retreat option
}
void land_air_units() {
  // loop through all air units
  // 1. (if available) no more movement
  // 2. move 1 space, reduce movement, and ask again
  // crash air unit if unsavable

  // old notes:
  // loop through all air units
  //  1. no movement - set moves to 0
  //  2. move 1 space, reduce movement, and ask again
  //  crash air unit if unsavable
}
void move_aa_guns() {

  // loop through all aa units
  //  1. no movement - set moves to 0
  //  2. move 1 space, reduce movement, and ask again
}
void reset_units_fully() {}
void buy_units() {}
void crash_air_units() {}
void collect_money() {}