#pragma once
#include "typedefs.h"
#include "canal.h"
#include "sea.h"
#include <stdint.h>

#define LANDS_COUNT 5
#define MAX_LAND_TO_SEA_CONNECTIONS 4
#define MAX_LAND_TO_LAND_CONNECTIONS 6
#define MIN_LAND_HOPS 1
#define MAX_LAND_HOPS 2
#define LAND_MOVE_SIZE (1 + MAX_LAND_HOPS - MIN_LAND_HOPS)

typedef struct {
  char* name;
  PlayerIndex original_owner_index;
  Dollars land_value;
  SeaIndexCount sea_conn_count;
  LandIndexCount land_conn_count;
  SeaIndex connected_sea_index[MAX_LAND_TO_SEA_CONNECTIONS];
  LandIndex connected_land_index[MAX_LAND_TO_LAND_CONNECTIONS];
} __attribute__((aligned(ALIGNMENT_32))) Land;

static inline LandIndexCount get_land_conn_count(LandIndex land_idx);
inline LandIndex get_connected_land_index(LandIndex land_idx, LandConnectionIndex conn_idx);
inline SeaIndexCount get_sea_conn_count(LandIndex land_idx);
inline AirIndex get_land_to_sea(LandIndex land_idx, SeaConnectionIndex sea_conn_idx);
inline Dollars get_land_value(LandIndex land_idx);
inline SeaIndex* get_sea_dist(CanalState canal_state);
inline char* get_land_name(LandIndex land_idx);
inline LandIndexCount get_land_to_land_count(LandIndex land_idx);
inline LandIndex* get_land_to_land_conn(LandIndex land_idx);
inline SeaIndexCount get_land_to_sea_count(LandIndex land_idx);
inline SeaIndex* get_land_to_sea_conn(LandIndex land_idx);
inline LandIndexCount get_lands_within_2_moves_count(LandIndex land_idx);
inline LandIndexArray get_lands_within_2_moves(LandIndex land_idx);
