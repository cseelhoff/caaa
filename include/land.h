#pragma once
#include "typedefs.h"
#include "canal.h"
#include "sea.h"
#include <stdint.h>

#define LANDS_COUNT 5
#define MAX_LAND_TO_SEA_CONNECTIONS 4 //land to sea within 2 moves
#define MAX_LAND_TO_LAND_CONNECTIONS 6
#define MIN_LAND_HOPS 1
#define MAX_LAND_HOPS 2
#define LAND_MOVE_SIZE (1 + MAX_LAND_HOPS - MIN_LAND_HOPS)

typedef LandIndex LandArray[LANDS_COUNT];
//typedef LandIndex L2LConnections[MAX_LAND_TO_LAND_CONNECTIONS];
//typedef SeaIndex L2SConnections[MAX_LAND_TO_SEA_CONNECTIONS];

typedef struct {
  char* name;
  PlayerIndex original_owner_index;
  Dollars land_value;
  SeaConnIndex sea_conn_count;
  LandConnIndex land_conn_count;
  SeaConnections sea_connections;
  LandConnections land_connections;
} __attribute__((aligned(ALIGNMENT_32))) Land;

extern Land LANDS[LANDS_COUNT];
extern Dollars LAND_VALUE[LANDS_COUNT];
extern LandArray LANDS_WITHIN_2_MOVES[LANDS_COUNT];
extern LandIndex LANDS_WITHIN_2_MOVES_COUNT[LANDS_COUNT];
extern SeaConnections LOAD_WITHIN_2_MOVES[LANDS_COUNT];
extern LandIndex LOAD_WITHIN_2_MOVES_COUNT[LANDS_COUNT];
extern LandIndex LAND_TO_LAND_COUNT[LANDS_COUNT];
extern LandConnections LAND_TO_LAND_CONN[LANDS_COUNT];
extern SeaIndex LAND_TO_SEA_COUNT[LANDS_COUNT];
extern SeaConnections LAND_TO_SEA_CONN[LANDS_COUNT];

void generate_land_value_array();
LandIndex get_land_to_land(LandIndex land_idx, LandConnIndex conn_idx);
