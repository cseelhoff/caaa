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
  unsigned int sea_index[MAX_SEA_TO_SEA_CONNECTIONS];
  unsigned int connected_land_territory_indicies[MAX_SEA_TO_LAND_CONNECTIONS];
} Sea;

//  ATLANTIC         BALTIC         ATLANTIC
//  PACIFIC | USA | ATLANTIC | ENG | BALTIC | GER | RUS | JAP | PAC

Sea SEAS[SEAS_COUNT] = {
    "Pacific",  1, 2, {1, 0, 0, 0, 0, 0, 0}, {0, 4, 0, 0, 0, 0},
    "Atlantic", 2, 2, {0, 2, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0},
    "Baltic",   1, 2, {1, 0, 0, 0, 0, 0, 0}, {0, 4, 0, 0, 0, 0}};

const uint8_t seaMove2Destination[SEAS_COUNT][SEAS_COUNT] = {0};
const uint8_t seaMove1Destination[SEAS_COUNT][SEAS_COUNT] = {0};
const uint8_t seaMove1DestinationAlt[SEAS_COUNT][SEAS_COUNT] = {0};
const uint8_t seaDistanceMap[SEAS_COUNT][SEAS_COUNT] = {0};

#endif
