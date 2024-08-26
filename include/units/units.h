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

#define SEA_UNIT_TYPES_COUNT 15
#define FIGHTERS 0
#define TRANS_EMPTY 1
#define TRANS_1I 2
#define TRANS_1A 3
#define TRANS_1T 4
#define TRANS_2I 5
#define TRANS_1I_1A 6
#define TRANS_1I_1T 7
#define SUBMARINES 8
#define DESTROYERS 9
#define CARRIERS 10
#define CRUISERS 11
#define BATTLESHIPS 12
#define BS_DAMAGED 13
#define BOMBERS_SEA 14

#define LAND_UNIT_TYPES_COUNT 6
#define BOMBERS_LAND_AIR 1
#define INFANTRY 2
#define ARTILLERY 3
#define TANKS 4
#define AA_GUNS 5

#define AIR_UNIT_TYPES_COUNT 2
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
