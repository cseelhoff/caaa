#ifndef LAND_H
#define LAND_H

#include <stdint.h>

#define LANDS_COUNT 5
#define MAX_LAND_TO_SEA_CONNECTIONS 4
#define MAX_LAND_TO_LAND_CONNECTIONS 6

typedef struct {
  char* name;
  uint8_t original_owner_index;
  uint8_t land_value;
  uint8_t sea_conn_count;
  uint8_t land_conn_count;
  uint8_t connected_sea_index[MAX_LAND_TO_SEA_CONNECTIONS];
  uint8_t connected_land_index[MAX_LAND_TO_LAND_CONNECTIONS];
} Land;

extern const Land LANDS[LANDS_COUNT];

#endif
