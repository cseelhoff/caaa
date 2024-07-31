#ifndef TERRITORRY_DATA_H
#define TERRITORRY_DATA_H

#include "config.h"
#include "territory.h"

Territory Territories[TERRITORIES_COUNT] = {
  "Moscow", 0, 0, {0, 1}, {1}, 2, 1, 8, 8, 8, 8, true, true, false,
  "Berlin", 1, 1, {0, 1}, {0}, 2, 1, 8, 8, 8, 8, true, true, false
};

#endif