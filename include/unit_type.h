#ifndef UNIT_TYPE_H
#define UNIT_TYPE_H

#include <stdbool.h>

typedef struct UnitType {
    const char* name;
    unsigned char attack;
    unsigned char defense;
    unsigned char max_moves;
    unsigned char max_hits;
    unsigned char cost;
    unsigned char max_supportable;
    unsigned char max_supported;
    unsigned char weight;
    bool is_air;
    unsigned char bombs;
    bool is_water;
    unsigned char max_land;
    unsigned char max_air;
    bool is_sub;
    bool is_anti_sub;
    unsigned char bombard;
    unsigned char aa_shots;
} UnitType;

#endif // UNIT_TYPE_H