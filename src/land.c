#include "land.h"
//  PACIFIC | USA | ATLANTIC | ENG | BALTIC | GER | RUS | JAP | PAC

const Land LANDS[LANDS_COUNT] = {
  "Washington", 4, 10, 2, 0, //owner, value, sea_con_count, land_conn_count
  {0, 1, 0, 0}, //sea
  {0, 0, 0, 0, 0, 0}, //land
  "London", 2, 8, 2, 0, 
  {1, 2, 0, 0}, 
  {0, 0, 0, 0, 0, 0},
  "Berlin", 1, 10, 1, 1,
  {2, 0, 0, 0}, 
  {3, 0, 0, 0, 0, 0},
  "Moscow", 0, 8, 0, 2, 
  {0, 0, 0, 0}, 
  {2, 4, 0, 0, 0, 0},
  "Japan", 3, 8, 1, 1,
  {0, 0, 0, 0}, 
  {3, 0, 0, 0, 0, 0}  
};
