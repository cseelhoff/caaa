#include "game_data.h"
#include "cJSON.h"
#include "config.h"
#include "json_data_loader.h"
#include "team.h"
#include "territory.h"
#include "unit_type.h"
#include "unit_health.h"
#include "connection.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

GameData* initializeGameData(int a) {
  // use mallac for GameData gameDataInstance
  GameData* gameDataInstance = malloc(sizeof(GameData));
  // print out current directory
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("Current directory: %s\n", cwd);
  } else {
    printf("Failed to get current directory\n");
    return gameDataInstance;
  }

  // Load Players
  Players players = getPlayersFromJson("./data/players.json");
  Teams teams = getTeamsFromJson("./data/teams.json", players);
  const UnitTypes unitTypes = getUnitTypesFromJson("./data/unit_types.json");
  //UnitTypes cargo = {};

  const UnitHealths unitHealths = createUnitHealths(unitTypes);
  const Territories territories =
      getTerritoriesFromJson("./data/territories.json", players);
  const Connections connections =
      getConnectionsFromJson("./data/connections.json", territories);  

  TerritoryState territoriesStates[TERRITORIES_COUNT];
  for (int i = 0; i < TERRITORIES_COUNT; i++) {
    territoriesStates[i].builds_left = territories.array[i].builds_left;
    territoriesStates[i].factory_hp = territories.array[i].factory_hp;
    territoriesStates[i].factory_max = territories.array[i].factory_max;
    territoriesStates[i].newly_conquered = territories.array[i].newly_conquered;
  }

  uint8_t money[PLAYERS_COUNT];
  for (int i = 0; i < PLAYERS_COUNT; i++) {
    money[i] = players.array[i].money;
  }

  uint8_t unitCounts[PLAYERS_COUNT][TERRITORIES_COUNT][UNIT_HEALTHS_COUNT];
  for (int playerIndex = 0; playerIndex < PLAYERS_COUNT; playerIndex++) {
    for (int territoryIndex = 0; territoryIndex < TERRITORIES_COUNT;
         territoryIndex++) {
      for (int unitIndex = 0; unitIndex < UNIT_HEALTHS_COUNT; unitIndex++) {
        unitCounts[playerIndex][territoryIndex][unitIndex] = 0;
      }
    }
  }

  uint8_t mobileCounts[PLAYERS_COUNT][TERRITORIES_COUNT][MOBILE_UNITS_COUNT];
  for (int playerIndex = 0; playerIndex < PLAYERS_COUNT; playerIndex++) {
    for (int terIndex = 0; terIndex < TERRITORIES_COUNT; terIndex++) {
      for (int mobileIndex = 0; mobileIndex < MOBILE_UNITS_COUNT; mobileIndex++) {
        mobileCounts[playerIndex][territoryIndex][mobileIndex] = 0;
      }
    }
  }

  uint8_t gameState[] = {
      // fixed memory positions...
      // phase
      0, // 0 = combat move, 1 = land planes/purchase/place units
      // russia territory 1
      8, // factory_max in russia
      8, // factory_health in russia
      8, // construction points remaining in russia
      0, // was russia recently conquered
      // germany territory 2
      8, // factory_max in germany
      8, // factory_health in germany
      8, // construction points remaining in germany
      0, // was germany recently conquered
      // dynamic memory positions...
      // inactive/total unit statuses
      // russia (current player)
      0, // rus money (current player)
      0, // ger money (current player+1)
      // russia territory 1
      1, // is russia owned by russia (current player)
      1, // is russia owned by russian ally
      // russia units in russia
      0, // qty of rus art in russia
      // germany territory 1
      1, // is germany owned by russia (current player)
      1, // is germany owned by russian ally
      // russia units in germany
      0, // qty of rus art in germany
      // germany (current player)
      0, // germany money (current player)
      // russia territory 1
      1, // is russia owned by germany (current player+1)
      1, // is russia owned by german ally
      // russia units in russia
      0, // qty of ger art in russia
      // germany territory 1
      1, // is germany owned by germany (current player)
      1, // is germany owned by german ally
      // german units in germany
      0, // qty of german art in germany
      // active unit statuses (current player)
      0, // qty of rus art in russia with 1 move left
      0, // qty of rus art in germany with 1 move left

      // note on land units on transports:
      // before a transport is split into statuses, it needs to be split into
      // cargo
      // statuses a russian transport could fall under any of these types (45
      // yikes
      // - 8 if no ally coop) +1 empty:
      // 1. a transport with 1 russian infantry
      // 2. a transport with 1 russian tank
      // 3. a transport with 1 russian artillery
      // 4. a transport with 1 russian AA
      // 5. a transport with 1 british infantry
      // 6. a transport with 1 british tank
      // 7. a transport with 1 british artillery
      // 8. a transport with 1 british AA
      // 9. a transport with 1 american infantry
      // 10. a transport with 1 american tank
      // 11. a transport with 1 american artillery
      // 12. a transport with 1 american AA
      // 13. a transport with 1 russian infantry and 1 russian infantry
      // 14. a transport with 1 russian infantry and 1 russian tank
      // 15. a transport with 1 russian infantry and 1 russian artillery
      // 16. a transport with 1 russian infantry and 1 russian AA
      // 17. a transport with 1 russian infantry and 1 british infantry
      // 18. a transport with 1 russian infantry and 1 british tank
      // 19. a transport with 1 russian infantry and 1 british artillery
      // 20. a transport with 1 russian infantry and 1 british AA
      // 21. a transport with 1 russian infantry and 1 american infantry
      // 22. a transport with 1 russian infantry and 1 american tank
      // 23. a transport with 1 russian infantry and 1 american artillery
      // 24. a transport with 1 russian infantry and 1 american AA
      // 25. a transport with 1 russian tank and 1 british infantry
      // 26. a transport with 1 russian tank and 1 american infantry
      // 27. a transport with 1 russian artillery and 1 british infantry
      // 28. a transport with 1 russian artillery and 1 american infantry
      // 29. a transport with 1 russian AA and 1 british infantry
      // 30. a transport with 1 russian AA and 1 american infantry
      // 31. a transport with 1 british infantry and 1 british infantry
      // 32. a transport with 1 british infantry and 1 british tank
      // 33. a transport with 1 british infantry and 1 british artillery
      // 34. a transport with 1 british infantry and 1 british AA
      // 35. a transport with 1 british infantry and 1 american infantry
      // 36. a transport with 1 british infantry and 1 american tank
      // 37. a transport with 1 british infantry and 1 american artillery
      // 38. a transport with 1 british infantry and 1 american AA
      // 39. a transport with 1 british tank and 1 american infantry
      // 40. a transport with 1 british artillery and 1 american infantry
      // 41. a transport with 1 british AA and 1 american infantry
      // 42. a transport with 1 american infantry and 1 american infantry
      // 43. a transport with 1 american infantry and 1 american tank
      // 44. a transport with 1 american infantry and 1 american artillery
      // 45. a transport with 1 american infantry and 1 american AA

  };
  int gameStateIndex = 0;
  for (int i = 0; i < territories.count; i++) {
    gameState[gameStateIndex++] = territories.array[i].factory_max;
    gameState[gameStateIndex++] = territories.array[i].factory_hp;
    gameState[gameStateIndex++] = territories.array[i].builds_left;
    gameState[gameStateIndex++] = territories.array[i].newly_conquered;
  }
  /*
  for (int i = 0; i < players_count; i++) {
    gameState[gameStateIndex++] = players[i].money;
    for (int j = 0; j < territories_count; j++) {
      gameState[gameStateIndex++] = territories[j].is_owned_by_current_player;
      gameState[gameStateIndex++] = territories[j].is_ally_owned;
      for (int k = 0; k < inactive_unit_stacks_count; k++) {
        gameState[gameStateIndex++] =
  territories[j].inactive_armies[i][k].unit_count;
      }
    }
  }
  for (int i = 0; i < active_unit_stacks_count; i++) {
    gameState[gameStateIndex++] = active_unit_stacks[i].unit_count;
  }
  */
  return gameDataInstance;
}
