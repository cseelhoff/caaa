#include "player.h"
#include "json_data_loader.h"
#include "territory.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Players getPlayersFromJson(char* json_path) {
  cJSON* players_cjson = loadJsonPath(json_path, "players");
  Players players = {};
  players.count = getJsonArrayLength(players_cjson);

  if (players.count == 0) {
    printf("No players found\n");
    cJSON_Delete(players_cjson);
    return players;
  }

  players.array = malloc(players.count * sizeof(Player));
  if (!players.array) {
    printf("Memory allocation failed\n");
    cJSON_Delete(players_cjson);
    return players;
  }

  int index = 0;
  cJSON* cjson;
  cJSON_ArrayForEach(cjson, players_cjson) {
    Player p = p;
    p.player_index = index;
    p.name = getJsonString(cjson, "name", DEF_PLAYER_NAME);
    p.capital_name = getJsonString(cjson, "capital_name", DEF_CAPITAL_NAME);
    p.team = getJsonUint8_t(cjson, "team", DEF_TEAM);
    p.money = getJsonUint8_t(cjson, "money", DEF_MONEY);
    p.is_allied = malloc(players.count * sizeof(bool));
    p.is_human = false;
    players.array[index] = p;
    index++;
  }

  cJSON_Delete(players_cjson);
  return players;
}

Player* getPlayerByName(Players players, char* name) {
  for (int i = 0; i < players.count; i++) {
    if (strcmp(players.array[i].name, name) == 0) {
      return &players.array[i];
    }
  }
  printf("Player name: %s not found\n", name);
  return NULL;
}
