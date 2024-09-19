#ifndef SEA_H
#define SEA_H

#include <array>

#define SEAS_COUNT 3
#define MAX_SEA_TO_LAND_CONNECTIONS 6
#define MAX_SEA_TO_SEA_CONNECTIONS 7

using SeaArray = std::array<int, SEAS_COUNT>;
using S2SConn = std::array<int, MAX_SEA_TO_SEA_CONNECTIONS>;
using S2LConn = std::array<int, MAX_SEA_TO_LAND_CONNECTIONS>;
using SeaSeaArray = std::array<SeaArray, SEAS_COUNT>;

constexpr int SEA_ALIGNMENT = 128;

using Sea = struct {
  const char* name;
  int sea_conn_count;
  int land_conn_count;
  S2SConn connected_sea_index;
  S2LConn connected_land_index;
} __attribute__((aligned(SEA_ALIGNMENT)));

using SeaStructs = std::array<Sea, SEAS_COUNT>;

extern const SeaStructs SEAS;

#endif
