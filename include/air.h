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

typedef AirIndex AirIndexArray[AIRS_COUNT];
typedef AirIndexArray AirIndexMatrix[AIRS_COUNT];
typedef AirIndex AirToAirConnection[MAX_AIR_TO_AIR_CONNECTIONS];
typedef LandIndex AirToLandConnection[LANDS_COUNT];

inline LandIndex get_land_path1(LandIndex src_land, AirIndex dst_air);
inline LandIndex get_land_path_alt(LandIndex src_land, AirIndex dst_air);
inline AirIndex convert_sea_to_air(SeaIndex sea_idx);
inline AirIndexArray* get_airs_winthin_x_moves(Movement moves, AirIndex src_air);
inline AirIndexCount get_airs_winthin_x_moves_count(Movement moves, AirIndex src_air);
inline AirIndex get_air_from_array(AirIndexArray* air_array, uint8_t air_array_idx);
inline char* get_air_name(AirIndex air_idx);
inline AirIndexArray* get_land_dist_array(LandIndex land_idx);
inline AirIndexCount get_air_conn_count(AirIndex air_idx);
inline AirIndex get_air_from_a2a(AirToAirConnection* air_conn, uint8_t air_conn_idx);
inline AirToAirConnection* get_air_conn(AirIndex air_idx);
inline Distance get_air_dist(AirIndex src_air, AirIndex dst_air);
inline AirIndexArray* get_airs_within_x_moves(Movement moves, AirIndex src_air);
inline uint8_t get_airs_within_x_moves_count(Movement moves, AirIndex src_air);
inline LandIndex convert_air_to_land(AirIndex air_idx);
inline SeaIndex convert_air_to_sea(AirIndex air_idx);
inline AirToLandConnection* get_air_to_land_within_x_moves(Movement moves, AirIndex src_air);
inline LandIndex get_land_from_a2l(AirToLandConnection* air_to_land_conn, uint8_t land_conn_idx);
inline LandIndexCount get_air_to_land_within_x_moves_count(Movement moves, AirIndex src_air);
