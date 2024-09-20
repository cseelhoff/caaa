#ifndef UNITS_H
#define UNITS_H

#include "aa_gun.hpp"
#include "artillery.hpp"
#include "battleship.hpp"
#include "bomber.hpp"
#include "carrier.hpp"
#include "cruiser.hpp"
#include "destroyer.hpp"
#include "fighter.hpp"
#include "infantry.hpp"
#include "sub.hpp"
#include "tank.hpp"
#include "transport.hpp"
#include <array>
#include <cstdint>
#include <limits>
#include <sys/types.h>

enum SeaUnitTypesEnum {
  FIGHTERS,
  TRANS_EMPTY,
  TRANS_1I,
  TRANS_1A,
  TRANS_1T,
  TRANS_2I,
  TRANS_1I_1A,
  TRANS_1I_1T,
  SUBMARINES,
  DESTROYERS,
  CARRIERS,
  CRUISERS,
  BATTLESHIPS,
  BS_DAMAGED,
  BOMBERS_SEA,
  SEA_UNIT_TYPES_COUNT
};

enum LandUnitTypeEnum {
  FIGHTERS_LAND,
  BOMBERS_LAND_AIR,
  INFANTRY,
  ARTILLERY,
  TANKS,
  AA_GUNS,
  LAND_UNIT_TYPES_COUNT
};

enum AirUnitTypeEnum {
    FIGHTERS_AIR,
    BOMBERS_AIR,
    AIR_UNIT_TYPES_COUNT
};

constexpr int COST_UNIT_SEA_COUNT = 7;
constexpr int BLOCKADE_UNIT_TYPES_COUNT = 5;
constexpr int DEFENDER_LAND_UNIT_TYPES_COUNT = 6;
constexpr int ATTACKER_LAND_UNIT_TYPES_COUNT_1 = 3;
constexpr int ATTACKER_LAND_UNIT_TYPES_COUNT_2 = 2;
constexpr int DEFENDER_SEA_UNIT_TYPES_COUNT = 13;
constexpr int ATTACKER_SEA_UNIT_TYPES_COUNT_1 = 2;
constexpr int ATTACKER_SEA_UNIT_TYPES_COUNT_2 = 2;
constexpr int ATTACKER_SEA_UNIT_TYPES_COUNT_3 = 8;
#define MAX_INT std::numeric_limits<int>::max()

// Define type aliases for the arrays
using Landunittypes = std::array<uint, LAND_UNIT_TYPES_COUNT>;
using Seanunittypes = std::array<uint, SEA_UNIT_TYPES_COUNT>;
using LandUTSeaUT = std::array<Seanunittypes, LAND_UNIT_TYPES_COUNT>;
using LandUnitNames = std::array<const char*, LAND_UNIT_TYPES_COUNT>;
using SeaUnitNames = std::array<const char*, SEA_UNIT_TYPES_COUNT>;
using BuyableSeaUnits = std::array<uint, COST_UNIT_SEA_COUNT>;
using BlockadeUnitTypes = std::array<uint, BLOCKADE_UNIT_TYPES_COUNT>;
using OrderOfLandAttackers1 = std::array<uint, ATTACKER_LAND_UNIT_TYPES_COUNT_1>;
using OrderOfLandAttackers2 = std::array<uint, ATTACKER_LAND_UNIT_TYPES_COUNT_2>;
using OrderOfSeaDefenders = std::array<uint, DEFENDER_SEA_UNIT_TYPES_COUNT>;
using OrderOfSeaAttackers1 = std::array<uint, ATTACKER_SEA_UNIT_TYPES_COUNT_1>;
using OrderOfSeaAttackers2 = std::array<uint, ATTACKER_SEA_UNIT_TYPES_COUNT_2>;
using OrderOfSeaAttackers3 = std::array<uint, ATTACKER_SEA_UNIT_TYPES_COUNT_3>;

// Declare the arrays as extern
extern const LandUnitNames NAMES_UNIT_LAND;
extern const Landunittypes UNIT_WEIGHTS;
extern const Landunittypes ATTACK_UNIT_LAND;
extern const Landunittypes DEFENSE_UNIT_LAND;
extern const Landunittypes MAX_MOVE_LAND;
extern const Landunittypes STATES_MOVE_LAND;
extern const Landunittypes COST_UNIT_LAND;
extern const SeaUnitNames NAMES_UNIT_SEA;
extern const Seanunittypes ATTACK_UNIT_SEA;
extern const Seanunittypes DEFENSE_UNIT_SEA;
extern const Seanunittypes MAX_MOVE_SEA;
extern const Seanunittypes STATES_MOVE_SEA;
extern const Seanunittypes COST_UNIT_SEA;
extern const BuyableSeaUnits BUY_UNIT_SEA;
extern const Seanunittypes STATES_STAGING;
extern const Seanunittypes STATES_UNLOADING;
extern const LandUTSeaUT LOAD_UNIT_TYPE;
extern const Seanunittypes UNLOAD_CARGO1;
extern const Seanunittypes UNLOAD_CARGO2;
extern const Seanunittypes UNMOVED_SEA;
extern const Seanunittypes DONE_MOVING_SEA;
extern const BlockadeUnitTypes BLOCKADE_UNIT_TYPES;
extern const Landunittypes ORDER_OF_LAND_DEFENDERS;
extern const OrderOfLandAttackers1 ORDER_OF_LAND_ATTACKERS_1;
extern const OrderOfLandAttackers2 ORDER_OF_LAND_ATTACKERS_2;
extern const OrderOfSeaDefenders ORDER_OF_SEA_DEFENDERS;
extern const OrderOfSeaAttackers1 ORDER_OF_SEA_ATTACKERS_1;
extern const OrderOfSeaAttackers2 ORDER_OF_SEA_ATTACKERS_2;
extern const OrderOfSeaAttackers3 ORDER_OF_SEA_ATTACKERS_3;
#endif