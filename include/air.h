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

LandIndex get_land_path1(LandIndex src_land, AirIndex dst_air);
LandIndex get_land_path_alt(LandIndex src_land, AirIndex dst_air);
AirIndex get_airs_winthin_x_moves_count(Distance moves, AirIndex src_air);
AirIndex get_air_from_array(AirIndexArray* air_array, AirConnIndex air_array_idx);
AirIndex get_airs_winthin_x_moves_count(Distance moves, AirIndex src_air);
AirIndex get_airs_winthin_x_moves_count(Distance moves, AirIndex src_air);
AirIndexArray* get_airs_winthin_x_moves(Distance moves, AirIndex src_air);
AirIndexArray* get_airs_within_x_moves(Distance moves, AirIndex src_air);
AirIndex get_airs_within_x_moves_count(Distance moves, AirIndex src_air);
Distance get_air_dist(AirIndex src_air, AirIndex dst_air);
AirIndex get_air_from_a2a(AirConnection* air_conn, AirConnIndex air_conn_idx);
AirConnection* get_a2a_conn(AirIndex air_idx);
LandArray* get_air_to_land_within_x_moves(Distance moves, AirIndex src_air);
LandIndex get_air_to_land_within_x_moves_count(Distance moves, AirIndex src_air);
AirIndex get_a2a_count(AirIndex air_idx);
AirDistances* get_land_to_air_dist(LandIndex land_idx);
void set_air_distance(AirDistances* air_dist, AirIndex dst_air, Distance dist);
LandIndex get_land_from_conn(LandConnections* land_connections, LandConnIndex land_conn_idx);
AirIndex convert_land_to_air(LandIndex land_idx);
void set_land_dist(LandIndex src_land, AirIndex dst_air, Distance dist);
SeaIndex get_sea_from_conn(SeaConnections* land_to_sea_conn, SeaConnIndex conn_idx);
AirIndex convert_sea_to_air(SeaIndex sea_idx);
LandConnIndex get_s2l_count(SeaIndex src_sea);
AirDistances* get_land_dist_land(LandIndex land_idx);
Distance get_air_distance(AirDistances* air_dist, AirIndex air_idx);
void generate_landMoveAllDestination();
void floyd_warshall();
void generate_airMoveAllDestination();
void generate_within_x_moves();
void initialize_air_dist();
void populate_initial_distances();
void floyd_warshall_air();
