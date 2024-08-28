#ifndef UNITS_H
#define UNITS_H

#include "aa_gun.h"
#include "artillery.h"
#include "battleship.h"
#include "bomber.h"
#include "carrier.h"
#include "cruiser.h"
#include "destroyer.h"
#include "fighter.h"
#include "infantry.h"
#include "sub.h"
#include "tank.h"
#include "transport.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
  FIGHTERS_SEA,
  TRANS_EMPTY,
  TRANS_1I,
  TRANS_1A,
  TRANS_1T,
  TRANS_2I,
  TRANS_1I_1A,
  TRANS_1I_1T,
  SUBMARINES,
  CARRIERS,
  DESTROYERS,
  CRUISERS,
  BATTLESHIPS,
  BS_DAMAGED,
  BOMBERS_SEA,
  SEA_UNIT_TYPES_COUNT
} SeaUnitType;

typedef enum {
  FIGHTERS_LAND,
  BOMBERS_LAND,
  INFANTRY,
  ARTILLERY,
  TANKS,
  AA_GUNS,
  LAND_UNIT_TYPES_COUNT
} LandUnitType;

typedef enum {
  FIGHTERS_AIR,
  BOMBERS_AIR,
  AIR_UNIT_TYPES_COUNT
} AirUnitType;

#define MAX_UINT8_T 255

#define COST_UNIT_SEA_COUNT 7

extern const char* NAMES_UNIT_LAND[LAND_UNIT_TYPES_COUNT];
extern const uint8_t UNIT_WEIGHTS[LAND_UNIT_TYPES_COUNT];
extern const uint8_t ATTACK_UNIT_LAND[LAND_UNIT_TYPES_COUNT];
extern const uint8_t DEFENSE_UNIT_LAND[LAND_UNIT_TYPES_COUNT];
extern const uint8_t MAX_MOVE_LAND[LAND_UNIT_TYPES_COUNT];
extern const uint8_t STATES_MOVE_LAND[LAND_UNIT_TYPES_COUNT];
extern const uint8_t COST_UNIT_LAND[LAND_UNIT_TYPES_COUNT];
extern const char* NAMES_UNIT_SEA[SEA_UNIT_TYPES_COUNT];
extern const uint8_t ATTACK_UNIT_SEA[SEA_UNIT_TYPES_COUNT];
extern const uint8_t DEFENSE_UNIT_SEA[SEA_UNIT_TYPES_COUNT];
extern const uint8_t MAX_MOVE_SEA[SEA_UNIT_TYPES_COUNT];
extern const uint8_t STATES_MOVE_SEA[SEA_UNIT_TYPES_COUNT];
extern const uint8_t COST_UNIT_SEA[SEA_UNIT_TYPES_COUNT];
extern const uint8_t BUY_UNIT_SEA[COST_UNIT_SEA_COUNT];
extern const uint8_t STATES_STAGING[SEA_UNIT_TYPES_COUNT];
extern const uint8_t STATES_UNLOADING[SEA_UNIT_TYPES_COUNT];
extern const uint8_t LOAD_UNIT_TYPE[LAND_UNIT_TYPES_COUNT][SEA_UNIT_TYPES_COUNT];
extern const uint8_t UNLOAD_CARGO1[SEA_UNIT_TYPES_COUNT];
extern const uint8_t UNLOAD_CARGO2[SEA_UNIT_TYPES_COUNT];
extern const uint8_t UNMOVED_SEA[SEA_UNIT_TYPES_COUNT];
extern const uint8_t DONE_MOVING_SEA[SEA_UNIT_TYPES_COUNT];

#endif
