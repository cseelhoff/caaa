#pragma once
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
#include <string_view>
#include <sys/types.h>
#include <vector>

enum SeaUnitTypesEnum : uint {
  FIGHTERS,
  TRANSEMPTY,
  TRANS1I,
  TRANS1A,
  TRANS1T,
  TRANS2I,
  TRANS1I1A,
  TRANS1I1T,
  SUBMARINES,
  DESTROYERS,
  CARRIERS,
  CRUISERS,
  BATTLESHIPS,
  BS_DAMAGED,
  BOMBERS_SEA,
  SEA_UNIT_TYPES_COUNT
};

enum LandUnitTypeEnum : uint {
  FIGHTERS_LAND,
  BOMBERS_LAND_AIR,
  INFANTRY,
  ARTILLERY,
  TANKS,
  AAGUNS,
  LAND_UNIT_TYPES_COUNT
};

enum AirUnitTypeEnum : uint {
    FIGHTERS_AIR,
    BOMBERS_AIR,
    AIR_UNIT_TYPES_COUNT
};

constexpr uint MAX_INT = std::numeric_limits<int>::max();

// Define type aliases for the arrays
using Landunittypes = std::array<LandUnitTypeEnum, LAND_UNIT_TYPES_COUNT>;
using LandunitCargo = std::array<LandUnitTypeEnum, SEA_UNIT_TYPES_COUNT>;
using Landunitarray = std::array<uint, LAND_UNIT_TYPES_COUNT>;
using Seaunittypes = std::array<SeaUnitTypesEnum, SEA_UNIT_TYPES_COUNT>;
using Seaunitarray = std::array<uint, SEA_UNIT_TYPES_COUNT>;
using Airunitarray = std::array<uint, AIR_UNIT_TYPES_COUNT>;
using LandUTSeaUT = std::array<Seaunittypes, LAND_UNIT_TYPES_COUNT>;
using LandUnitNames = std::array<std::string_view, LAND_UNIT_TYPES_COUNT>;
using SeaUnitNames = std::array<std::string_view, SEA_UNIT_TYPES_COUNT>;
using LandunitVector = std::vector<LandUnitTypeEnum>;
using SeaunitVector = std::vector<SeaUnitTypesEnum>;

// Declare the arrays as extern
extern const LandUnitNames NAMES_UNIT_LAND;
extern const Landunitarray UNIT_WEIGHTS;
extern const Landunitarray ATTACK_UNIT_LAND;
extern const Landunitarray DEFENSE_UNIT_LAND;
extern const Landunitarray MAX_MOVE_LAND;
extern const Landunitarray STATES_MOVE_LAND;
extern const Landunitarray COST_UNIT_LAND;
extern const SeaUnitNames NAMES_UNIT_SEA;
extern const Seaunitarray ATTACK_UNIT_SEA;
extern const Seaunitarray DEFENSE_UNIT_SEA;
extern const Seaunitarray MAX_MOVE_SEA;
extern const Seaunitarray STATES_MOVE_SEA;
extern const Seaunitarray COST_UNIT_SEA;
extern const SeaunitVector BUY_UNIT_SEA;
extern const Seaunitarray STATES_STAGING;
extern const Seaunitarray STATES_UNLOADING;
extern const LandUTSeaUT NEW_TRANS_TYPE_AFTER_LOAD;
extern const LandunitCargo UNLOAD_CARGO1;
extern const LandunitCargo UNLOAD_CARGO2;
extern const Seaunitarray DONE_MOVING_SEA;
extern const SeaunitVector BLOCKADE_UNIT_TYPES;
extern const Landunittypes ORDER_OF_LAND_DEFENDERS;
extern const LandunitVector ORDER_OF_LAND_ATTACKERS_1;
extern const LandunitVector ORDER_OF_LAND_ATTACKERS_2;
extern const SeaunitVector ORDER_OF_SEA_DEFENDERS;
extern const SeaunitVector ORDER_OF_SEA_ATTACKERS_1;
extern const SeaunitVector ORDER_OF_SEA_ATTACKERS_2;
extern const SeaunitVector ORDER_OF_SEA_ATTACKERS_3;
extern const Airunitarray MAX_MOVE_AIR;
extern const Seaunitarray BOMBARD_STATES;
