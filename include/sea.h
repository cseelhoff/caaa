#ifndef SEA_H
#define SEA_H

#include "config.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
  char* name;
  uint8_t sea_conn_count;
  uint8_t land_conn_count;
  unsigned int sea_index[MAX_SEA_TO_SEA_CONNECTIONS];
  unsigned int connected_land_territory_indicies[MAX_SEA_TO_LAND_CONNECTIONS];
//  bool is_passable_for_player[PLAYERS_COUNT];
//  bool has_blockade;
} Sea;

extern Sea SEAS[SEAS_COUNT];

#endif
