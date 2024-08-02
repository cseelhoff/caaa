#include "game_data.h"
#include "cJSON.h"
#include "config.h"
#include "land.h"
#include "player.h"
#include "sea_data.h"
#include "serialize_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* phases[2] = {"Combat", "Landing, Purchase"};
char buffer[STRING_BUFFER_SIZE];
char threeCharStr[4];
GameData gameData = {0};
cJSON* json;
GameCache cache = {0};
uint8_t player_index;
Player player;
char* playerName;
char printableGameStatus[5000] = "";
UnitsSeaMobileTotal units_sea_mobile_total;
UnitsSeaMobile units_sea_mobile;
UnitsSeaStatic units_sea_static;
UnitsSea units_sea;
LandState land_state;
UnitsLandStatic land_static;
UnitsLandMobile land_mobile;

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
  buildCache(&gameData, &cache);
  if (Players[0].is_human) {
    setPrintableStatus(&gameData, &cache, printableGameStatus);
    printf("%s\n", printableGameStatus);
  }
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
  for (int i = 0; i < LANDS_COUNT; i++) {
    cache.income_per_turn[Lands[i].original_owner_index] += Lands[i].land_value;
    land_static = cache.units_land_static[i];
    land_mobile = gameData.land_state[i].units_land.units_land_mobile;
    land_static.infantry = land_mobile.infantry_0 + land_mobile.infantry_1;
    land_static.artillery = land_mobile.artillery_0 + land_mobile.artillery_1;
    land_static.tanks =
        land_mobile.tanks_0 + land_mobile.tanks_1 + land_mobile.tanks_2;
    land_static.aa_guns = land_mobile.aa_guns_0 + land_mobile.aa_guns_1;
    land_static.fighters = land_mobile.fighters_0 + land_mobile.fighters_1 +
                           land_mobile.fighters_2 + land_mobile.fighters_3 +
                           land_mobile.fighters_4;
    land_static.bombers = land_mobile.bombers_0 + land_mobile.bombers_1 +
                          land_mobile.bombers_2 + land_mobile.bombers_3 +
                          land_mobile.bombers_4 + land_mobile.bombers_5 +
                          land_mobile.bombers_6;
    cache.units_land_total[i][0] = cache.units_land_static[i].infantry +
                                   cache.units_land_static[i].artillery +
                                   cache.units_land_static[i].tanks +
                                   cache.units_land_static[i].aa_guns +
                                   cache.units_land_static[i].fighters +
                                   cache.units_land_static[i].bombers;
    for (int j = 0; j < PLAYERS_COUNT - 1; j++) {
      cache.units_land_total[i][j + 1] =
          gameData.land_state[i].units_land.units_land_static[j].infantry +
          gameData.land_state[i].units_land.units_land_static[j].artillery +
          gameData.land_state[i].units_land.units_land_static[j].tanks +
          gameData.land_state[i].units_land.units_land_static[j].aa_guns +
          gameData.land_state[i].units_land.units_land_static[j].fighters +
          gameData.land_state[i].units_land.units_land_static[j].bombers;
    }
    cache.units_land_grand_total[i] =
        cache.units_land_total[i][0] + cache.units_land_total[i][1] +
        cache.units_land_total[i][2] + cache.units_land_total[i][3] +
        cache.units_land_total[i][4];
  }
  for (int i = 0; i < SEAS_COUNT; i++) {
    units_sea_mobile_total = cache.units_sea_mobile_total[i];
    units_sea_mobile = gameData.units_sea[i].units_sea_mobile;
    units_sea_mobile_total.transports_empty =
        units_sea_mobile.transports_empty_0 +
        units_sea_mobile.transports_empty_1 +
        units_sea_mobile.transports_empty_2 +
        units_sea_mobile.transports_empty_s;
    units_sea_mobile_total.transports_1i = units_sea_mobile.transports_1i_0 +
                                           units_sea_mobile.transports_1i_1 +
                                           units_sea_mobile.transports_1i_2+
                                           units_sea_mobile.transports_1i_s;
    units_sea_mobile_total.transports_1a = units_sea_mobile.transports_1a_0 +
                                           units_sea_mobile.transports_1a_1 +
                                           units_sea_mobile.transports_1a_2 +
                                           units_sea_mobile.transports_1a_s;
    units_sea_mobile_total.transports_1t = units_sea_mobile.transports_1t_0 +
                                           units_sea_mobile.transports_1t_1 +
                                           units_sea_mobile.transports_1t_2 +
                                           units_sea_mobile.transports_1t_s;
    units_sea_mobile_total.transports_2i = units_sea_mobile.transports_2i_0 +
                                           units_sea_mobile.transports_2i_1 +
                                           units_sea_mobile.transports_2i_2;
    units_sea_mobile_total.transports_1i_1a =
        units_sea_mobile.transports_1i_1a_0 +
        units_sea_mobile.transports_1i_1a_1 +
        units_sea_mobile.transports_1i_1a_2;
    units_sea_mobile_total.transports_1i_1t =
        units_sea_mobile.transports_1i_1t_0 +
        units_sea_mobile.transports_1i_1t_1 +
        units_sea_mobile.transports_1i_1t_2;
    units_sea_mobile_total.destroyers = units_sea_mobile.destroyers_0 +
                                        units_sea_mobile.destroyers_1 +
                                        units_sea_mobile.destroyers_2;
    units_sea_mobile_total.carriers = units_sea_mobile.carriers_0 +
                                      units_sea_mobile.carriers_1 +
                                      units_sea_mobile.carriers_2;
    units_sea_mobile_total.battleships = units_sea_mobile.battleships_0 +
                                         units_sea_mobile.battleships_1 +
                                         units_sea_mobile.battleships_2;
    units_sea_mobile_total.battleships_damaged =
        units_sea_mobile.battleships_damaged_0 +
        units_sea_mobile.battleships_damaged_1 +
        units_sea_mobile.battleships_damaged_2;
    units_sea_mobile_total.submarines = units_sea_mobile.submarines_0 +
                                        units_sea_mobile.submarines_1 +
                                        units_sea_mobile.submarines_2;
    units_sea_mobile_total.fighters =
        units_sea_mobile.fighters_0 + units_sea_mobile.fighters_1 +
        units_sea_mobile.fighters_2 + units_sea_mobile.fighters_3 +
        units_sea_mobile.fighters_4;
    units_sea_mobile_total.bombers =
        units_sea_mobile.bombers_1 + units_sea_mobile.bombers_2 +
        units_sea_mobile.bombers_3 + units_sea_mobile.bombers_4 +
        units_sea_mobile.bombers_5;
    cache.units_sea_total[i][0] =
        units_sea_mobile_total.transports_empty +
        units_sea_mobile_total.transports_1i +
        units_sea_mobile_total.transports_1a +
        units_sea_mobile_total.transports_1t +
        units_sea_mobile_total.transports_2i +
        units_sea_mobile_total.transports_1i_1a +
        units_sea_mobile_total.transports_1i_1t +
        units_sea_mobile_total.destroyers + units_sea_mobile_total.carriers +
        units_sea_mobile_total.battleships +
        units_sea_mobile_total.battleships_damaged +
        units_sea_mobile_total.submarines + units_sea_mobile_total.fighters +
        units_sea_mobile_total.bombers;

    for (int j = 0; j < PLAYERS_COUNT - 1; j++) {
      cache.units_sea_total[i][j + 1] =
          gameData.units_sea[i].units_sea_static[j].transports_empty +
          gameData.units_sea[i].units_sea_static[j].transports_1i +
          gameData.units_sea[i].units_sea_static[j].transports_1a +
          gameData.units_sea[i].units_sea_static[j].transports_1t +
          gameData.units_sea[i].units_sea_static[j].transports_2i +
          gameData.units_sea[i].units_sea_static[j].transports_1i_1a +
          gameData.units_sea[i].units_sea_static[j].transports_1i_1t +
          gameData.units_sea[i].units_sea_static[j].destroyers +
          gameData.units_sea[i].units_sea_static[j].carriers +
          gameData.units_sea[i].units_sea_static[j].battleships +
          gameData.units_sea[i].units_sea_static[j].battleships_damaged +
          gameData.units_sea[i].units_sea_static[j].submarines +
          gameData.units_sea[i].units_sea_static[j].fighters;
    }
    cache.units_sea_grand_total[i] =
        cache.units_sea_total[i][0] + cache.units_sea_total[i][1] +
        cache.units_sea_total[i][2] + cache.units_sea_total[i][3] +
        cache.units_sea_total[i][4];
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
  setPrintableStatusLands(gameData, cache, printableGameStatus);
  setPrintableStatusSeas(gameData, cache, printableGameStatus);
}

void setPrintableStatusLands() {
  for (int i = 0; i < LANDS_COUNT; i++) {
    land_state = gameData.land_state[i];
    land_mobile = land_state.units_land.units_land_mobile;
    strcat(printableGameStatus, "\n");
    strcat(printableGameStatus, Players[land_state.owner_index].color);
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
    if (cache.units_land_static[i].infantry > 0) {
      snprintf(buffer, STRING_BUFFER_SIZE, "%s Infantry: |%3d%3d%3d\n",
               player.name, cache.units_land_static[i].infantry,
               land_mobile.infantry_0, land_mobile.infantry_1);
      strcat(printableGameStatus, buffer);
    }
    if (cache.units_land_static[i].artillery > 0) {
      snprintf(buffer, STRING_BUFFER_SIZE, "%s Artillery:|%3d%3d%3d\n",
               player.name, cache.units_land_static[i].artillery,
               land_mobile.artillery_0, land_mobile.artillery_1);
      strcat(printableGameStatus, buffer);
    }
    if (cache.units_land_static[i].tanks > 0) {
      snprintf(buffer, STRING_BUFFER_SIZE, "%s Tanks:    |%3d%3d%3d%3d\n",
               player.name, cache.units_land_static[i].tanks,
               land_mobile.tanks_0, land_mobile.tanks_1, land_mobile.tanks_2);
      strcat(printableGameStatus, buffer);
    }
    if (cache.units_land_static[i].aa_guns > 0) {
      snprintf(buffer, STRING_BUFFER_SIZE, "%s AA Guns:  |%3d%3d%3d\n",
               player.name, cache.units_land_static[i].aa_guns,
               land_mobile.aa_guns_0, land_mobile.aa_guns_1);
      strcat(printableGameStatus, buffer);
    }
    if (cache.units_land_static[i].fighters > 0) {
      snprintf(buffer, STRING_BUFFER_SIZE, "%s Fighters: |%3d%3d%3d%3d%3d%3d\n",
               player.name, cache.units_land_static[i].fighters,
               land_mobile.fighters_0, land_mobile.fighters_1,
               land_mobile.fighters_2, land_mobile.fighters_3,
               land_mobile.fighters_4);
      strcat(printableGameStatus, buffer);
    }
    if (cache.units_land_static[i].bombers > 0) {
      snprintf(buffer, STRING_BUFFER_SIZE,
               "%s Bombers:  |%3d%3d%3d%3d%3d%3d%3d%3d\n", player.name,
               cache.units_land_static[i].bombers, land_mobile.bombers_0,
               land_mobile.bombers_1, land_mobile.bombers_2,
               land_mobile.bombers_3, land_mobile.bombers_4,
               land_mobile.bombers_5, land_mobile.bombers_6);
      strcat(printableGameStatus, buffer);
    }
    strcat(printableGameStatus, "\033[0m");
    for (int j = 0; j < PLAYERS_COUNT - 1; j++) {
      player = Players[(player_index + j) % PLAYERS_COUNT];
      land_static = gameData.land_state[i].units_land.units_land_static[j];
      strcat(printableGameStatus, player.color);
      if (land_static.infantry > 0) {
        strcat(printableGameStatus, player.name);
        strcat(printableGameStatus, " Infantry: |");
        sprintf(threeCharStr, "%3d", land_static.infantry);
        strcat(printableGameStatus, threeCharStr);
        strcat(printableGameStatus, "\n");
      }
      if (land_static.artillery > 0) {
        strcat(printableGameStatus, player.name);
        strcat(printableGameStatus, " Artillery:|");
        sprintf(threeCharStr, "%3d", land_static.artillery);
        strcat(printableGameStatus, threeCharStr);
        strcat(printableGameStatus, "\n");
      }
      if (land_static.tanks > 0) {
        strcat(printableGameStatus, player.name);
        strcat(printableGameStatus, " Tanks:    |");
        sprintf(threeCharStr, "%3d", land_static.tanks);
        strcat(printableGameStatus, threeCharStr);
        strcat(printableGameStatus, "\n");
      }
      if (land_static.aa_guns > 0) {
        strcat(printableGameStatus, player.name);
        strcat(printableGameStatus, " AA Guns:  |");
        sprintf(threeCharStr, "%3d", land_static.aa_guns);
        strcat(printableGameStatus, threeCharStr);
        strcat(printableGameStatus, "\n");
      }
      if (land_static.fighters > 0) {
        strcat(printableGameStatus, player.name);
        strcat(printableGameStatus, " Fighters: |");
        sprintf(threeCharStr, "%3d", land_static.fighters);
        strcat(printableGameStatus, threeCharStr);
        strcat(printableGameStatus, "\n");
      }
      if (land_static.bombers > 0) {
        strcat(printableGameStatus, player.name);
        strcat(printableGameStatus, " Bombers:  |");
        sprintf(threeCharStr, "%3d", land_static.bombers);
        strcat(printableGameStatus, threeCharStr);
        strcat(printableGameStatus, "\n");
      }
      strcat(printableGameStatus, "\033[0m");
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
    strcat(printableGameStatus, "\n");
    strcat(printableGameStatus, Seas[i].name);
    strcat(printableGameStatus, "             |Tot| 0| 1| 2| 3| 4| 5| 6|\n");
    strcat(printableGameStatus, player.color);
    if (cache.units_sea_total[i][0] > 0) {
      units_sea_mobile_total = cache.units_sea_mobile_total[i];
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
            buffer, STRING_BUFFER_SIZE, "%s Transports 1i:   |%3d%3d%3d%3d%3d\n",
            player.name, units_sea_mobile_total.transports_1i,
            units_sea_mobile.transports_1i_0, units_sea_mobile.transports_1i_1,
            units_sea_mobile.transports_1i_2,
            units_sea_mobile.transports_1i_s);
        strcat(printableGameStatus, buffer);
      }
      if (units_sea_mobile_total.transports_1a > 0) {
        snprintf(
            buffer, STRING_BUFFER_SIZE, "%s Transports 1a:   |%3d%3d%3d%3d%3d\n",
            player.name, units_sea_mobile_total.transports_1a,
            units_sea_mobile.transports_1a_0, units_sea_mobile.transports_1a_1,
            units_sea_mobile.transports_1a_2,
            units_sea_mobile.transports_1a_s);
        strcat(printableGameStatus, buffer);
      }
      if (units_sea_mobile_total.transports_1t > 0) {
        snprintf(
            buffer, STRING_BUFFER_SIZE, "%s Transports 1t:   |%3d%3d%3d%3d%3d\n",
            player.name, units_sea_mobile_total.transports_1t,
            units_sea_mobile.transports_1t_0, units_sea_mobile.transports_1t_1,
            units_sea_mobile.transports_1t_2,
            units_sea_mobile.transports_1t_s);
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

void stage_transport_units() {
  //loop through transports with 2 moves remaining that aren't full, start at sea 0 to n
  //ask for destination start at sea 0 to n
  //
  //get actual destination based on pathing and movement remaining
  //conduct movement -
}

void move_land_units() {
      //foreach unit stack that the player owns
    //loop through land units
    // 1. no movement - set moves to 0
    // 2. move 1 space, reduce movement, and ask again
    //   a. if boarding, set movement remaining to receiving transports movement remaining
    //   b. if not boarding ask:
    //     1. 1 unit
    //     2. 50% stack (rounded down)
    //     3. 100% stack (rounded down)
    // 3. (if adj to water and transports with moves still exist) wait
}
void move_transport_units() {
      //---repeat until all transports done---
    //loop through all transports
    // 1. no movement
    // 2. move 1 space
    //loop through land units
    // 1. load transport
    //   a. set movement remaining to receiving transports movement remaining
    // 2. (if adj to water and transports with moves still exist) wait
    //---
}
void move_sea_units() {
      //loop through all remaining sea units
    // 1. no movement - set moves to 0
    // 2. move 1 space, reduce movement, and ask again
}
void move_fighter_units() {
      //loop through all fighter units (no kamakaze check yet)
    // 1. (if available) no more movement
    // 2. move 1 space, reduce movement, and ask again
    // crash fighter if unsavable
}
void move_bomber_units() {
      //loop through all bomber units (no kamakaze check yet)
    // 1. (if available) no more movement - bomber mode
    // 2. (if available) no more movement - attack mode
    // 3. move 1 space, reduce movement, and ask again
    // crash bomber if unsavable
}
void resolve_sea_battles() {}
void unload_transports() {
      //loop through all transports
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
      //loop through all land units
    // 1. (if available) no more movement
    // 2. move 1 space, reduce movement, and ask again
    // 3. (if adj to water and transports with moves still exist) wait

    //old notes:
        //land combat
    // a. round with retreat option  
}
void land_air_units() {
      //loop through all air units
    // 1. (if available) no more movement
    // 2. move 1 space, reduce movement, and ask again
    // crash air unit if unsavable

    //old notes:
    //loop through all air units
    // 1. no movement - set moves to 0
    // 2. move 1 space, reduce movement, and ask again
    // crash air unit if unsavable
}
void move_aa_guns() {
  
    //loop through all aa units
    // 1. no movement - set moves to 0
    // 2. move 1 space, reduce movement, and ask again
}
void reset_units_fully() {}
void buy_units() {}
void crash_air_units() {}
void collect_money() {}