#include "game_data.h"
#include "config.h"
#include "land.h"
#include "player.h"
#include <stdio.h>
#include <string.h>

char* phases[2] = {"Combat", "Landing, Purchase"};

void initializeGameData() {
  GameData gameData;
  GameCache cache;
  char gameStatus[5000] = "";
  gameData.phase = 0;
  setStaticTotals(&gameData, &cache);
  if (Players[0].is_human) {
    setGameStatus(&gameData, &cache, gameStatus);
    printf("%s\n", gameStatus);
  }
}

void setStaticTotals(GameData* gameData, GameCache* cache) {
  for (int i; i < LANDS_COUNT; i++) {
    UnitsLandStatic land_static = cache->units_land_static[i];
    UnitsLandMobile land_mobile =
        gameData->land_state[i].units_land.units_land_mobile;
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
    cache->units_land_total[i][0] = cache->units_land_static[i].infantry +
                                    cache->units_land_static[i].artillery +
                                    cache->units_land_static[i].tanks +
                                    cache->units_land_static[i].aa_guns +
                                    cache->units_land_static[i].fighters +
                                    cache->units_land_static[i].bombers;
    for (int j; j < PLAYERS_COUNT - 1; j++) {
      cache->units_land_total[i][j + 1] =
          gameData->land_state[i].units_land.units_land_static[j].infantry +
          gameData->land_state[i].units_land.units_land_static[j].artillery +
          gameData->land_state[i].units_land.units_land_static[j].tanks +
          gameData->land_state[i].units_land.units_land_static[j].aa_guns +
          gameData->land_state[i].units_land.units_land_static[j].fighters +
          gameData->land_state[i].units_land.units_land_static[j].bombers;
    }
    cache->units_land_grand_total[i] =
        cache->units_land_total[i][0] + cache->units_land_total[i][1] +
        cache->units_land_total[i][2] + cache->units_land_total[i][3] +
        cache->units_land_total[i][4];
  }
  for (int i; i < SEAS_COUNT; i++) {
    UnitsSeaMobileTotal units_sea_mobile_total =
        cache->units_sea_mobile_total[i];
    UnitsSeaMobile sea_mobile = gameData->units_sea[i].units_sea_mobile;
    units_sea_mobile_total.transports_empty = sea_mobile.transports_empty_0 +
                                              sea_mobile.transports_empty_1 +
                                              sea_mobile.transports_empty_2;
    units_sea_mobile_total.transports_1i = sea_mobile.transports_1i_0 +
                                           sea_mobile.transports_1i_1 +
                                           sea_mobile.transports_1i_2;
    units_sea_mobile_total.transports_1a = sea_mobile.transports_1a_0 +
                                           sea_mobile.transports_1a_1 +
                                           sea_mobile.transports_1a_2;
    units_sea_mobile_total.transports_1t = sea_mobile.transports_1t_0 +
                                           sea_mobile.transports_1t_1 +
                                           sea_mobile.transports_1t_2;
    units_sea_mobile_total.transports_2i = sea_mobile.transports_2i_0 +
                                           sea_mobile.transports_2i_1 +
                                           sea_mobile.transports_2i_2;
    units_sea_mobile_total.transports_1i_1a = sea_mobile.transports_1i_1a_0 +
                                              sea_mobile.transports_1i_1a_1 +
                                              sea_mobile.transports_1i_1a_2;
    units_sea_mobile_total.transports_1i_1t = sea_mobile.transports_1i_1t_0 +
                                              sea_mobile.transports_1i_1t_1 +
                                              sea_mobile.transports_1i_1t_2;
    units_sea_mobile_total.destroyers = sea_mobile.destroyers_0 +
                                        sea_mobile.destroyers_1 +
                                        sea_mobile.destroyers_2;
    units_sea_mobile_total.carriers =
        sea_mobile.carriers_0 + sea_mobile.carriers_1 + sea_mobile.carriers_2;
    units_sea_mobile_total.battleships = sea_mobile.battleships_0 +
                                         sea_mobile.battleships_1 +
                                         sea_mobile.battleships_2;
    units_sea_mobile_total.battleships_damaged =
        sea_mobile.battleships_damaged_0 + sea_mobile.battleships_damaged_1 +
        sea_mobile.battleships_damaged_2;
    units_sea_mobile_total.submarines = sea_mobile.submarines_0 +
                                        sea_mobile.submarines_1 +
                                        sea_mobile.submarines_2;
    units_sea_mobile_total.fighters =
        sea_mobile.fighters_0 + sea_mobile.fighters_1 + sea_mobile.fighters_2 +
        sea_mobile.fighters_3 + sea_mobile.fighters_4;
    units_sea_mobile_total.bombers =
        sea_mobile.bombers_1 + sea_mobile.bombers_2 + sea_mobile.bombers_3 +
        sea_mobile.bombers_4 + sea_mobile.bombers_5;
    cache->units_sea_total[i][0] =
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

    for (int j; j < PLAYERS_COUNT - 1; j++) {
      cache->units_sea_total[i][j + 1] =
          gameData->units_sea[i].units_sea_static[j].transports_empty +
          gameData->units_sea[i].units_sea_static[j].transports_1i +
          gameData->units_sea[i].units_sea_static[j].transports_1a +
          gameData->units_sea[i].units_sea_static[j].transports_1t +
          gameData->units_sea[i].units_sea_static[j].transports_2i +
          gameData->units_sea[i].units_sea_static[j].transports_1i_1a +
          gameData->units_sea[i].units_sea_static[j].transports_1i_1t +
          gameData->units_sea[i].units_sea_static[j].destroyers +
          gameData->units_sea[i].units_sea_static[j].carriers +
          gameData->units_sea[i].units_sea_static[j].battleships +
          gameData->units_sea[i].units_sea_static[j].battleships_damaged +
          gameData->units_sea[i].units_sea_static[j].submarines +
          gameData->units_sea[i].units_sea_static[j].fighters;
    }
    cache->units_sea_grand_total[i] =
        cache->units_sea_total[i][0] + cache->units_sea_total[i][1] +
        cache->units_sea_total[i][2] + cache->units_sea_total[i][3] +
        cache->units_sea_total[i][4];
  }
}

void setGameStatus(GameData* gameData, GameCache* cache, char* gameStatus) {
  uint8_t player_index = gameData->player_index;
  Player player = Players[player_index];
  char* playerName = player.name;
  char threeCharStr[4];
  strcat(gameStatus, player.color);
  strcat(gameStatus, playerName);
  strcat(gameStatus, "\033[0m");
  strcat(gameStatus, " is in phase ");
  strcat(gameStatus, phases[gameData->phase]);
  strcat(gameStatus, " with money ");
  sprintf(threeCharStr, "%d", gameData->money[player_index]);
  strcat(gameStatus, threeCharStr);
  // append gameData.land_state
  for (int i = 0; i < LANDS_COUNT; i++) {
    LandState land_state = gameData->land_state[i];
    UnitsLandMobile land_mobile = land_state.units_land.units_land_mobile;
    strcat(gameStatus, "\n");
    strcat(gameStatus, Players[land_state.owner_index].color);
    strcat(gameStatus, Lands[i].name);
    strcat(gameStatus, ": ");
    strcat(gameStatus, Players[land_state.owner_index].name);
    strcat(gameStatus, " ");
    sprintf(threeCharStr, "%d", land_state.builds_left);
    strcat(gameStatus, threeCharStr);
    strcat(gameStatus, "/");
    sprintf(threeCharStr, "%d", land_state.factory_hp);
    strcat(gameStatus, threeCharStr);
    strcat(gameStatus, "/");
    sprintf(threeCharStr, "%d", land_state.factory_max);
    strcat(gameStatus, threeCharStr);
    strcat(gameStatus, " Conquered: ");
    if (land_state.conquered) {
      strcat(gameStatus, "true\n");
    } else {
      strcat(gameStatus, "false\n");
    }
    if(cache->units_land_grand_total[i] == 0) {
      strcat(gameStatus, "\033[0m");
      continue;
    }
    char buffer[STRING_BUFFER_SIZE]; // Adjust size as needed
    strcat(gameStatus, "             |Tot| 0| 1| 2| 3| 4| 5| 6|\n");
    if (cache->units_land_static[i].infantry > 0) {
      snprintf(buffer, STRING_BUFFER_SIZE, "%s Infantry: |%3d%3d%3d\n",
               player.name, cache->units_land_static[i].infantry,
               land_mobile.infantry_0, land_mobile.infantry_1);
      strcat(gameStatus, buffer);
    }
    if (cache->units_land_static[i].artillery > 0) {
      snprintf(buffer, STRING_BUFFER_SIZE, "%s Artillery:|%3d%3d%3d\n",
               player.name, cache->units_land_static[i].artillery,
               land_mobile.artillery_0, land_mobile.artillery_1);
      strcat(gameStatus, buffer);
    }
    if (cache->units_land_static[i].tanks > 0) {
      snprintf(buffer, STRING_BUFFER_SIZE, "%s Tanks:    |%3d%3d%3d%3d\n",
               player.name, cache->units_land_static[i].tanks,
               land_mobile.tanks_0, land_mobile.tanks_1, land_mobile.tanks_2);
      strcat(gameStatus, buffer);
    }
    if (cache->units_land_static[i].aa_guns > 0) {
      snprintf(buffer, STRING_BUFFER_SIZE, "%s AA Guns:  |%3d%3d%3d\n",
               player.name, cache->units_land_static[i].aa_guns,
               land_mobile.aa_guns_0, land_mobile.aa_guns_1);
      strcat(gameStatus, buffer);
    }
    if (cache->units_land_static[i].fighters > 0) {
      snprintf(buffer, STRING_BUFFER_SIZE, "%s Fighters: |%3d%3d%3d%3d%3d%3d\n",
               player.name, cache->units_land_static[i].fighters,
               land_mobile.fighters_0, land_mobile.fighters_1,
               land_mobile.fighters_2, land_mobile.fighters_3,
               land_mobile.fighters_4);
      strcat(gameStatus, buffer);
    }
    if (cache->units_land_static[i].bombers > 0) {
      snprintf(buffer, STRING_BUFFER_SIZE,
               "%s Bombers:  |%3d%3d%3d%3d%3d%3d%3d%3d\n", player.name,
               cache->units_land_static[i].bombers, land_mobile.bombers_0,
               land_mobile.bombers_1, land_mobile.bombers_2,
               land_mobile.bombers_3, land_mobile.bombers_4,
               land_mobile.bombers_5, land_mobile.bombers_6);
      strcat(gameStatus, buffer);
    }
    strcat(gameStatus, "\033[0m");
    for (int j = 0; j < PLAYERS_COUNT - 1; j++) {
      Player otherPlayer = Players[(player_index + j) % PLAYERS_COUNT];
      UnitsLandStatic other_land_static =
          gameData->land_state[i].units_land.units_land_static[j];
      strcat(gameStatus, player.color);
      if (other_land_static.infantry > 0) {
        strcat(gameStatus, player.name);
        strcat(gameStatus, " Infantry: |");
        sprintf(threeCharStr, "%3d", other_land_static.infantry);
        strcat(gameStatus, threeCharStr);
        strcat(gameStatus, "\n");
      }
      if (other_land_static.artillery > 0) {
        strcat(gameStatus, player.name);
        strcat(gameStatus, " Artillery:|");
        sprintf(threeCharStr, "%3d", other_land_static.artillery);
        strcat(gameStatus, threeCharStr);
        strcat(gameStatus, "\n");
      }
      if (other_land_static.tanks > 0) {
        strcat(gameStatus, player.name);
        strcat(gameStatus, " Tanks:    |");
        sprintf(threeCharStr, "%3d", other_land_static.tanks);
        strcat(gameStatus, threeCharStr);
        strcat(gameStatus, "\n");
      }
      if (other_land_static.aa_guns > 0) {
        strcat(gameStatus, player.name);
        strcat(gameStatus, " AA Guns:  |");
        sprintf(threeCharStr, "%3d", other_land_static.aa_guns);
        strcat(gameStatus, threeCharStr);
        strcat(gameStatus, "\n");
      }
      if (other_land_static.fighters > 0) {
        strcat(gameStatus, player.name);
        strcat(gameStatus, " Fighters: |");
        sprintf(threeCharStr, "%3d", other_land_static.fighters);
        strcat(gameStatus, threeCharStr);
        strcat(gameStatus, "\n");
      }
      if (other_land_static.bombers > 0) {
        strcat(gameStatus, player.name);
        strcat(gameStatus, " Bombers:  |");
        sprintf(threeCharStr, "%3d", other_land_static.bombers);
        strcat(gameStatus, threeCharStr);
        strcat(gameStatus, "\n");
      }
      strcat(gameStatus, "\033[0m");
    }
  }
}
