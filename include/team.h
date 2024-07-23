#ifndef TEAM_H
#define TEAM_H

#define DEF_TEAM_NAME "DEFAULT TEAM"

#include "player.h"
#include <sys/types.h>

typedef struct team {
  char* name;
  Players players;
} Team;

typedef struct {
  Team* array;
  u_int8_t count;
} Teams;

Teams getTeamsFromJson(char* json_path);

#endif
