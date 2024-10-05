#include "units/units.hpp"
#include "units/artillery.hpp"
#include "units/bomber.hpp"
#include "units/infantry.hpp"
#include "units/tank.hpp"
#include "units/transport.hpp"
#include <sys/types.h>
#include <vector>

const LandUnitNames NAMES_UNIT_LAND = {FIGHTER_NAME,   BOMBER_NAME, INFANTRY_NAME,
                                       ARTILLERY_NAME, TANK_NAME,   AA_GUN_NAME};

const Landunitarray UNIT_WEIGHTS = {MAX_INT,          MAX_INT,     INFANTRY_WEIGHT,
                                    ARTILLERY_WEIGHT, TANK_WEIGHT, AA_GUN_WEIGHT};

const Landunitarray ATTACK_UNIT_LAND = {FIGHTER_ATTACK,   BOMBER_ATTACK, INFANTRY_ATTACK,
                                        ARTILLERY_ATTACK, TANK_ATTACK,   AA_GUN_ATTACK};

const Landunitarray DEFENSE_UNIT_LAND = {FIGHTER_DEFENSE,   BOMBER_DEFENSE, INFANTRY_DEFENSE,
                                         ARTILLERY_DEFENSE, TANK_DEFENSE,   AA_GUN_DEFENSE};

const Landunitarray MAX_MOVE_LAND = {FIGHTER_MOVES_MAX,   BOMBER_MOVES_MAX, INFANTRY_MOVES_MAX,
                                     ARTILLERY_MOVES_MAX, TANK_MOVES_MAX,   AA_GUN_MOVES_MAX};

const Landunitarray STATES_MOVE_LAND = {FIGHTER_STATES,   BOMBER_LAND_STATES, INFANTRY_STATES,
                                        ARTILLERY_STATES, TANK_STATES,        AA_GUN_STATES};

const Landunitarray COST_UNIT_LAND = {FIGHTER_COST,   BOMBER_COST, INFANTRY_COST,
                                      ARTILLERY_COST, TANK_COST,   AA_GUN_COST};

const SeaUnitNames NAMES_UNIT_SEA = {
    FIGHTER_NAME, TRANSPORT_NAME, TRANS1I_NAME,    TRANS1A_NAME,    TRANS1T_NAME,
    TRANS2I_NAME, TRANS1I1A_NAME, TRANS1I1T_NAME,  SUB_NAME,        DESTROYER_NAME,
    CARRIER_NAME, CRUISER_NAME,   BATTLESHIP_NAME, BS_DAMAGED_NAME, BOMBER_NAME};

const Seaunitarray ATTACK_UNIT_SEA = {
    FIGHTER_ATTACK,   TRANSPORT_ATTACK, TRANSPORT_ATTACK,  TRANSPORT_ATTACK,  TRANSPORT_ATTACK,
    TRANSPORT_ATTACK, TRANSPORT_ATTACK, TRANSPORT_ATTACK,  SUB_ATTACK,        DESTROYER_ATTACK,
    CARRIER_ATTACK,   CRUISER_ATTACK,   BATTLESHIP_ATTACK, BATTLESHIP_ATTACK, BOMBER_ATTACK};

const Seaunitarray DEFENSE_UNIT_SEA = {
    FIGHTER_DEFENSE,   TRANSPORT_DEFENSE, TRANSPORT_DEFENSE,  TRANSPORT_DEFENSE,  TRANSPORT_DEFENSE,
    TRANSPORT_DEFENSE, TRANSPORT_DEFENSE, TRANSPORT_DEFENSE,  SUB_DEFENSE,        DESTROYER_DEFENSE,
    CARRIER_DEFENSE,   CRUISER_DEFENSE,   BATTLESHIP_DEFENSE, BATTLESHIP_DEFENSE, BOMBER_DEFENSE};

const Seaunitarray MAX_MOVE_SEA = {FIGHTER_MOVES_MAX,    TRANSPORT_MOVES_MAX,  TRANSPORT_MOVES_MAX,
                                   TRANSPORT_MOVES_MAX,  TRANSPORT_MOVES_MAX,  TRANSPORT_MOVES_MAX,
                                   TRANSPORT_MOVES_MAX,  TRANSPORT_MOVES_MAX,  SUB_MOVES_MAX,
                                   DESTROYER_MOVES_MAX,  CARRIER_MOVES_MAX,    CRUISER_MOVES_MAX,
                                   BATTLESHIP_MOVES_MAX, BATTLESHIP_MOVES_MAX, BOMBER_MOVES_MAX};

const Seaunitarray STATES_MOVE_SEA = {
    FIGHTER_STATES, TRANSEMPTY_STATES, TRANS1I_STATES,    TRANS1A_STATES,    TRANS1T_STATES,
    TRANS2I_STATES, TRANS1I1A_STATES,  TRANS1I1T_STATES,  SUB_STATES,        DESTROYER_STATES,
    CARRIER_STATES, CRUISER_STATES,    BATTLESHIP_STATES, BATTLESHIP_STATES, BOMBER_SEA_STATES};

const Seaunitarray BOMBARD_STATES = {
    FIGHTER_BOMBARD,   TRANSPORT_BOMBARD, TRANSPORT_BOMBARD,  TRANSPORT_BOMBARD,  TRANSPORT_BOMBARD,
    TRANSPORT_BOMBARD, TRANSPORT_BOMBARD, TRANSPORT_BOMBARD,  SUB_BOMBARD,        DESTROYER_BOMBARD,
    CARRIER_BOMBARD,   CRUISER_BOMBARD,   BATTLESHIP_BOMBARD, BATTLESHIP_BOMBARD, BOMBER_BOMBARD};

const Seaunitarray COST_UNIT_SEA = {FIGHTER_COST,
                                    TRANSPORT_COST,
                                    TRANSPORT_COST + INFANTRY_COST,
                                    TRANSPORT_COST + ARTILLERY_COST,
                                    TRANSPORT_COST + TANK_COST,
                                    TRANSPORT_COST + INFANTRY_COST + INFANTRY_COST,
                                    TRANSPORT_COST + INFANTRY_COST + ARTILLERY_COST,
                                    TRANSPORT_COST + INFANTRY_COST + TANK_COST,
                                    SUB_COST,
                                    DESTROYER_COST,
                                    CARRIER_COST,
                                    CRUISER_COST,
                                    BATTLESHIP_COST,
                                    BATTLESHIP_COST,
                                    BOMBER_COST};

const SeaunitVector BUY_UNIT_SEA = {FIGHTERS, TRANSEMPTY, SUBMARINES, DESTROYERS,
                                   CARRIERS, CRUISERS,   BATTLESHIPS};

const Seaunitarray STATES_STAGING = {0,
                             TRANSEMPTY_STAGING_STATES,
                             TRANS1I_STAGING_STATES,
                             TRANS1A_STAGING_STATES,
                             TRANS1T_STAGING_STATES,
                             TRANS2I_STAGING_STATES,
                             TRANS1I1A_STAGING_STATES,
                             TRANS1I1T_STAGING_STATES,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0};

const Seaunitarray STATES_UNLOADING = {0,
                               TRANSEMPTY_UNLOADING_STATES,
                               TRANS1I_UNLOADING_STATES,
                               TRANS1A_UNLOADING_STATES,
                               TRANS1T_UNLOADING_STATES,
                               TRANS2I_UNLOADING_STATES,
                               TRANS1I1A_UNLOADING_STATES,
                               TRANS1I1T_UNLOADING_STATES,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0};

const LandUTSeaUT NEW_TRANS_TYPE_AFTER_LOAD = {
    {// FIGHTER
     {SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT,
      SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT,
      SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT,
      SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT},
     // BOMBER
     {SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT,
      SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT,
      SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT,
      SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT},
     // LOAD_INFANTRY
     {SEA_UNIT_TYPES_COUNT, TRANS1I, TRANS2I, TRANS1I1A, TRANS1I1T, SEA_UNIT_TYPES_COUNT,
      SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT,
      SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT,
      SEA_UNIT_TYPES_COUNT},
     // LOAD_ARTILLERY
     {SEA_UNIT_TYPES_COUNT, TRANS1A, TRANS1I1A, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT,
      SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT,
      SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT,
      SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT},
     // LOAD_TANK
     {SEA_UNIT_TYPES_COUNT, TRANS1T, TRANS1I1T, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT,
      SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT,
      SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT,
      SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT},
     // AA-GUN
     {SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT,
      SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT,
      SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT,
      SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT, SEA_UNIT_TYPES_COUNT}}};

const LandunitCargo UNLOAD_CARGO1 = {LAND_UNIT_TYPES_COUNT,
                                     LAND_UNIT_TYPES_COUNT,
                                     INFANTRY,
                                     ARTILLERY,
                                     TANKS,
                                     INFANTRY,
                                     ARTILLERY,
                                     TANKS,
                                     LAND_UNIT_TYPES_COUNT,
                                     LAND_UNIT_TYPES_COUNT,
                                     LAND_UNIT_TYPES_COUNT,
                                     LAND_UNIT_TYPES_COUNT,
                                     LAND_UNIT_TYPES_COUNT,
                                     LAND_UNIT_TYPES_COUNT,
                                     LAND_UNIT_TYPES_COUNT};

const LandunitCargo UNLOAD_CARGO2 = {LAND_UNIT_TYPES_COUNT,
                                     LAND_UNIT_TYPES_COUNT,
                                     LAND_UNIT_TYPES_COUNT,
                                     LAND_UNIT_TYPES_COUNT,
                                     LAND_UNIT_TYPES_COUNT,
                                     INFANTRY,
                                     INFANTRY,
                                     INFANTRY,
                                     LAND_UNIT_TYPES_COUNT,
                                     LAND_UNIT_TYPES_COUNT,
                                     LAND_UNIT_TYPES_COUNT,
                                     LAND_UNIT_TYPES_COUNT,
                                     LAND_UNIT_TYPES_COUNT,
                                     LAND_UNIT_TYPES_COUNT,
                                     LAND_UNIT_TYPES_COUNT};

const Seaunitarray DONE_MOVING_SEA = {FIGHTER_DONE_MOVING,    TRANSEMPTY_DONE_MOVING, TRANS1I_DONE_MOVING,
                              TRANS1A_DONE_MOVING,    TRANS1T_DONE_MOVING,    TRANS2I_DONE_MOVING,
                              TRANS1I1A_DONE_MOVING,  TRANS1I1T_DONE_MOVING,  SUB_DONE_MOVING,
                              DESTROYER_DONE_MOVING,  CARRIER_DONE_MOVING,    CRUISER_DONE_MOVING,
                              BATTLESHIP_DONE_MOVING, BATTLESHIP_DONE_MOVING, 0};

const SeaunitVector BLOCKADE_UNIT_TYPES = {DESTROYERS, CARRIERS, CRUISERS,
                                                           BATTLESHIPS, BS_DAMAGED};

const Landunittypes ORDER_OF_LAND_DEFENDERS = {AAGUNS, BOMBERS_LAND_AIR, INFANTRY, ARTILLERY,
                                               TANKS,  FIGHTERS_LAND};

const LandunitVector ORDER_OF_LAND_ATTACKERS_1 = {INFANTRY, ARTILLERY, TANKS};
const LandunitVector ORDER_OF_LAND_ATTACKERS_2 = {FIGHTERS, BOMBERS_LAND_AIR};

const SeaunitVector ORDER_OF_SEA_DEFENDERS = {
    SUBMARINES, DESTROYERS, CARRIERS, CRUISERS, FIGHTERS,  BS_DAMAGED, TRANSEMPTY,
    TRANS1I,    TRANS1A,    TRANS1T,  TRANS2I,  TRANS1I1A, TRANS1I1T};
const SeaunitVector ORDER_OF_SEA_ATTACKERS_1 = {SUBMARINES, DESTROYERS};
const SeaunitVector ORDER_OF_SEA_ATTACKERS_2 = {FIGHTERS, BOMBERS_SEA};
const SeaunitVector ORDER_OF_SEA_ATTACKERS_3 = {
    BS_DAMAGED, TRANSEMPTY, TRANS1I, TRANS1A, TRANS1T, TRANS2I, TRANS1I1A, TRANS1I1T};

const Airunitarray MAX_MOVE_AIR = {FIGHTER_MOVES_MAX, BOMBER_MOVES_MAX};
