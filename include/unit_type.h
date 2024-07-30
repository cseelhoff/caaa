#ifndef UNITTYPE_H
#define UNITTYPE_H

#include "config.h"
#include <stdbool.h>
#include <stdint.h>

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

UnitType UnitTypes[UNIT_TYPES_COUNT];

void loadUnitTypesFromJson(char* json_path);

#endif
