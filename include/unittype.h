#ifndef UNITTYPE_H
#define UNITTYPE_H

#include <stdbool.h>

#define UT_DEF_NAME "DEFAULT UNITTYPE"
#define UT_DEF_ATTACK 0
#define UT_DEF_DEFENSE 0
#define UT_DEF_MAX_MOVES 1
#define UT_DEF_COST 1
#define UT_DEF_MAX_SUPPORTABLE 0
#define UT_DEF_MAX_SUPPORTED 0
#define UT_DEF_WEIGHT 6
#define UT_DEF_RAID_STRENGTH 0
#define UT_DEF_MAX_LAND 0
#define UT_DEF_MAX_AIR 0
#define UT_DEF_SUB_STRENGTH 0
#define UT_DEF_BOMBARD 0
#define UT_DEF_AA_SHOTS 0
#define UT_DEF_IS_AIR false
#define UT_DEF_IS_WATER false
#define UT_DEF_IS_DETECTOR false

typedef struct UnitType {
  char* name;
  unsigned char attack;
  unsigned char defense;
  unsigned char max_moves;
  unsigned char max_hits;
  unsigned char cost;
  unsigned char max_supportable;
  unsigned char max_supported;
  unsigned char weight;
  unsigned char raid_strength;
  unsigned char max_land;
  unsigned char max_air;
  unsigned char sub_strength;
  unsigned char bombard;
  unsigned char aa_shots;
  bool is_air;
  bool is_water;
  bool is_detector;
} UnitType;

#endif
