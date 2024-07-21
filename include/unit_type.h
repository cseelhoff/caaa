#ifndef UNIT_TYPE_H
#define UNIT_TYPE_H

#include <stdbool.h>

typedef struct UnitType {
    const char* name;
    const unsigned char attack;
    const unsigned char defense;
    const unsigned char max_moves;
    const unsigned char max_hits;
    const unsigned char cost;
    const unsigned char max_supportable;
    const unsigned char max_supported;
    const unsigned char weight;
    const bool is_air;
    const unsigned char bombs;
    const bool is_water;
    const unsigned char max_land;
    const unsigned char max_air;
    const unsigned char sub_strength;
    const bool is_anti_sub;
    const unsigned char bombard;
    const unsigned char aa_shots;
} UnitType;

#endif
