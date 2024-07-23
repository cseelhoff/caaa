#include "player.h"
#include "territory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "json_data_loader.h"

Player* getJsonPlayers(cJSON* players_cjson, int player_count) {
  // Allocate memory for the array of Player structures
  Player* players = malloc(player_count * sizeof(Player));
  if (!players) {
    printf("Memory allocation failed\n");
    cJSON_Delete(players_cjson);
    return players;
  }
  int index = 0;
  cJSON* player;
  cJSON_ArrayForEach(player, players_cjson) {
    Player p = p;
    p.player_index = index;
    p.name = getJsonString(player, "name", DEF_PLAYER_NAME);
    p.capital_name = getJsonString(player, "capital_name", DEF_CAPITAL_NAME);
    p.team = getJsonUint8_t(player, "team", DEF_TEAM);
    p.money = getJsonUint8_t(player, "money", DEF_MONEY);
    index++;
  }
  return players;
}

Player* getPlayerByName(Player* players, int player_count, char* original_owner) {
  for (int i = 0; i < player_count; i++) {
    if (strcmp(players[i].name, original_owner) == 0) {
      return &players[i];
    }
  }
  printf("Player name: %s not found\n", original_owner);
  return NULL;
}
