#pragma once
#include "typedefs.h"
#include "land.h"
#include "sea.h"
#include "units/bomber.h"

#define AIRS_COUNT (LANDS_COUNT + SEAS_COUNT)
#define MAX_AIR_TO_AIR_CONNECTIONS 7
#define MIN_AIR_HOPS 2
#define MAX_AIR_HOPS 6
#define AIR_MOVE_SIZE 1 + MAX_AIR_HOPS - MIN_AIR_HOPS

inline LandIndex get_land_path1(LandIndex src_land, AirIndex dst_air);
inline LandIndex get_land_path_alt(LandIndex src_land, AirIndex dst_air);
inline AirIndex convert_sea_to_air(SeaIndex sea_idx);