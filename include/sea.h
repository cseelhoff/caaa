#ifndef SEA_H
#define SEA_H

#include <stdint.h>
#include <stdbool.h>

#define SEAS_COUNT 3
#define MAX_SEA_TO_LAND_CONNECTIONS 6
#define MAX_SEA_TO_SEA_CONNECTIONS 7

typedef struct {
  char* name;
  uint8_t sea_conn_count;
  uint8_t land_conn_count;
  uint8_t connected_sea_index[MAX_SEA_TO_SEA_CONNECTIONS];
  uint8_t connected_land_index[MAX_SEA_TO_LAND_CONNECTIONS];
} Sea;

extern const Sea SEAS[SEAS_COUNT];

#endif
