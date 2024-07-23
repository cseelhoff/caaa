#ifndef PLAYER_H
#define PLAYER_H

#include <cJSON.h>
#include <stdbool.h>
#include <stdint.h>

#define DEF_PLAYER_NAME "DEFAULT PLAYER"
#define DEF_CAPITAL_NAME "DEFAULT PLAYER"
#define DEF_TEAM 0
#define DEF_MONEY 0

struct Territory;

typedef struct {
  char* name;
  char* capital_name;
  struct Territory* capital_territory;
  uint8_t player_index;
  uint8_t team;
  uint8_t money;
  bool* is_allied; //[TOTAL_PLAYERS - 1];
  bool is_human;
} Player;

typedef struct {
  Player* array;
  int count;
} Players;

Players getPlayersFromJson(char* json_path);
Player* getPlayerByName(Players players, char* name);

#endif
