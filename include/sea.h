#pragma once
#include "typedefs.h"
#include <stdbool.h>
#include <stdint.h>

#define SEAS_COUNT 3
#define MAX_SEA_TO_LAND_CONNECTIONS 6
#define MAX_SEA_TO_SEA_CONNECTIONS 7

typedef uint8_t SeaIndex;
typedef uint8_t Distance;
typedef SeaIndex SeaToSeaConnections[MAX_SEA_TO_SEA_CONNECTIONS];
typedef LandIndex SeaToLandConnections[MAX_SEA_TO_LAND_CONNECTIONS];
typedef Distance SeaDistances[SEAS_COUNT];
typedef SeaDistances SeaDistancesSources[SEAS_COUNT];
typedef SeaIndex SeaArray[SEAS_COUNT];
typedef SeaIndexCount SeaCountsArray[SEAS_COUNT];
typedef SeaArray SeaMatrix[SEAS_COUNT];
typedef SeaToSeaConnections SeaConnArray[SEAS_COUNT];

typedef struct {
  char* name;
  SeaIndexCount sea_conn_count;
  LandIndexCount land_conn_count;
  SeaIndex connected_sea_index[MAX_SEA_TO_SEA_CONNECTIONS];
  LandIndex connected_land_index[MAX_SEA_TO_LAND_CONNECTIONS];
} __attribute__((aligned(ALIGNMENT_32))) Sea;

extern const Sea SEAS[SEAS_COUNT];
inline SeaIndexCount get_sea_to_sea_count(SeaIndex src_sea);
static inline void set_sea_to_sea_count(SeaIndex src_sea, SeaIndexCount sea_to_sea_count);
inline SeaToSeaConnections* get_sea_to_sea_conn(SeaIndex src_sea);
inline SeaIndex get_sea_path1(CanalState canal_state, SeaIndex src_sea, SeaIndex dst_sea);
inline SeaIndex get_sea_path1_alt(CanalState canal_state, SeaIndex src_sea, SeaIndex dst_sea);
