#ifndef TEAM_H
#define TEAM_H

#include "config.h"
#include "player.h"
#include <stdint.h>

typedef struct team {
  char* name;
  Player* players[MAX_TEAM_SIZE];
} Team;

Team* Teams[TEAMS_COUNT];

void loadTeamsFromJson(char* json_path);

#endif
