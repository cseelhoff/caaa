#include "team.h"
#include "player.h"
#include "json_data_loader.h"
#include <stdio.h>
#include <stdlib.h>

Teams getTeamsFromJson(char* json_path, Players players) {
  cJSON* teams_cjson = loadJsonPath(json_path, "teams");
  Teams teams = {};
  teams.count = getJsonArrayLength(teams_cjson);
  if (teams.count == 0) {
    printf("No teams found\n");
    cJSON_Delete(teams_cjson);
    return teams;
  }
  teams.array = malloc(teams.count * sizeof(Team));
  if (!teams.array) {
    printf("Memory allocation failed\n");
    cJSON_Delete(teams_cjson);
    return teams;
  }

  int index = 0;
  cJSON* cjson;
  cJSON_ArrayForEach(cjson, teams_cjson) {
    Team t = t;
    t.name = getJsonString(cjson, "name", DEF_TEAM_NAME);
    cJSON* players_cjson = cJSON_GetObjectItemCaseSensitive(cjson, "players");
    cJSON_ArrayForEach(cjson, players_cjson) {
      char* player_name = cJSON_GetStringValue(cjson);
      Player* player = getPlayerByName(players, player_name);
      if (player == NULL) {
        printf("Player name: %s not found\n", player_name);
        continue;
      }
      t.players.array[t.players.count] = *player;
      t.players.count++;
    }
    // Players players;
    teams.array[index] = t;
    index++;
  }

  cJSON_Delete(teams_cjson);
  return teams;
}
