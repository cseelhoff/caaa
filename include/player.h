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
  bool is_allied[PLAYERS_COUNT - 1];
  bool is_human;
} Player;

Player Players[PLAYERS_COUNT] = {
  "Rus", "Rus", "\033[1;31m", 0, 0, 0, {false, true, false, true}, true,
  "Ger", "Ger", "\033[1;34m", 1, 1, 0, {false, true, false, false}, true,
  "Eng", "Eng", "\033[1;96m", 2, 2, 0, {false, true, true, false}, true,
  "Jap", "Jap", "\033[1;33m", 3, 3, 0, {false, true, false, false}, true,
  "USA", "USA", "\033[1;32m", 4, 4, 0, {true, false, true, false}, true
};

extern const char* PLAYER_COLORS[PLAYERS_COUNT];
extern const char* PLAYER_NAMES[PLAYERS_COUNT];
extern const uint8_t PLAYER_TEAMS[PLAYERS_COUNT];
#endif
