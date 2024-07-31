#ifndef PLAYER_DATA_H
#define PLAYER_DATA_H
#include "player.h"

Player Players[PLAYERS_COUNT] = {
  "Rus", "Rus", 0, 0, 0, {false, true, false, true}, true,
  "Ger", "Ger", 1, 1, 0, {false, true, false, false}, true,
  "Eng", "Eng", 2, 2, 0, {false, true, true, false}, true,
  "Jap", "Jap", 3, 3, 0, {false, true, false, false}, true,
  "USA", "USA", 4, 4, 0, {true, false, true, false}, true
};

#endif