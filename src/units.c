#include "units/units.h"
#include "typedefs.h"
#include <stdint.h>

const SeaUnitType ORDER_OF_SEA_DEFENDERS[DEFENDER_SEA_UNIT_TYPES_COUNT] = {
    SUBMARINES, DESTROYERS, CARRIERS, CRUISERS, FIGHTERS_SEA, BS_DAMAGED, TRANS_EMPTY,
    TRANS_1I,   TRANS_1A,   TRANS_1T, TRANS_2I, TRANS_1I_1A,  TRANS_1I_1T};

const SeaUnitType ORDER_OF_SEA_ATTACKERS_1[ATTACKER_SEA_UNIT_TYPES_COUNT_1] = {SUBMARINES,
                                                                               DESTROYERS};

const SeaUnitType ORDER_OF_SEA_ATTACKERS_2[ATTACKER_SEA_UNIT_TYPES_COUNT_2] = {FIGHTERS_SEA,
                                                                               BOMBERS_SEA};

const SeaUnitType ORDER_OF_SEA_ATTACKERS_3[ATTACKER_SEA_UNIT_TYPES_COUNT_3] = {
    BS_DAMAGED, TRANS_EMPTY, TRANS_1I, TRANS_1A, TRANS_1T, TRANS_2I, TRANS_1I_1A, TRANS_1I_1T};

const LandUnitType ORDER_OF_LAND_DEFENDERS[DEFENDER_LAND_UNIT_TYPES_COUNT] = {
    AA_GUNS, BOMBERS_LAND, INFANTRY, ARTILLERY, TANKS, FIGHTERS_LAND};

const LandUnitType ORDER_OF_LAND_ATTACKERS_1[ATTACKER_LAND_UNIT_TYPES_COUNT_1] = {INFANTRY,
                                                                                  ARTILLERY, TANKS};
const LandUnitType ORDER_OF_LAND_ATTACKERS_2[ATTACKER_LAND_UNIT_TYPES_COUNT_2] = {FIGHTERS_LAND,
                                                                                  BOMBERS_LAND};

const char* NAMES_UNIT_LAND[LAND_UNIT_TYPES_COUNT] = {FIGHTER_NAME,   BOMBER_NAME, INFANTRY_NAME,
                                                      ARTILLERY_NAME, TANK_NAME,   AA_GUN_NAME};

const Tonnes UNIT_WEIGHTS[LAND_UNIT_TYPES_COUNT] = {MAX_UINT8_T,      MAX_UINT8_T, INFANTRY_WEIGHT,
                                                    ARTILLERY_WEIGHT, TANK_WEIGHT, AA_GUN_WEIGHT};

const Power ATTACK_UNIT_LAND[LAND_UNIT_TYPES_COUNT] = {
    FIGHTER_ATTACK, BOMBER_ATTACK, INFANTRY_ATTACK, ARTILLERY_ATTACK, TANK_ATTACK, AA_GUN_ATTACK};

const Power DEFENSE_UNIT_LAND[LAND_UNIT_TYPES_COUNT] = {FIGHTER_DEFENSE,  BOMBER_DEFENSE,
                                                        INFANTRY_DEFENSE, ARTILLERY_DEFENSE,
                                                        TANK_DEFENSE,     AA_GUN_DEFENSE};

const Movement MAX_MOVE_LAND[LAND_UNIT_TYPES_COUNT] = {FIGHTER_MOVES_MAX,  BOMBER_MOVES_MAX,
                                                       INFANTRY_MOVES_MAX, ARTILLERY_MOVES_MAX,
                                                       TANK_MOVES_MAX,     AA_GUN_MOVES_MAX};

const LandUnitStateCount STATES_MOVE_LAND[LAND_UNIT_TYPES_COUNT] = {
    FIGHTER_STATES,   BOMBER_LAND_STATES, INFANTRY_STATES,
    ARTILLERY_STATES, TANK_STATES,        AA_GUN_STATES};

const Dollars COST_UNIT_LAND[LAND_UNIT_TYPES_COUNT] = {FIGHTER_COST,   BOMBER_COST, INFANTRY_COST,
                                                       ARTILLERY_COST, TANK_COST,   AA_GUN_COST};

const char* NAMES_UNIT_SEA[SEA_UNIT_TYPES_COUNT] = {
    FIGHTER_NAME,  TRANSPORT_NAME,   TRANS_1I_NAME,    TRANS_1A_NAME,   TRANS_1T_NAME,
    TRANS_2I_NAME, TRANS_1I_1A_NAME, TRANS_1I_1T_NAME, SUB_NAME,        DESTROYER_NAME,
    CARRIER_NAME,  CRUISER_NAME,     BATTLESHIP_NAME,  BS_DAMAGED_NAME, BOMBER_NAME};

const Power ATTACK_UNIT_SEA[SEA_UNIT_TYPES_COUNT] = {
    FIGHTER_ATTACK,   TRANSPORT_ATTACK, TRANSPORT_ATTACK,  TRANSPORT_ATTACK,  TRANSPORT_ATTACK,
    TRANSPORT_ATTACK, TRANSPORT_ATTACK, TRANSPORT_ATTACK,  SUB_ATTACK,        DESTROYER_ATTACK,
    CARRIER_ATTACK,   CRUISER_ATTACK,   BATTLESHIP_ATTACK, BATTLESHIP_ATTACK, BOMBER_ATTACK};

const Power DEFENSE_UNIT_SEA[SEA_UNIT_TYPES_COUNT] = {
    FIGHTER_DEFENSE,   TRANSPORT_DEFENSE, TRANSPORT_DEFENSE,  TRANSPORT_DEFENSE,  TRANSPORT_DEFENSE,
    TRANSPORT_DEFENSE, TRANSPORT_DEFENSE, TRANSPORT_DEFENSE,  SUB_DEFENSE,        DESTROYER_DEFENSE,
    CARRIER_DEFENSE,   CRUISER_DEFENSE,   BATTLESHIP_DEFENSE, BATTLESHIP_DEFENSE, BOMBER_DEFENSE};

const Movement MAX_MOVE_SEA[SEA_UNIT_TYPES_COUNT] = {
    FIGHTER_MOVES_MAX,    TRANSPORT_MOVES_MAX,  TRANSPORT_MOVES_MAX, TRANSPORT_MOVES_MAX,
    TRANSPORT_MOVES_MAX,  TRANSPORT_MOVES_MAX,  TRANSPORT_MOVES_MAX, TRANSPORT_MOVES_MAX,
    SUB_MOVES_MAX,        DESTROYER_MOVES_MAX,  CARRIER_MOVES_MAX,   CRUISER_MOVES_MAX,
    BATTLESHIP_MOVES_MAX, BATTLESHIP_MOVES_MAX, BOMBER_MOVES_MAX};

const SeaUnitStateCount STATES_MOVE_SEA[SEA_UNIT_TYPES_COUNT] = {
    FIGHTER_STATES,  TRANS_EMPTY_STATES, TRANS_1I_STATES,    TRANS_1A_STATES,   TRANS_1T_STATES,
    TRANS_2I_STATES, TRANS_1I_1A_STATES, TRANS_1I_1T_STATES, SUB_STATES,        DESTROYER_STATES,
    CARRIER_STATES,  CRUISER_STATES,     BATTLESHIP_STATES,  BATTLESHIP_STATES, BOMBER_SEA_STATES};

const Dollars COST_UNIT_SEA[SEA_UNIT_TYPES_COUNT] = {FIGHTER_COST,
                                                     TRANSPORT_COST,
                                                     TRANSPORT_COST + INFANTRY_COST,
                                                     TRANSPORT_COST + ARTILLERY_COST,
                                                     TRANSPORT_COST + TANK_COST,
                                                     TRANSPORT_COST + INFANTRY_COST + INFANTRY_COST,
                                                     TRANSPORT_COST + INFANTRY_COST +
                                                         ARTILLERY_COST,
                                                     TRANSPORT_COST + INFANTRY_COST + TANK_COST,
                                                     SUB_COST,
                                                     DESTROYER_COST,
                                                     CARRIER_COST,
                                                     CRUISER_COST,
                                                     BATTLESHIP_COST,
                                                     BATTLESHIP_COST,
                                                     BOMBER_COST};

const SeaUnitType BUY_UNIT_SEA[COST_UNIT_SEA_COUNT] = {
    FIGHTERS_SEA, TRANS_EMPTY, SUBMARINES, DESTROYERS, CARRIERS, CRUISERS, BATTLESHIPS};

const SeaUnitStateCount STATES_STAGING[SEA_UNIT_TYPES_COUNT] = {0,
                                                                TRANS_EMPTY_STAGING_STATES,
                                                                TRANS_1I_STAGING_STATES,
                                                                TRANS_1A_STAGING_STATES,
                                                                TRANS_1T_STAGING_STATES,
                                                                TRANS_2I_STAGING_STATES,
                                                                TRANS_1I_1A_STAGING_STATES,
                                                                TRANS_1I_1T_STAGING_STATES,
                                                                0,
                                                                0,
                                                                0,
                                                                0,
                                                                0,
                                                                0,
                                                                0};

const SeaUnitStateCount STATES_UNLOADING[SEA_UNIT_TYPES_COUNT] = {0,
                                                                  TRANS_EMPTY_UNLOADING_STATES,
                                                                  TRANS_1I_UNLOADING_STATES,
                                                                  TRANS_1A_UNLOADING_STATES,
                                                                  TRANS_1T_UNLOADING_STATES,
                                                                  TRANS_2I_UNLOADING_STATES,
                                                                  TRANS_1I_1A_UNLOADING_STATES,
                                                                  TRANS_1I_1T_UNLOADING_STATES,
                                                                  0,
                                                                  0,
                                                                  0,
                                                                  0,
                                                                  0,
                                                                  0,
                                                                  0};

const SeaUnitType LOAD_UNIT_TYPE[LAND_UNIT_TYPES_COUNT][SEA_UNIT_TYPES_COUNT] = {
    // FIGHTER
    {MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T,
     MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T,
     MAX_UINT8_T},
    // BOMBER
    {MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T,
     MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T,
     MAX_UINT8_T},
    // LOAD_INFANTRY
    {MAX_UINT8_T, TRANS_1I, TRANS_2I, TRANS_1I_1A, TRANS_1I_1T, MAX_UINT8_T, MAX_UINT8_T,
     MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T,
     MAX_UINT8_T},
    // LOAD_ARTILLERY
    {MAX_UINT8_T, TRANS_1A, TRANS_1I_1A, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T,
     MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T,
     MAX_UINT8_T},
    // LOAD_TANK
    {MAX_UINT8_T, TRANS_1T, TRANS_1I_1T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T,
     MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T,
     MAX_UINT8_T},
    // AA-GUN
    {MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T,
     MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T,
     MAX_UINT8_T}};

const LandUnitType UNLOAD_CARGO1[SEA_UNIT_TYPES_COUNT] = {
    MAX_UINT8_T, MAX_UINT8_T, INFANTRY,    ARTILLERY,   TANKS,       INFANTRY,    ARTILLERY,  TANKS,
    MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T};

const LandUnitType UNLOAD_CARGO2[SEA_UNIT_TYPES_COUNT] = {
    MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T,
    INFANTRY,    INFANTRY,    INFANTRY,    MAX_UINT8_T, MAX_UINT8_T,
    MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T};

const SeaUnitState UNMOVED_SEA[SEA_UNIT_TYPES_COUNT] = {
    FIGHTER_UNMOVED,    TRANS_EMPTY_UNMOVED, TRANS_1I_UNMOVED,    TRANS_1A_UNMOVED,
    TRANS_1T_UNMOVED,   TRANS_2I_UNMOVED,    TRANS_1I_1A_UNMOVED, TRANS_1I_1T_UNMOVED,
    SUB_UNMOVED,        DESTROYER_UNMOVED,   CARRIER_UNMOVED,     CRUISER_UNMOVED,
    BATTLESHIP_UNMOVED, BATTLESHIP_UNMOVED,  MAX_UINT8_T};

const SeaUnitState DONE_MOVING_SEA[SEA_UNIT_TYPES_COUNT] = {
    FIGHTER_DONE_MOVING,     TRANS_EMPTY_DONE_MOVING, TRANS_1I_DONE_MOVING,
    TRANS_1A_DONE_MOVING,    TRANS_1T_DONE_MOVING,    TRANS_2I_DONE_MOVING,
    TRANS_1I_1A_DONE_MOVING, TRANS_1I_1T_DONE_MOVING, SUB_DONE_MOVING,
    DESTROYER_DONE_MOVING,   CARRIER_DONE_MOVING,     CRUISER_DONE_MOVING,
    BATTLESHIP_DONE_MOVING,  BATTLESHIP_DONE_MOVING,  0};

const SeaUnitType BLOCKADE_UNIT_TYPES[BLOCKADE_UNIT_TYPES_COUNT] = {DESTROYERS, CARRIERS, CRUISERS,
                                                                    BATTLESHIPS, BS_DAMAGED};
