#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>

typedef enum { Axis, Allies } Team;

#define TOTAL_PLAYERS 2

struct Territory;

typedef struct {
  const char* name;
  const Team team;
  const unsigned char player_index;
  const struct Territory* capital_territory_ptr;
  const bool is_allied[TOTAL_PLAYERS - 1];
  bool is_human;
  int money;
} Player;

#endif
