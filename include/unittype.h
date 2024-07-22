#ifndef UNITTYPE_H
#define UNITTYPE_H

#include <stdbool.h>
#include <stdint.h>
#include <cJSON.h>

#define DEF_UT_NAME "DEFAULT UNITTYPE"
#define DEF_ATTACK 0
#define DEF_DEFENSE 0
#define DEF_MAX_MOVES 1
#define DEF_COST 1
#define DEF_BONUS_ATTACK 0
#define DEF_GIVES_BONUS 0
#define DEF_WEIGHT 6
#define DEF_RAID_STRENGTH 0
#define DEF_MAX_LAND 0
#define DEF_MAX_AIR 0
#define DEF_SUB_STRENGTH 0
#define DEF_BOMBARD 0
#define DEF_AA_SHOTS 0
#define DEF_IS_AIR false
#define DEF_IS_WATER false
#define DEF_IS_DETECTOR false

typedef struct UnitType {
  char* name;
  uint8_t attack;
  uint8_t defense;
  uint8_t max_moves;
  uint8_t max_hits;
  uint8_t cost;
  uint8_t bonus_attack;
  uint8_t gives_bonus;
  uint8_t weight;
  uint8_t raid_strength;
  uint8_t max_land;
  uint8_t max_air;
  uint8_t sub_strength;
  uint8_t bombard;
  uint8_t aa_shots;
  bool is_air;
  bool is_water;
  bool is_detector;
} UnitType;

UnitType* get_unitTypes_from_cjson(cJSON* unitTypes_cjson, int unitType_count);

#endif
