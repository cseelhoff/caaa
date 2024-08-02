#include "game_data.h"
#include "cJSON.h"
#include "config.h"
#include "land.h"
#include "player.h"
#include "sea_data.h"
#include "serialize_data.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* phases[2] = {"Combat", "Landing, Purchase"};
char* LAND_UNIT_NAMES[LAND_UNIT_TYPES] = {"Fighters ", "Bombers ", "Infantry",
                                          "Artillery", "Tanks   ", "AA Guns "};
uint8_t move_max_land[LAND_UNIT_TYPES] = {4, 6, 1, 1, 2, 1};
uint8_t move_states_land[LAND_UNIT_TYPES] = {5, 7, 2, 2, 3, 2};

char* SEA_UNIT_NAMES[SEA_UNIT_TYPES] = {
    "Fighters   ", "Trans Empty",  "Trans 1 Inf",  "Trans 1 Art", "Trans 1 Tnk",
    "Trans 2 Inf", "Trans 1I 1A ", "Trans 1I 1T",  "Submarines ", "Destroyers ",
    "Carriers   ", "Battleships",  "BS Damaged  ", "Bombers   "};
uint8_t move_max_sea[SEA_UNIT_TYPES] = {4, 2, 2, 2, 2, 2, 2,
                                        2, 2, 2, 2, 2, 2, 6};
uint8_t move_states_sea[SEA_UNIT_TYPES] = {5, 4, 4, 4, 4, 3, 3,
                                           3, 3, 3, 3, 3, 3, 6};

char buffer[STRING_BUFFER_SIZE];
char threeCharStr[5];
GameData gameData = {0};
cJSON* json;
GameCache cache = {0};
uint8_t player_index;
Player player;
char* playerName;
char printableGameStatus[5000] = "";
uint8_t enemies[PLAYERS_COUNT - 1];
uint8_t enemies_count;
UnitsSeaMobileTotal units_sea_mobile_total;
UnitsSeaMobile units_sea_mobile;
UnitsSeaStatic units_sea_static;
UnitsSea units_sea;
LandState land_state;
UnitsLandStatic land_static;
UnitsLandMobile land_mobile;
int user_input;
Land land;

void initializeGameData() {
  // json = serialize_game_data_to_json(&gameData);
  // write_json_to_file("game_data_0.json", json);
  // cJSON_Delete(json);

  json = read_json_from_file("game_data.json");
  deserialize_game_data_from_json(&gameData, json);
  cJSON_Delete(json);

  // clear printableGameStatus
  gameData.phase = 0;
  player_index = gameData.player_index;
  player = Players[player_index];
  playerName = player.name;
  printableGameStatus[0] = '\0';
  buildCache();
  stage_transport_units();
  move_land_units();
  move_transport_units();
  move_sea_units();
  move_fighter_units();
  move_bomber_units();
  resolve_sea_battles();
  unload_transports();
  bomb_factories();
  bombard_shores();
  fire_aa_guns();
  resolve_land_battles();
  land_air_units();
  move_aa_guns();
  reset_units_fully();
  buy_units();
  crash_air_units();
  collect_money();
}

void buildCache() {
  for (int i = 0; i < PLAYERS_COUNT; i++) {
    // cache.income_per_turn[i] = 0;
    // cache.enemies_count = 0;
    uint8_t modPlayerIndex = (player_index + i) % PLAYERS_COUNT;
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
    LandState land_state = gameData.land_state[i];
    cache.income_per_turn[land_state.owner_index] += Lands[i].land_value;
    cache.units_land_ptr[i][FIGHTERS] = (uint8_t*)land_state.fighters;
    cache.units_land_ptr[i][BOMBERS_LAND] = (uint8_t*)land_state.bombers;
    cache.units_land_ptr[i][INFANTRY] = (uint8_t*)land_state.infantry;
    cache.units_land_ptr[i][ARTILLERY] = (uint8_t*)land_state.artillery;
    cache.units_land_ptr[i][TANKS] = (uint8_t*)land_state.tanks;
    cache.units_land_ptr[i][AA_GUNS] = (uint8_t*)land_state.aa_guns;
    cache.units_land_grand_total[i] = cache.units_land_player_total[i][0];
    for (int j = 0; j < LAND_UNIT_TYPES; j++) {
      for (int k = 0; k < move_states_land[j]; k++) {
        cache.units_land_type_total[i][j] += cache.units_land_ptr[i][j][k];
      }
      cache.units_land_player_total[i][0] += cache.units_land_type_total[i][j];
    }
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
    UnitsSea units_sea = gameData.units_sea[i];
    cache.units_sea_ptr[i][TRANS_EMPTY] = (uint8_t*)units_sea.trans_empty;
    cache.units_sea_ptr[i][TRANS_1I] = (uint8_t*)units_sea.trans_1i;
    cache.units_sea_ptr[i][TRANS_1A] = (uint8_t*)units_sea.trans_1a;
    cache.units_sea_ptr[i][TRANS_1T] = (uint8_t*)units_sea.trans_1t;
    cache.units_sea_ptr[i][TRANS_2I] = (uint8_t*)units_sea.trans_2i;
    cache.units_sea_ptr[i][TRANS_1I_1A] = (uint8_t*)units_sea.trans_1i_1a;
    cache.units_sea_ptr[i][TRANS_1I_1T] = (uint8_t*)units_sea.trans_1i_1t;
    cache.units_sea_ptr[i][SUBMARINES] = (uint8_t*)units_sea.submarines;
    cache.units_sea_ptr[i][DESTROYERS] = (uint8_t*)units_sea.destroyers;
    cache.units_sea_ptr[i][CARRIERS] = (uint8_t*)units_sea.carriers;
    cache.units_sea_ptr[i][BATTLESHIPS] = (uint8_t*)units_sea.battleships;
    cache.units_sea_ptr[i][BS_DAMAGED] = (uint8_t*)units_sea.bs_damaged;
    cache.units_sea_ptr[i][BOMBERS_SEA] = (uint8_t*)units_sea.bombers;
    for (int j = 0; j < SEA_UNIT_TYPES; j++) {
      cache.units_sea_ptr[i][j] = 0;
      for (int k = 0; k < move_states_sea[j]; k++) {
        cache.units_sea_type_total[i][j] += cache.units_sea_ptr[i][j][k];
      }
      cache.units_sea_player_total[i][0] += cache.units_sea_type_total[i][j];
    }
    cache.units_sea_grand_total[i] = cache.units_sea_player_total[i][0];
    for (int j = 0; j < PLAYERS_COUNT - 1; j++) {
      for (int k = 0; k < SEA_UNIT_TYPES; k++) {
        cache.units_sea_player_total[i][j] +=
            gameData.units_sea[i].other_units[j][k];
      }
      cache.units_sea_grand_total[i] += cache.units_sea_player_total[i][j];
    }
  }
}

void setPrintableStatus() {
  strcat(printableGameStatus, player.color);
  strcat(printableGameStatus, playerName);
  strcat(printableGameStatus, "\033[0m");
  strcat(printableGameStatus, " is in phase ");
  strcat(printableGameStatus, phases[gameData.phase]);
  strcat(printableGameStatus, " with money ");
  sprintf(threeCharStr, "%d", gameData.money[player_index]);
  strcat(printableGameStatus, threeCharStr);
  strcat(printableGameStatus, "\n");
  setPrintableStatusLands(gameData, cache, printableGameStatus);
  setPrintableStatusSeas(gameData, cache, printableGameStatus);
}

void setPrintableStatusLands() {
  for (int i = 0; i < LANDS_COUNT; i++) {
    LandState land_state = gameData.land_state[i];
    strcat(printableGameStatus, Players[land_state.owner_index].color);
    sprintf(threeCharStr, "%d ", i);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, Lands[i].name);
    strcat(printableGameStatus, ": ");
    strcat(printableGameStatus, Players[land_state.owner_index].name);
    strcat(printableGameStatus, " ");
    sprintf(threeCharStr, "%d", land_state.builds_left);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, "/");
    sprintf(threeCharStr, "%d", land_state.factory_hp);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, "/");
    sprintf(threeCharStr, "%d", land_state.factory_max);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, " Conquered: ");
    if (land_state.conquered) {
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
        strcat(printableGameStatus, LAND_UNIT_NAMES[j]);
        for (int k = 0; k < move_states_land[j]; k++) {
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
            strcat(printableGameStatus, LAND_UNIT_NAMES[k]);
            for (int l = 0; l < move_states_land[k]; l++) {
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
    units_sea_mobile = units_sea.units_sea_mobile;
    sprintf(threeCharStr, "%d ", LANDS_COUNT + i);
    strcat(printableGameStatus, threeCharStr);
    strcat(printableGameStatus, Seas[i].name);
    strcat(printableGameStatus, "             |Tot| 0| 1| 2| 3| 4| 5| 6|\n");
    strcat(printableGameStatus, player.color);
    if (cache.units_sea_player_total[i][0] > 0) {
      units_sea_mobile_total = cache.units_sea_type_total[i];
      if (units_sea_mobile_total.transports_empty > 0) {
        snprintf(buffer, STRING_BUFFER_SIZE,
                 "%s Transports Empty: |%3d%3d%3d%3d%3d\n", player.name,
                 units_sea_mobile_total.transports_empty,
                 units_sea_mobile.transports_empty_0,
                 units_sea_mobile.transports_empty_1,
                 units_sea_mobile.transports_empty_2,
                 units_sea_mobile.transports_empty_s);
        strcat(printableGameStatus, buffer);
      }
      if (units_sea_mobile_total.transports_1i > 0) {
        snprintf(
            buffer, STRING_BUFFER_SIZE,
            "%s Transports 1i:   |%3d%3d%3d%3d%3d\n", player.name,
            units_sea_mobile_total.transports_1i,
            units_sea_mobile.transports_1i_0, units_sea_mobile.transports_1i_1,
            units_sea_mobile.transports_1i_2, units_sea_mobile.transports_1i_s);
        strcat(printableGameStatus, buffer);
      }
      if (units_sea_mobile_total.transports_1a > 0) {
        snprintf(
            buffer, STRING_BUFFER_SIZE,
            "%s Transports 1a:   |%3d%3d%3d%3d%3d\n", player.name,
            units_sea_mobile_total.transports_1a,
            units_sea_mobile.transports_1a_0, units_sea_mobile.transports_1a_1,
            units_sea_mobile.transports_1a_2, units_sea_mobile.transports_1a_s);
        strcat(printableGameStatus, buffer);
      }
      if (units_sea_mobile_total.transports_1t > 0) {
        snprintf(
            buffer, STRING_BUFFER_SIZE,
            "%s Transports 1t:   |%3d%3d%3d%3d%3d\n", player.name,
            units_sea_mobile_total.transports_1t,
            units_sea_mobile.transports_1t_0, units_sea_mobile.transports_1t_1,
            units_sea_mobile.transports_1t_2, units_sea_mobile.transports_1t_s);
        strcat(printableGameStatus, buffer);
      }
      if (units_sea_mobile_total.transports_2i > 0) {
        snprintf(
            buffer, STRING_BUFFER_SIZE, "%s Transports 2i:   |%3d%3d%3d%3d\n",
            player.name, units_sea_mobile_total.transports_2i,
            units_sea_mobile.transports_2i_0, units_sea_mobile.transports_2i_1,
            units_sea_mobile.transports_2i_2);
        strcat(printableGameStatus, buffer);
      }
      if (units_sea_mobile_total.transports_1i_1a > 0) {
        snprintf(buffer, STRING_BUFFER_SIZE,
                 "%s Transports 1i 1a:|%3d%3d%3d%3d\n", player.name,
                 units_sea_mobile_total.transports_1i_1a,
                 units_sea_mobile.transports_1i_1a_0,
                 units_sea_mobile.transports_1i_1a_1,
                 units_sea_mobile.transports_1i_1a_2);
        strcat(printableGameStatus, buffer);
      }
      if (units_sea_mobile_total.transports_1i_1t > 0) {
        snprintf(buffer, STRING_BUFFER_SIZE,
                 "%s Transports 1i 1t:|%3d%3d%3d%3d\n", player.name,
                 units_sea_mobile_total.transports_1i_1t,
                 units_sea_mobile.transports_1i_1t_0,
                 units_sea_mobile.transports_1i_1t_1,
                 units_sea_mobile.transports_1i_1t_2);
        strcat(printableGameStatus, buffer);
      }
      if (units_sea_mobile_total.destroyers > 0) {
        snprintf(buffer, STRING_BUFFER_SIZE,
                 "%s Destroyers:      |%3d%3d%3d%3d\n", player.name,
                 units_sea_mobile_total.destroyers,
                 units_sea_mobile.destroyers_0, units_sea_mobile.destroyers_1,
                 units_sea_mobile.destroyers_2);
        strcat(printableGameStatus, buffer);
      }
      if (units_sea_mobile_total.carriers > 0) {
        snprintf(buffer, STRING_BUFFER_SIZE,
                 "%s Carriers:        |%3d%3d%3d%3d\n", player.name,
                 units_sea_mobile_total.carriers, units_sea_mobile.carriers_0,
                 units_sea_mobile.carriers_1, units_sea_mobile.carriers_2);
        strcat(printableGameStatus, buffer);
      }
      if (units_sea_mobile_total.battleships > 0) {
        snprintf(buffer, STRING_BUFFER_SIZE,
                 "%s Battleships:     |%3d%3d%3d%3d\n", player.name,
                 units_sea_mobile_total.battleships,
                 units_sea_mobile.battleships_0, units_sea_mobile.battleships_1,
                 units_sea_mobile.battleships_2);
        strcat(printableGameStatus, buffer);
      }
      if (units_sea_mobile_total.battleships_damaged > 0) {
        snprintf(buffer, STRING_BUFFER_SIZE,
                 "%s BS Damaged:      |%3d%3d%3d%3d\n", player.name,
                 units_sea_mobile_total.battleships_damaged,
                 units_sea_mobile.battleships_damaged_0,
                 units_sea_mobile.battleships_damaged_1,
                 units_sea_mobile.battleships_damaged_2);
        strcat(printableGameStatus, buffer);
      }
      if (units_sea_mobile_total.submarines > 0) {
        snprintf(buffer, STRING_BUFFER_SIZE,
                 "%s Submarines:      |%3d%3d%3d%3d\n", player.name,
                 units_sea_mobile_total.submarines,
                 units_sea_mobile.submarines_0, units_sea_mobile.submarines_1,
                 units_sea_mobile.submarines_2);
        strcat(printableGameStatus, buffer);
      }
      if (units_sea_mobile_total.fighters > 0) {
        snprintf(buffer, STRING_BUFFER_SIZE,
                 "%s Fighters:        |%3d%3d%3d%3d%3d%3d\n", player.name,
                 units_sea_mobile_total.fighters, units_sea_mobile.fighters_0,
                 units_sea_mobile.fighters_1, units_sea_mobile.fighters_2,
                 units_sea_mobile.fighters_3, units_sea_mobile.fighters_4);
        strcat(printableGameStatus, buffer);
      }
      if (units_sea_mobile_total.bombers > 0) {
        snprintf(buffer, STRING_BUFFER_SIZE,
                 "%s Bombers:        |%3d%3d%3d%3d%3d%3d%3d%3d\n", player.name,
                 units_sea_mobile_total.bombers, 0, units_sea_mobile.bombers_1,
                 units_sea_mobile.bombers_2, units_sea_mobile.bombers_3,
                 units_sea_mobile.bombers_4, units_sea_mobile.bombers_5, 0);
        strcat(printableGameStatus, buffer);
      }
      strcat(printableGameStatus, "\033[0m");
      for (int j = 0; j < PLAYERS_COUNT - 1; j++) {
        player = Players[(player_index + j) % PLAYERS_COUNT];
        units_sea_static = units_sea.units_sea_static[j];
        strcat(printableGameStatus, player.color);
        if (units_sea_static.transports_empty > 0) {
          snprintf(buffer, STRING_BUFFER_SIZE, "%s Transports Empty: |%3d\n",
                   player.name, units_sea_static.transports_empty);
          strcat(printableGameStatus, buffer);
        }
        if (units_sea_static.transports_1i > 0) {
          snprintf(buffer, STRING_BUFFER_SIZE, "%s Transports 1i:   |%3d\n",
                   player.name, units_sea_static.transports_1i);
          strcat(printableGameStatus, buffer);
        }
        if (units_sea_static.transports_1a > 0) {
          snprintf(buffer, STRING_BUFFER_SIZE, "%s Transports 1a:   |%3d\n",
                   player.name, units_sea_static.transports_1a);
          strcat(printableGameStatus, buffer);
        }
        if (units_sea_static.transports_1t > 0) {
          snprintf(buffer, STRING_BUFFER_SIZE, "%s Transports 1t:   |%3d\n",
                   player.name, units_sea_static.transports_1t);
          strcat(printableGameStatus, buffer);
        }
        if (units_sea_static.transports_2i > 0) {
          snprintf(buffer, STRING_BUFFER_SIZE, "%s Transports 2i:   |%3d\n",
                   player.name, units_sea_static.transports_2i);
          strcat(printableGameStatus, buffer);
        }
        if (units_sea_static.transports_1i_1a > 0) {
          snprintf(buffer, STRING_BUFFER_SIZE, "%s Transports 1i 1a:|%3d\n",
                   player.name, units_sea_static.transports_1i_1a);
          strcat(printableGameStatus, buffer);
        }
        if (units_sea_static.transports_1i_1t > 0) {
          snprintf(buffer, STRING_BUFFER_SIZE, "%s Transports 1i 1t:|%3d\n",
                   player.name, units_sea_static.transports_1i_1t);
          strcat(printableGameStatus, buffer);
        }
        if (units_sea_static.destroyers > 0) {
          snprintf(buffer, STRING_BUFFER_SIZE, "%s Destroyers:      |%3d\n",
                   player.name, units_sea_static.destroyers);
          strcat(printableGameStatus, buffer);
        }
        if (units_sea_static.carriers > 0) {
          snprintf(buffer, STRING_BUFFER_SIZE, "%s Carriers:        |%3d\n",
                   player.name, units_sea_static.carriers);
          strcat(printableGameStatus, buffer);
        }
        if (units_sea_static.battleships > 0) {
          snprintf(buffer, STRING_BUFFER_SIZE, "%s Battleships:     |%3d\n",
                   player.name, units_sea_static.battleships);
          strcat(printableGameStatus, buffer);
        }
        if (units_sea_static.battleships_damaged > 0) {
          snprintf(buffer, STRING_BUFFER_SIZE, "%s BS Damaged:      |%3d\n",
                   player.name, units_sea_static.battleships_damaged);
          strcat(printableGameStatus, buffer);
        }
        if (units_sea_static.submarines > 0) {
          snprintf(buffer, STRING_BUFFER_SIZE, "%s Submarines:      |%3d\n",
                   player.name, units_sea_static.submarines);
          strcat(printableGameStatus, buffer);
        }
        if (units_sea_static.fighters > 0) {
          snprintf(buffer, STRING_BUFFER_SIZE, "%s Fighters:        |%3d\n",
                   player.name, units_sea_static.fighters);
          strcat(printableGameStatus, buffer);
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

void moveAllTransports_S(uint8_t from_sea, uint8_t* transports_s,
                         uint8_t* transports_2, uint8_t* transports_1,
                         uint8_t* transports_0, char* transport_name) {
  uint8_t actualDestination;
  uint8_t nextSeaMovement;
  uint8_t nextSeaMovement2;
  uint8_t seaDistance;
  while (transports_s > 0) {
    if (player.is_human) {
      setPrintableStatus();
      strcat(printableGameStatus, "Staging Transport ");
      strcat(printableGameStatus, transport_name);
      strcat(printableGameStatus, " From: ");
      strcat(printableGameStatus, Seas[from_sea].name);
      strcat(printableGameStatus, " To: ");
      printf("%s\n", printableGameStatus);
      getUserInput();
    } else {
      // AI
      getAIInput();
    }
    // what is the actual destination that is a max of 2 sea moves away?
    actualDestination = seaMove2Destination[from_sea][user_input];
    if (from_sea == actualDestination) {
      transports_2++;
      transports_s--;
      continue;
    }
    // what is the actual sea distance between the two?
    seaDistance = seaDistance[i, actualDestination];
    // if the distance is 2, is the primary path blocked?
    if (seaDistance == 2) {
      nextSeaMovement = seaMove1Destination[from_sea][actualDestination];
      // check if the next sea movement has enemy ships
      bool hasEnemyShips = false;
      for (int k = 0; k < cache.enemies_count; k++) {
        if (cache.units_sea_blockade_total[nextSeaMovement][cache.enemies[k]] >
            0) {
          hasEnemyShips = true;
          nextSeaMovement2 =
              seaMove1DestinationAlt[nextSeaMovement][actualDestination];
          break;
        }
      }
      if (hasEnemyShips && nextSeaMovement2 != nextSeaMovement) {
        // check if the next sea movement has enemy ships
        hasEnemyShips = false;
        for (int k = 0; k < cache.enemies_count; k++) {
          if (cache.units_sea_blockade_total[nextSeaMovement2]
                                            [cache.enemies[k]] > 0) {
            hasEnemyShips = true;
            break;
          }
        }
      }
      if (hasEnemyShips) {
        actualDestination = nextSeaMovement;
      }
      gameData.units_sea[actualDestination]
          .units_sea_mobile.transports_empty_0++;
      cache.units_sea_type_total[actualDestination].transports_empty++;
      cache.units_sea_player_total[actualDestination][0]++;
      cache.units_sea_grand_total[actualDestination]++;
      gameData.units_sea[from_sea].units_sea_mobile.transports_empty_s--;
      cache.units_sea_type_total[from_sea].transports_empty--;
      cache.units_sea_player_total[from_sea][0]--;
      cache.units_sea_grand_total[from_sea]--;
      continue;
    }
    // move 1 destination
    gameData.units_sea[actualDestination].units_sea_mobile.transports_empty_1++;
    cache.units_sea_type_total[actualDestination].transports_empty++;
    cache.units_sea_player_total[actualDestination][0]++;
    cache.units_sea_grand_total[actualDestination]++;
    gameData.units_sea[from_sea].units_sea_mobile.transports_empty_s--;
    cache.units_sea_type_total[from_sea].transports_empty--;
    cache.units_sea_player_total[from_sea][0]--;
    cache.units_sea_grand_total[from_sea]--;
  }
}

void stage_transport_units() {
  // loop through transports with "3" moves remaining (that aren't full), start
  // at sea 0 to n
  // TODO: optimize with cache
  for (int i = 0; i < SEAS_COUNT; i++) {
    if (cache.units_sea_grand_total[i] == 0) {
      continue;
    }
    // ask for destination of all land and sea zones 0 to n
    moveAllTransports_S(
        i, &gameData.units_sea[i].units_sea_mobile.transports_empty_s,
        &gameData.units_sea[i].units_sea_mobile.transports_empty_2,
        &gameData.units_sea[i].units_sea_mobile.transports_empty_1,
        &gameData.units_sea[i].units_sea_mobile.transports_empty_0, "Empty");
  }

  // move 1
  // get actual destination based on pathing and movement remaining
  // get distance between current and destination, subract movement
  // accordingly conduct actual movement by decrementing one unit counter and
  // incrementing another update caches accordingly
}

void move_land_units() {
  // foreach unit stack that the player owns
  // loop through land units
  //  1. no movement - set moves to 0
  //  2. move 1 space, reduce movement, and ask again
  //    a. if boarding, set movement remaining to receiving transports
  //    movement remaining b. if not boarding ask:
  //      1. 1 unit
  //      2. 50% stack (rounded down)
  //      3. 100% stack (rounded down)
  //  3. (if adj to water and transports with moves still exist) wait
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
void move_fighter_units() {
  // loop through all fighter units (no kamakaze check yet)
  // 1. (if available) no more movement
  // 2. move 1 space, reduce movement, and ask again
  // crash fighter if unsavable
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