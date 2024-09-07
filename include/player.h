#pragma once
#include "typedefs.h"
#include <stdbool.h>
#include <stdint.h>

#define PLAYERS_COUNT 5
typedef struct {
  char* name;
  char* capital_name;
  char* color;
  int capital_territory_index;
  uint8_t team;
  bool is_allied[PLAYERS_COUNT]; // alied with self
  bool is_human;
} Player;

// extern const Player PLAYERS[PLAYERS_COUNT];
bool are_players_allied(PlayerIndex player_idx, PlayerIndex ally_idx);
// Player get_player(PlayerIndex player_idx);
bool is_player_human(PlayerIndex player_idx);
LandIndex get_player_capital(PlayerIndex player_idx);
char* get_player_name(PlayerIndex player_idx);
char* get_player_color(PlayerIndex player_idx);
