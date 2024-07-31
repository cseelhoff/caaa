#ifndef TEAM_H
#define TEAM_H

#include "config.h"
#include "player.h"
#include <stdint.h>

typedef struct {
  char* name;
  uint8_t players[MAX_TEAM_SIZE];
} Team;

#endif
