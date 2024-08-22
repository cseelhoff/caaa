#include "units/units.h"

const char* NAMES_UNIT_LAND[LAND_UNIT_TYPES] = {FIGHTER_NAME,   BOMBER_NAME, INFANTRY_NAME,
                                                ARTILLERY_NAME, TANK_NAME,   AA_GUN_NAME};

const uint8_t UNIT_WEIGHTS[LAND_UNIT_TYPES] = {MAX_UINT8_T,         MAX_UINT8_T,    INFANTRY_WEIGHT,
                                               ARTILLERY_WEIGHT, TANK_WEIGHT, AA_GUN_WEIGHT};

const uint8_t ATTACK_UNIT_LAND[LAND_UNIT_TYPES] = {FIGHTER_ATTACK,   BOMBER_ATTACK, INFANTRY_ATTACK,
                                                   ARTILLERY_ATTACK, TANK_ATTACK,   AA_GUN_ATTACK};

const uint8_t DEFENSE_UNIT_LAND[LAND_UNIT_TYPES] = {FIGHTER_DEFENSE,  BOMBER_DEFENSE,
                                                    INFANTRY_DEFENSE, ARTILLERY_DEFENSE,
                                                    TANK_DEFENSE,     AA_GUN_DEFENSE};

const uint8_t MAX_MOVE_LAND[LAND_UNIT_TYPES] = {FIGHTER_MOVES_MAX,  BOMBER_MOVES_MAX,
                                                INFANTRY_MOVES_MAX, ARTILLERY_MOVES_MAX,
                                                TANK_MOVES_MAX,     AA_GUN_MOVES_MAX};

const uint8_t STATES_MOVE_LAND[LAND_UNIT_TYPES] = {FIGHTER_STATES,  BOMBER_LAND_STATES,
                                                   INFANTRY_STATES, ARTILLERY_STATES,
                                                   TANK_STATES,     AA_GUN_STATES};

const uint8_t COST_UNIT_LAND[LAND_UNIT_TYPES] = {FIGHTER_COST,   BOMBER_COST, INFANTRY_COST,
                                                 ARTILLERY_COST, TANK_COST,   AA_GUN_COST};

const char* NAMES_UNIT_SEA[SEA_UNIT_TYPES] = {
    FIGHTER_NAME,  TRANSPORT_NAME,   TRANS_1I_NAME,    TRANS_1A_NAME,   TRANS_1T_NAME,
    TRANS_2I_NAME, TRANS_1I_1A_NAME, TRANS_1I_1T_NAME, SUB_NAME,        DESTROYER_NAME,
    CARRIER_NAME,  CRUISER_NAME,     BATTLESHIP_NAME,  BS_DAMAGED_NAME, BOMBER_NAME};

const uint8_t ATTACK_UNIT_SEA[SEA_UNIT_TYPES] = {
    FIGHTER_ATTACK,   TRANSPORT_ATTACK, TRANSPORT_ATTACK,  TRANSPORT_ATTACK,  TRANSPORT_ATTACK,
    TRANSPORT_ATTACK, TRANSPORT_ATTACK, TRANSPORT_ATTACK,  SUB_ATTACK,        DESTROYER_ATTACK,
    CARRIER_ATTACK,   CRUISER_ATTACK,   BATTLESHIP_ATTACK, BATTLESHIP_ATTACK, BOMBER_ATTACK};

const uint8_t DEFENSE_UNIT_SEA[SEA_UNIT_TYPES] = {
    FIGHTER_DEFENSE,   TRANSPORT_DEFENSE, TRANSPORT_DEFENSE,  TRANSPORT_DEFENSE,  TRANSPORT_DEFENSE,
    TRANSPORT_DEFENSE, TRANSPORT_DEFENSE, TRANSPORT_DEFENSE,  SUB_DEFENSE,        DESTROYER_DEFENSE,
    CARRIER_DEFENSE,   CRUISER_DEFENSE,   BATTLESHIP_DEFENSE, BATTLESHIP_DEFENSE, BOMBER_DEFENSE};

const uint8_t MAX_MOVE_SEA[SEA_UNIT_TYPES] = {
    FIGHTER_MOVES_MAX,    TRANSPORT_MOVES_MAX,  TRANSPORT_MOVES_MAX, TRANSPORT_MOVES_MAX,
    TRANSPORT_MOVES_MAX,  TRANSPORT_MOVES_MAX,  TRANSPORT_MOVES_MAX, TRANSPORT_MOVES_MAX,
    SUB_MOVES_MAX,        DESTROYER_MOVES_MAX,  CARRIER_MOVES_MAX,   CRUISER_MOVES_MAX,
    BATTLESHIP_MOVES_MAX, BATTLESHIP_MOVES_MAX, BOMBER_MOVES_MAX};

const uint8_t STATES_MOVE_SEA[SEA_UNIT_TYPES] = {
    FIGHTER_STATES,  TRANS_EMPTY_STATES, TRANS_1I_STATES,    TRANS_1A_STATES,   TRANS_1T_STATES,
    TRANS_2I_STATES, TRANS_1I_1A_STATES, TRANS_1I_1T_STATES, SUB_STATES,        DESTROYER_STATES,
    CARRIER_STATES,  CRUISER_STATES,     BATTLESHIP_STATES,  BATTLESHIP_STATES, BOMBER_SEA_STATES};

const uint8_t COST_UNIT_SEA[SEA_UNIT_TYPES] = {
    FIGHTER_COST, TRANSPORT_COST,  0, 0, 0, 0, 0, 0, SUB_COST, DESTROYER_COST, CARRIER_COST,
    CRUISER_COST, BATTLESHIP_COST, 0, 0};

const uint8_t BUY_UNIT_SEA[COST_UNIT_SEA_COUNT] = {FIGHTERS, TRANS_EMPTY, SUBMARINES, DESTROYERS,
                                                   CARRIERS, CRUISERS,    BATTLESHIPS};

const uint8_t STATES_STAGING[SEA_UNIT_TYPES] = {0,
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

const uint8_t STATES_UNLOADING[SEA_UNIT_TYPES] = {0,
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

const uint8_t LOAD_UNIT_TYPE[LAND_UNIT_TYPES][SEA_UNIT_TYPES] = {
    // FIGHTER
    {MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T,
     MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T},
    // BOMBER
    {MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T,
     MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T},
    // LOAD_INFANTRY
    {MAX_UINT8_T, TRANS_1I, TRANS_2I, TRANS_1I_1A, TRANS_1I_1T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T,
     MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T},
    // LOAD_ARTILLERY
    {MAX_UINT8_T, TRANS_1A, TRANS_1I_1A, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T,
     MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T},
    // LOAD_TANK
    {MAX_UINT8_T, TRANS_1T, TRANS_1I_1T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T,
     MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T},
    // AA-GUN
    {MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T,
     MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T}};

const uint8_t UNLOAD_CARGO1[SEA_UNIT_TYPES] = {MAX_UINT8_T, MAX_UINT8_T,  INFANTRY, ARTILLERY, TANKS,
                                               INFANTRY, ARTILLERY, TANKS,    MAX_UINT8_T,  MAX_UINT8_T,
                                               MAX_UINT8_T, MAX_UINT8_T,  MAX_UINT8_T, MAX_UINT8_T,  MAX_UINT8_T};

const uint8_t UNLOAD_CARGO2[SEA_UNIT_TYPES] = {MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T,
                                               INFANTRY, INFANTRY, INFANTRY, MAX_UINT8_T, MAX_UINT8_T,
                                               MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T, MAX_UINT8_T};

const uint8_t UNMOVED_SEA[SEA_UNIT_TYPES] = {
    FIGHTER_UNMOVED,     TRANS_EMPTY_UNMOVED, TRANS_1I_UNMOVED,
    TRANS_1A_UNMOVED,    TRANS_1T_UNMOVED,    TRANS_2I_UNMOVED,
    TRANS_1I_1A_UNMOVED, TRANS_1I_1T_UNMOVED, SUB_UNMOVED,
    DESTROYER_UNMOVED,   CARRIER_UNMOVED,     CRUISER_UNMOVED,
    BATTLESHIP_UNMOVED,  BATTLESHIP_UNMOVED,  MAX_UINT8_T};

const uint8_t DONE_MOVING_SEA[SEA_UNIT_TYPES] = {
    FIGHTER_DONE_MOVING,     TRANS_EMPTY_DONE_MOVING, TRANS_1I_DONE_MOVING,
    TRANS_1A_DONE_MOVING,    TRANS_1T_DONE_MOVING,    TRANS_2I_DONE_MOVING,
    TRANS_1I_1A_DONE_MOVING, TRANS_1I_1T_DONE_MOVING, SUB_DONE_MOVING,
    DESTROYER_DONE_MOVING,   CARRIER_DONE_MOVING,     CRUISER_DONE_MOVING,
    BATTLESHIP_DONE_MOVING,  BATTLESHIP_DONE_MOVING,  0};
