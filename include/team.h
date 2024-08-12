#ifndef TEAM_H
#define TEAM_H

#include "player.h"
#include <stdint.h>

#define MAX_TEAM_SIZE 3

typedef struct {
  char* name;
  uint8_t players[MAX_TEAM_SIZE];
} Team;

#define TEAMS_COUNT 2

Team Teams[TEAMS_COUNT] = {
    "Allies", {0, 2, 4},
    "Axis", {1, 3}
};

extern const uint8_t TEAM_PLAYERS[TEAMS_COUNT][MAX_TEAM_SIZE];

#endif
