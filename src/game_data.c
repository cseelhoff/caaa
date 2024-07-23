#include "game_data.h"
#include "cJSON.h"
#include "inactive_unit_stack.h"
#include "json_data_loader.h"
#include "static_unit.h"
#include "unit_type.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
  const UnitTypes unitTypes = getUnitTypesFromJson("./data/unit_types.json");
  UnitTypes cargo = {};

  
  const UnitHealths unitHealths = createUnitHealths(unitTypes);

  int inactive_unit_stacks_count = players.count * unitHealths_count;
  for (int i = 0; i < players.count; i++) {
    inactive_unit_stacks_count += unitHealths[i].unit_type->max_land;
  }
  const InactiveUnitStack* inactive_unit_stacks = create_inactive_unit_stacks(
      unitHealths, unitHealths_count, inactive_unit_stacks_count);

  int unitStatuses_count = 0;
  for (int i = 0; i < unitHealths_count; i++) {
    unitStatuses_count += unitTypes[i].max_moves;
  }
  const UnitStatus* unitStatuses =
      create_unit_statuses(unitHealths, unitHealths_count, unitStatuses_count);

  // Load Territories
  cJSON* ters_cjson = loadJsonPath("./data/territories.json", "territories");
  int ters_count = getJsonArrayLength(ters_cjson);
  if (ters_count == 0) {
    printf("No territories found\n");
    return gameDataInstance;
  }
  Territory* territories = getJsonTerritories(ters_cjson, ters_count);

  // Load Connections
  cJSON* cons_cjson = loadJsonPath("./data/connections.json", "connections");
  int cons_count = getJsonArrayLength(cons_cjson);
  if (cons_count == 0) {
    printf("No connections found\n");
    return gameDataInstance;
  }
  Connection* connections =
      getJsonConnections(cons_cjson, cons_count, territories, ters_count);

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
  for (int i = 0; i < territories_count; i++) {
    gameState[gameStateIndex++] = territories[i].factory_max;
    gameState[gameStateIndex++] = territories[i].factory_health;
    gameState[gameStateIndex++] = territories[i].construction_remaining;
    gameState[gameStateIndex++] = territories[i].recently_conquered;
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
  return &gameDataInstance;
}
