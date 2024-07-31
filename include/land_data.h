#ifndef LAND_DATA_H
#define LAND_DATA_H

#include "config.h"
#include "land.h"

//  PACIFIC | USA | ATLANTIC | ENG | BALTIC | GER | RUS | JAP | PAC

Land Lands[LANDS_COUNT] = {
  "Washington", 4, 8, 2, 0,
  {0, 1, 0, 0}, 
  {0, 0, 0, 0, 0, 0},
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