#pragma once
#include "player.h"
#include <stdint.h>

#define MAX_TEAM_SIZE 3

typedef struct {
  char* name;
  uint8_t players[MAX_TEAM_SIZE];
} Team;

#define TEAMS_COUNT 2

extern const uint8_t TEAM_PLAYERS[TEAMS_COUNT][MAX_TEAM_SIZE];
