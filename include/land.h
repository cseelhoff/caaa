#ifndef LAND_H
#define LAND_H

#include <array>

#define LANDS_COUNT 5
#define MAX_LAND_TO_SEA_CONNECTIONS 4
#define MAX_LAND_TO_LAND_CONNECTIONS 6
#define LAND_ALIGNMENT 64

using L2LConn = std::array<int, MAX_LAND_TO_LAND_CONNECTIONS>;
using L2SConn = std::array<int, MAX_LAND_TO_SEA_CONNECTIONS>;
using LandArray = std::array<int, LANDS_COUNT>;
using PtrLandArray = std::array<int*, LANDS_COUNT>;

struct Land {
  const char* name;
  int original_owner_index;
  int land_value;
  int sea_conn_count;
  int land_conn_count;
  L2SConn sea_conns;
  L2LConn land_conns;
} __attribute__((aligned(LAND_ALIGNMENT)));

using LandStructs = std::array<Land, LANDS_COUNT>;

extern const LandStructs LANDS;

#endif
