#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include <stdint.h>

#define PLAYERS_COUNT 5
typedef struct {
  char* name;
  char* capital_name;
  char* color;
  int capital_territory_index;
  uint8_t team;
  uint8_t money;
  bool is_allied[PLAYERS_COUNT]; //alied with self
  bool is_human;
} Player;

extern const Player PLAYERS[PLAYERS_COUNT];

#endif
