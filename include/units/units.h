#pragma once
#include "aa_gun.h"
#include "artillery.h"
#include "battleship.h"
#include "bomber.h"
#include "carrier.h"
#include "cruiser.h"
#include "destroyer.h"
#include "fighter.h"
#include "infantry.h"
#include "land.h"
#include "sea.h"
#include "sub.h"
#include "tank.h"
#include "transport.h"
#include "typedefs.h"
#include <stdbool.h>
#include <stdint.h>

#define DEFENDER_LAND_UNIT_TYPES_COUNT 6
#define ATTACKER_LAND_UNIT_TYPES_COUNT_1 3
#define ATTACKER_LAND_UNIT_TYPES_COUNT_2 2
#define DEFENDER_SEA_UNIT_TYPES_COUNT 13
#define ATTACKER_SEA_UNIT_TYPES_COUNT_1 2
#define ATTACKER_SEA_UNIT_TYPES_COUNT_2 2
#define ATTACKER_SEA_UNIT_TYPES_COUNT_3 8
#define COST_UNIT_SEA_COUNT 7
#define BLOCKADE_UNIT_TYPES_COUNT 5
#define MAX_LAND_UNIT_STATES 7
#define MAX_SEA_UNIT_STATES 5

typedef enum : uint8_t {
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

typedef enum : uint8_t {
  FIGHTERS_LAND,
  BOMBERS_LAND,
  INFANTRY,
  ARTILLERY,
  TANKS,
  AA_GUNS,
  LAND_UNIT_TYPES_COUNT
} LandUnitType;

typedef enum : uint8_t {
  FIGHTERS_AIR,
  BOMBERS_AIR,
  AIR_UNIT_TYPES_COUNT
} AirUnitType;

typedef uint8_t GenericLandUnitState;
typedef uint8_t GenericSeaUnitState;
typedef LandUnitStateSum FighterStateSums[FIGHTER_STATES];
typedef LandUnitStateSum BomberLandStateSums[BOMBER_LAND_STATES];
typedef LandUnitStateSum InfantryStateSums[INFANTRY_STATES];
typedef LandUnitStateSum ArtilleryStateSums[ARTILLERY_STATES];
typedef LandUnitStateSum TanksStateSums[TANK_STATES];
typedef LandUnitStateSum AAGunsStateSums[AA_GUN_STATES];
typedef SeaUnitStateSum FightersSeaStateSums[FIGHTER_STATES];
typedef SeaUnitStateSum TransEmptyStateSums[TRANS_EMPTY_STATES];
typedef SeaUnitStateSum Trans1IStateSums[TRANS_1I_STATES];
typedef SeaUnitStateSum Trans1AStateSums[TRANS_1A_STATES];
typedef SeaUnitStateSum Trans1TStateSums[TRANS_1T_STATES];
typedef SeaUnitStateSum Trans2IStateSums[TRANS_2I_STATES];
typedef SeaUnitStateSum Trans1I1AStateSums[TRANS_1I_1A_STATES];
typedef SeaUnitStateSum Trans1I1TStateSums[TRANS_1I_1T_STATES];
typedef SeaUnitStateSum SubmarinesStateSums[SUBMARINE_STATES];
typedef SeaUnitStateSum DestroyersStateSums[DESTROYER_STATES];
typedef SeaUnitStateSum CarriersStateSums[CARRIER_STATES];
typedef SeaUnitStateSum CruisersStateSums[CRUISER_STATES];
typedef SeaUnitStateSum BattleshipsStateSums[BATTLESHIP_STATES];
typedef SeaUnitStateSum BSDamagedStateSums[BATTLESHIP_STATES];
typedef SeaUnitStateSum BombersSeaStateSums[BOMBER_SEA_STATES];
typedef AirUnitStateSum FightersAirStateSum[FIGHTER_STATES];
typedef AirUnitStateSum BombersAirStateSum[BOMBER_LAND_STATES];

typedef LandUnitSum LandUnitSumArray[LAND_UNIT_TYPES_COUNT];
typedef SeaUnitSum SeaUnitSumArray[SEA_UNIT_TYPES_COUNT];

typedef LandUnitSumArray LandUnitTypesSumArrayLands[LANDS_COUNT];
typedef SeaUnitSumArray SeaUnitTypesSumArraySeas[SEAS_COUNT];

extern const char* NAMES_UNIT_LAND[LAND_UNIT_TYPES_COUNT];
extern const Tonnes UNIT_WEIGHTS[LAND_UNIT_TYPES_COUNT];
extern const Power ATTACK_UNIT_LAND[LAND_UNIT_TYPES_COUNT];
extern const Power DEFENSE_UNIT_LAND[LAND_UNIT_TYPES_COUNT];
extern const Movement MAX_MOVE_LAND[LAND_UNIT_TYPES_COUNT];
extern const LandUnitStateCount STATES_MOVE_LAND[LAND_UNIT_TYPES_COUNT];
extern const Dollars COST_UNIT_LAND[LAND_UNIT_TYPES_COUNT];
extern const char* NAMES_UNIT_SEA[SEA_UNIT_TYPES_COUNT];
extern const Power ATTACK_UNIT_SEA[SEA_UNIT_TYPES_COUNT];
extern const Power DEFENSE_UNIT_SEA[SEA_UNIT_TYPES_COUNT];
extern const Movement MAX_MOVE_SEA[SEA_UNIT_TYPES_COUNT];
extern const SeaUnitStateCount STATES_MOVE_SEA[SEA_UNIT_TYPES_COUNT];
extern const Dollars COST_UNIT_SEA[SEA_UNIT_TYPES_COUNT];
extern const SeaUnitType BUY_UNIT_SEA[COST_UNIT_SEA_COUNT];
extern const SeaUnitStateCount STATES_STAGING[SEA_UNIT_TYPES_COUNT];
extern const SeaUnitStateCount STATES_UNLOADING[SEA_UNIT_TYPES_COUNT];
extern const SeaUnitType LOAD_UNIT_TYPE[LAND_UNIT_TYPES_COUNT][SEA_UNIT_TYPES_COUNT];
extern const LandUnitType UNLOAD_CARGO1[SEA_UNIT_TYPES_COUNT];
extern const LandUnitType UNLOAD_CARGO2[SEA_UNIT_TYPES_COUNT];
extern const SeaUnitState UNMOVED_SEA[SEA_UNIT_TYPES_COUNT];
extern const SeaUnitState DONE_MOVING_SEA[SEA_UNIT_TYPES_COUNT];
extern const SeaUnitType BLOCKADE_UNIT_TYPES[BLOCKADE_UNIT_TYPES_COUNT];

inline bool is_non_combat_unit(LandUnitType unit_type);
inline bool is_non_loadable_unit(LandUnitType unit_type);
inline bool is_heavy_unit(LandUnitType unit_type);
