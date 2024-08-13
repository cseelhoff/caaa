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

//  PACIFIC | USA | ATLANTIC | ENG | BALTIC | GER | RUS | JAP | PAC

Land LANDS[LANDS_COUNT] = {
  "Washington", 4, 8, 2, 0, //owner, money, sea_con, land_conn
  {0, 1, 0, 0}, //sea
  {0, 0, 0, 0, 0, 0}, //land
  "London", 2, 8, 2, 0, 
  {1, 2, 0, 0}, 
  {0, 0, 0, 0, 0, 0},
  "Berlin", 1, 8, 1, 1,
  {2, 0, 0, 0}, 
  {3, 0, 0, 0, 0, 0},
  "Moscow", 0, 8, 0, 2, 
  {0, 0, 0, 0}, 
  {2, 4, 0, 0, 0, 0},
  "Japan", 3, 8, 1, 1,
  {0, 0, 0, 0}, 
  {2, 4, 0, 0, 0, 0}  
};

#endif
