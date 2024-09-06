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
typedef AirIndex AirConnection[MAX_AIR_TO_AIR_CONNECTIONS];

extern AirDistances LAND_DIST[LANDS_COUNT];
extern LandIndex land_path1[LANDS_COUNT][AIRS_COUNT];
extern LandIndex LAND_PATH_ALT[LANDS_COUNT][AIRS_COUNT];
extern AirIndex AIR_WITHIN_X_MOVES_COUNT[BOMBER_MOVES_MAX][AIRS_COUNT];
extern AirIndex AIR_CONN_COUNT[AIRS_COUNT];
extern AirIndexMatrix AIR_WITHIN_X_MOVES[BOMBER_MOVES_MAX];
extern AirConnection AIR_CONNECTIONS[AIRS_COUNT];
extern AirDistances AIR_DIST[AIRS_COUNT];
extern LandArray AIR_TO_LAND_WITHIN_X_MOVES[BOMBER_MOVES_MAX][AIRS_COUNT];
extern LandConnIndex AIR_TO_LAND_WITHIN_X_MOVES_COUNT[BOMBER_MOVES_MAX][AIRS_COUNT];

AirDistances* get_land_to_air_dist(LandIndex land_idx);
void set_air_distance(AirDistances* air_dist, AirIndex dst_air, Distance dist);
LandConnections* get_l2l_conn(LandIndex land_idx);
LandIndex get_land_from_conn(LandConnections* land_connections, LandConnIndex land_conn_idx);
AirIndex convert_land_to_air(LandIndex land_idx);
void set_land_dist(LandIndex src_land, AirIndex dst_air, Distance dist);
SeaConnIndex get_l2s_count(LandIndex land_idx);
AirIndex get_land_to_sea(LandIndex land_idx, SeaConnIndex sea_conn_idx);
SeaConnections* get_l2s_conn(LandIndex land_idx);
SeaIndex get_sea_from_conn(SeaConnections* land_to_sea_conn, SeaConnIndex conn_idx);
AirIndex convert_sea_to_air(SeaIndex sea_idx);
LandConnections* get_s2l_conn(SeaIndex src_sea);
LandConnIndex get_s2l_count(SeaIndex src_sea);
SeaConnections* get_s2s_conn(SeaIndex src_sea);
SeaIndex get_s2s_count(SeaIndex src_sea);
LandIndex* get_l2l_count_ref(LandIndex land_idx);
LandIndex* get_lands_within_2_moves_count_ref(LandIndex land_idx);
LandArray* get_lands_within_2_moves(LandIndex land_idx);
SeaConnections* get_load_within_2_moves(LandIndex land_idx);
LandIndex* get_load_within_2_moves_count_ref(LandIndex land_idx);
Distance get_sea_dist(CanalState canal_state, SeaIndex src_sea, SeaIndex dst_sea);
void add_seas_within_x_moves(Distance moves, CanalState canal_state, SeaIndex src_sea,
                             SeaIndex dst_sea);
AirDistances* get_land_dist_land(LandIndex land_idx);
Distance get_air_distance(AirDistances* air_dist, AirIndex air_idx);

void generate_landMoveAllDestination();
void floyd_warshall();
void generate_airMoveAllDestination();
void generate_within_x_moves();
void initialize_air_dist();
void populate_initial_distances();
void floyd_warshall_air();
