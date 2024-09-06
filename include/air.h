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
typedef LandIndex LandConnection[LANDS_COUNT];

extern AirDistances LAND_DIST[LANDS_COUNT];
extern LandIndex land_path1[LANDS_COUNT][AIRS_COUNT];
extern LandIndex LAND_PATH_ALT[LANDS_COUNT][AIRS_COUNT];
extern AirIndex AIR_WITHIN_X_MOVES_COUNT[BOMBER_MOVES_MAX][AIRS_COUNT];
extern AirIndex AIR_CONN_COUNT[AIRS_COUNT];
extern AirIndexMatrix AIR_WITHIN_X_MOVES[BOMBER_MOVES_MAX];


void generate_landMoveAllDestination();
void floyd_warshall();
void generate_airMoveAllDestination();
void generate_within_x_moves();
void initialize_air_dist();
void populate_initial_distances();
void floyd_warshall_air();
