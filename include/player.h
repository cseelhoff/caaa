#ifndef PLAYER_H
#define PLAYER_H

#include "config.h"
#include <stdbool.h>
#include <stdint.h>

struct Territory;

typedef struct {
  char* name;
  char* capital_name;
  char* color;
  int capital_territory_index;
  uint8_t team;
  uint8_t money;
  bool is_allied[PLAYERS_COUNT - 1];
  bool is_human;
} Player;

Player* getPlayerByName(char* name);

extern Player Players[PLAYERS_COUNT];

#endif
