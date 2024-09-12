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

void initialize_sea_connections();
void initialize_sea_dist(CanalState canal_idx);
void populate_initial_distances_sea(CanalState canal_idx);
void process_canals(CanalState canal_idx);
void floyd_warshall_sea(CanalState canal_idx);
void generate_seaMoveAllDestination();
SeaIndex get_sea_conn_count(LandIndex land_idx);
void set_sea_dist(CanalState canal_idx, SeaIndex src_sea, SeaIndex dst_sea, Distance dist);
void set_sea_path(Distance hop, CanalState canal_state, SeaIndex src_sea, SeaIndex dst_sea,
                  SeaIndex cur_sea);
char* get_sea_name(SeaIndex sea_idx);
LandConnections* get_s2l_conn(SeaIndex src_sea);
LandConnIndex get_s2l_count(SeaIndex src_sea);
SeaConnections* get_s2s_conn(SeaIndex src_sea);
SeaConnIndex get_s2s_count(SeaIndex src_sea);
Distance get_sea_dist(CanalState canal_state, SeaIndex src_sea, SeaIndex dst_sea);
void add_seas_within_x_moves(Distance moves, CanalState canal_state, SeaIndex src_sea,
                             SeaIndex dst_sea);
SeaIndex get_sea_path1(CanalState canal_state, SeaIndex src_sea, SeaIndex dst_sea);
SeaIndex get_sea_path1_alt(CanalState canal_state, SeaIndex src_sea, SeaIndex dst_sea);
SeaIndex get_seas_within_1_move_count(CanalState canal_state, SeaIndex src_sea);
SeaArray* get_seas_within_1_move(CanalState canal_state, SeaIndex src_sea);
SeaIndex get_seas_within_2_moves_count(CanalState canal_state, SeaIndex src_sea);
SeaArray* get_seas_within_2_moves(CanalState canal_state, SeaIndex src_sea);
SeaIndex get_sea_from_array(SeaArray* sea_array, SeaConnIndex conn_idx);
