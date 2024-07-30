#include "player_data.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Player* getPlayerByName(char* name) {
  for (int i = 0; i < PLAYERS_COUNT; i++) {
    if (strcmp(Players[i].name, name) == 0) {
      return &Players[i];
    }
  }
  printf("Player name: %s not found\n", name);
  exit(1);
}
