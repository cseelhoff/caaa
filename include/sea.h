#pragma once
#include "canal.h"
#include "typedefs.h"
#include <stdbool.h>
#include <stdint.h>

#define SEAS_COUNT 3
#define MAX_SEA_TO_LAND_CONNECTIONS 6
#define MAX_SEA_TO_SEA_CONNECTIONS 7

typedef SeaIndex SeaConnections[MAX_SEA_TO_SEA_CONNECTIONS];
typedef LandIndex LandConnections[MAX_SEA_TO_LAND_CONNECTIONS];
typedef Distance SeaDistances[SEAS_COUNT];
typedef SeaDistances SeaDistancesSources[SEAS_COUNT];
typedef SeaIndex SeaArray[SEAS_COUNT];
typedef SeaIndex SeaCountsArray[SEAS_COUNT];
typedef SeaArray SeaMatrix[SEAS_COUNT];

typedef struct {
  char* name;
  SeaConnIndex sea_conn_count;
  LandConnIndex land_conn_count;
  SeaConnections sea_connections;
  LandConnections land_connections;
} __attribute__((aligned(ALIGNMENT_32))) Sea;

extern Sea SEAS[SEAS_COUNT];
extern SeaMatrix SEA_PATH1[CANAL_STATES];
extern SeaMatrix SEA_PATH2[CANAL_STATES];
extern SeaMatrix SEA_PATH1_ALT[CANAL_STATES];
extern SeaCountsArray SEAS_WITHIN_X_MOVES_COUNT[2][CANAL_STATES];
extern SeaMatrix SEAS_WITHIN_X_MOVES[2][CANAL_STATES];

void generate_SeaMoveDst(int hop, int src_sea, int dst_sea, int cur_sea, int min_dist, CanalState canal_state);
void initialize_sea_connections();
void initialize_sea_dist(CanalState canal_idx);
void populate_initial_distances_sea(CanalState canal_idx);
void process_canals(CanalState canal_idx);
void floyd_warshall_sea(CanalState canal_idx);
void generate_seaMoveAllDestination();
