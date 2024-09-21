#pragma once
#include <array>
#include <sys/types.h>

#define LANDS_COUNT 5
#define MAX_LAND_TO_SEA_CONNECTIONS 4
#define MAX_LAND_TO_LAND_CONNECTIONS 6
#define LAND_ALIGNMENT 64

using L2LConn = std::array<uint, MAX_LAND_TO_LAND_CONNECTIONS>;
using L2SConn = std::array<uint, MAX_LAND_TO_SEA_CONNECTIONS>;
using LandArray = std::array<uint, LANDS_COUNT>;
using PtrLandArray = std::array<uint*, LANDS_COUNT>;

struct Land {
  const char* name;
  uint original_owner_index;
  uint land_value;
  uint sea_conn_count;
  uint land_conn_count;
  L2SConn sea_conns;
  L2LConn land_conns;
} __attribute__((aligned(LAND_ALIGNMENT)));

using LandStructs = std::array<Land, LANDS_COUNT>;

extern const LandStructs LANDS;
