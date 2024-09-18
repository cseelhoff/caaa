#ifndef LAND_H
#define LAND_H

#include <array>
#include <cstdint>

#define LANDS_COUNT 5
#define MAX_LAND_TO_SEA_CONNECTIONS 4
#define MAX_LAND_TO_LAND_CONNECTIONS 6

constexpr std::size_t LAND_ALIGNMENT = 32;
using Land = struct {
  const char* name;
  uint8_t original_owner_index;
  uint8_t land_value;
  uint8_t sea_conn_count;
  uint8_t land_conn_count;
  std::array<uint8_t, MAX_LAND_TO_SEA_CONNECTIONS> connected_sea_index;
  std::array<uint8_t, MAX_LAND_TO_LAND_CONNECTIONS> connected_land_index;
} __attribute__((aligned(LAND_ALIGNMENT)));

extern const std::array<Land, LANDS_COUNT> LANDS;

#endif
