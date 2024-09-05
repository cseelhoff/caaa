#pragma once
#include "land.h"
#include "sea.h"
#include "typedefs.h"
#include "units/bomber.h"

#define AIRS_COUNT (LANDS_COUNT + SEAS_COUNT)
#define MAX_AIR_TO_AIR_CONNECTIONS 7
#define MIN_AIR_HOPS 2
#define MAX_AIR_HOPS 6
#define AIR_MOVE_SIZE 1 + MAX_AIR_HOPS - MIN_AIR_HOPS

typedef Distance AirDistances[AIRS_COUNT];
typedef AirIndex AirIndexArray[AIRS_COUNT];
typedef AirIndexArray AirIndexMatrix[AIRS_COUNT];
typedef AirIndex AirToAirConnection[MAX_AIR_TO_AIR_CONNECTIONS];
typedef LandIndex AirToLandConnection[LANDS_COUNT];

void initialize_air_dist();
void populate_initial_distances();
void floyd_warshall_air();
