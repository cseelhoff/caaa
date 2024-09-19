#include "units/units.h"
#include "units/artillery.h"
#include "units/bomber.h"
#include "units/infantry.h"
#include "units/tank.h"
#include "units/transport.h"


const char* NAMES_UNIT_LAND[LAND_UNIT_TYPES_COUNT] = {FIGHTER_NAME,   BOMBER_NAME, INFANTRY_NAME,
                                                ARTILLERY_NAME, TANK_NAME,   AA_GUN_NAME};

const Landunittypes UNIT_WEIGHTS = {MAX_INT,         MAX_INT,    INFANTRY_WEIGHT,
                                               ARTILLERY_WEIGHT, TANK_WEIGHT, AA_GUN_WEIGHT};

const Landunittypes ATTACK_UNIT_LAND = {FIGHTER_ATTACK,   BOMBER_ATTACK, INFANTRY_ATTACK,
                                                   ARTILLERY_ATTACK, TANK_ATTACK,   AA_GUN_ATTACK};

const Landunittypes DEFENSE_UNIT_LAND = {FIGHTER_DEFENSE,  BOMBER_DEFENSE,
                                                    INFANTRY_DEFENSE, ARTILLERY_DEFENSE,
                                                    TANK_DEFENSE,     AA_GUN_DEFENSE};

const Landunittypes MAX_MOVE_LAND = {FIGHTER_MOVES_MAX,  BOMBER_MOVES_MAX,
                                                INFANTRY_MOVES_MAX, ARTILLERY_MOVES_MAX,
                                                TANK_MOVES_MAX,     AA_GUN_MOVES_MAX};

const Landunittypes STATES_MOVE_LAND = {FIGHTER_STATES,  BOMBER_LAND_STATES,
                                                   INFANTRY_STATES, ARTILLERY_STATES,
                                                   TANK_STATES,     AA_GUN_STATES};

const Landunittypes COST_UNIT_LAND = {FIGHTER_COST,   BOMBER_COST, INFANTRY_COST,
                                                 ARTILLERY_COST, TANK_COST,   AA_GUN_COST};

const char* NAMES_UNIT_SEA[SEA_UNIT_TYPES_COUNT] = {
    FIGHTER_NAME,  TRANSPORT_NAME,   TRANS_1I_NAME,    TRANS_1A_NAME,   TRANS_1T_NAME,
    TRANS_2I_NAME, TRANS_1I_1A_NAME, TRANS_1I_1T_NAME, SUB_NAME,        DESTROYER_NAME,
    CARRIER_NAME,  CRUISER_NAME,     BATTLESHIP_NAME,  BS_DAMAGED_NAME, BOMBER_NAME};

const Seanunittypes ATTACK_UNIT_SEA = {
    FIGHTER_ATTACK,   TRANSPORT_ATTACK, TRANSPORT_ATTACK,  TRANSPORT_ATTACK,  TRANSPORT_ATTACK,
    TRANSPORT_ATTACK, TRANSPORT_ATTACK, TRANSPORT_ATTACK,  SUB_ATTACK,        DESTROYER_ATTACK,
    CARRIER_ATTACK,   CRUISER_ATTACK,   BATTLESHIP_ATTACK, BATTLESHIP_ATTACK, BOMBER_ATTACK};

const Seanunittypes DEFENSE_UNIT_SEA = {
    FIGHTER_DEFENSE,   TRANSPORT_DEFENSE, TRANSPORT_DEFENSE,  TRANSPORT_DEFENSE,  TRANSPORT_DEFENSE,
    TRANSPORT_DEFENSE, TRANSPORT_DEFENSE, TRANSPORT_DEFENSE,  SUB_DEFENSE,        DESTROYER_DEFENSE,
    CARRIER_DEFENSE,   CRUISER_DEFENSE,   BATTLESHIP_DEFENSE, BATTLESHIP_DEFENSE, BOMBER_DEFENSE};

const Seanunittypes MAX_MOVE_SEA = {
    FIGHTER_MOVES_MAX,    TRANSPORT_MOVES_MAX,  TRANSPORT_MOVES_MAX, TRANSPORT_MOVES_MAX,
    TRANSPORT_MOVES_MAX,  TRANSPORT_MOVES_MAX,  TRANSPORT_MOVES_MAX, TRANSPORT_MOVES_MAX,
    SUB_MOVES_MAX,        DESTROYER_MOVES_MAX,  CARRIER_MOVES_MAX,   CRUISER_MOVES_MAX,
    BATTLESHIP_MOVES_MAX, BATTLESHIP_MOVES_MAX, BOMBER_MOVES_MAX};

const Seanunittypes STATES_MOVE_SEA = {
    FIGHTER_STATES,  TRANS_EMPTY_STATES, TRANS_1I_STATES,    TRANS_1A_STATES,   TRANS_1T_STATES,
    TRANS_2I_STATES, TRANS_1I_1A_STATES, TRANS_1I_1T_STATES, SUB_STATES,        DESTROYER_STATES,
    CARRIER_STATES,  CRUISER_STATES,     BATTLESHIP_STATES,  BATTLESHIP_STATES, BOMBER_SEA_STATES};

const Seanunittypes COST_UNIT_SEA = {
    FIGHTER_COST, TRANSPORT_COST,  TRANSPORT_COST + INFANTRY_COST, TRANSPORT_COST + ARTILLERY_COST,
    TRANSPORT_COST + TANK_COST, TRANSPORT_COST + INFANTRY_COST + INFANTRY_COST, TRANSPORT_COST + INFANTRY_COST + ARTILLERY_COST,
    TRANSPORT_COST + INFANTRY_COST + TANK_COST, SUB_COST, DESTROYER_COST, CARRIER_COST,
    CRUISER_COST, BATTLESHIP_COST, BATTLESHIP_COST, BOMBER_COST};

const BuyableSeaUnits BUY_UNIT_SEA = {FIGHTERS, TRANS_EMPTY, SUBMARINES, DESTROYERS,
                                                   CARRIERS, CRUISERS,    BATTLESHIPS};

const Seanunittypes STATES_STAGING = {0,
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

const Seanunittypes STATES_UNLOADING = {0,
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

const LandUTSeaUT LOAD_UNIT_TYPE = {{
    // FIGHTER
    {MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT,
      MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT},
    // BOMBER
    {MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT,
      MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT},
    // LOAD_INFANTRY
    {MAX_INT, TRANS_1I, TRANS_2I, TRANS_1I_1A, TRANS_1I_1T, MAX_INT, MAX_INT, MAX_INT, MAX_INT,
      MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT},
    // LOAD_ARTILLERY
    {MAX_INT, TRANS_1A, TRANS_1I_1A, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT,
      MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT},
    // LOAD_TANK
    {MAX_INT, TRANS_1T, TRANS_1I_1T, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT,
      MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT},
    // AA-GUN
    {MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT,
      MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT}
}};

const Seanunittypes UNLOAD_CARGO1 = {MAX_INT, MAX_INT,  INFANTRY, ARTILLERY, TANKS,
                                               INFANTRY, ARTILLERY, TANKS,    MAX_INT,  MAX_INT,
                                               MAX_INT, MAX_INT,  MAX_INT, MAX_INT,  MAX_INT};

const Seanunittypes UNLOAD_CARGO2 = {MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT,
                                               INFANTRY, INFANTRY, INFANTRY, MAX_INT, MAX_INT,
                                               MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT};

const Seanunittypes UNMOVED_SEA = {
    FIGHTER_UNMOVED,     TRANS_EMPTY_UNMOVED, TRANS_1I_UNMOVED,
    TRANS_1A_UNMOVED,    TRANS_1T_UNMOVED,    TRANS_2I_UNMOVED,
    TRANS_1I_1A_UNMOVED, TRANS_1I_1T_UNMOVED, SUB_UNMOVED,
    DESTROYER_UNMOVED,   CARRIER_UNMOVED,     CRUISER_UNMOVED,
    BATTLESHIP_UNMOVED,  BATTLESHIP_UNMOVED,  MAX_INT};

const Seanunittypes DONE_MOVING_SEA = {
    FIGHTER_DONE_MOVING,     TRANS_EMPTY_DONE_MOVING, TRANS_1I_DONE_MOVING,
    TRANS_1A_DONE_MOVING,    TRANS_1T_DONE_MOVING,    TRANS_2I_DONE_MOVING,
    TRANS_1I_1A_DONE_MOVING, TRANS_1I_1T_DONE_MOVING, SUB_DONE_MOVING,
    DESTROYER_DONE_MOVING,   CARRIER_DONE_MOVING,     CRUISER_DONE_MOVING,
    BATTLESHIP_DONE_MOVING,  BATTLESHIP_DONE_MOVING,  0};

const BlockadeUnitTypes BLOCKADE_UNIT_TYPES = {DESTROYERS, CARRIERS, CRUISERS,
                                                          BATTLESHIPS, BS_DAMAGED};

const Landunittypes ORDER_OF_LAND_DEFENDERS = {
    AA_GUNS, BOMBERS_LAND_AIR, INFANTRY, ARTILLERY, TANKS, FIGHTERS};

const OrderOfLandAttackers1 ORDER_OF_LAND_ATTACKERS_1 = {INFANTRY, ARTILLERY, TANKS};
const OrderOfLandAttackers2 ORDER_OF_LAND_ATTACKERS_2 = {FIGHTERS, BOMBERS_LAND_AIR};

const OrderOfSeaDefenders ORDER_OF_SEA_DEFENDERS = {
    SUBMARINES, DESTROYERS, CARRIERS, CRUISERS, FIGHTERS,    BS_DAMAGED, TRANS_EMPTY,
    TRANS_1I,   TRANS_1A,   TRANS_1T, TRANS_2I, TRANS_1I_1A, TRANS_1I_1T};
const OrderOfSeaAttackers1 ORDER_OF_SEA_ATTACKERS_1 = {SUBMARINES, DESTROYERS};
const OrderOfSeaAttackers2 ORDER_OF_SEA_ATTACKERS_2 = {FIGHTERS, BOMBERS_SEA};
const OrderOfSeaAttackers3 ORDER_OF_SEA_ATTACKERS_3 = {
    BS_DAMAGED, TRANS_EMPTY, TRANS_1I, TRANS_1A, TRANS_1T, TRANS_2I, TRANS_1I_1A, TRANS_1I_1T};
    