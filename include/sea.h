#ifndef SEA_H
#define SEA_H

#include <array>
#include <limits>
#include <sys/types.h>

#define SEAS_COUNT 3
#define MAX_SEA_TO_LAND_CONNECTIONS 6
#define MAX_SEA_TO_SEA_CONNECTIONS 7
#define MAX_INT std::numeric_limits<int>::max()

using SeaArray = std::array<uint, SEAS_COUNT>;
using S2SConn = std::array<uint, MAX_SEA_TO_SEA_CONNECTIONS>;
using S2LConn = std::array<uint, MAX_SEA_TO_LAND_CONNECTIONS>;
using SeaSeaArray = std::array<SeaArray, SEAS_COUNT>;

constexpr int SEA_ALIGNMENT = 128;

struct Sea {
  const char* name;
  uint sea_conn_count;
  uint land_conn_count;
  S2SConn sea_conns;
  S2LConn land_conns;
} __attribute__((aligned(SEA_ALIGNMENT)));

using SeaStructs = std::array<Sea, SEAS_COUNT>;

extern const SeaStructs SEAS;

#endif
