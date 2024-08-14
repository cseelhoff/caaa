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

#define SEA_UNIT_TYPES 15
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
#define CRUISERS 9
#define CARRIERS 11
#define BATTLESHIPS 12
#define BS_DAMAGED 13
#define BOMBERS_SEA 14

#define LAND_UNIT_TYPES 6
#define BOMBERS_LAND_AIR 1
#define INFANTRY 2
#define ARTILLERY 3
#define TANKS 4
#define AA_GUNS 5

#define AIR_UNIT_TYPES 2

extern const char* NAMES_UNIT_LAND[LAND_UNIT_TYPES];
const char* NAMES_UNIT_LAND[LAND_UNIT_TYPES] = {FIGHTER_NAME,  BOMBER_NAME,
                                                INFANTRY_NAME, ARTILLERY_NAME,
                                                TANK_NAME,     AA_GUN_NAME};

extern const uint8_t ATTACK_UNIT_LAND[LAND_UNIT_TYPES];
const uint8_t ATTACK_UNIT_LAND[LAND_UNIT_TYPES] = {
    FIGHTER_ATTACK,   BOMBER_ATTACK, INFANTRY_ATTACK,
    ARTILLERY_ATTACK, TANK_ATTACK,   AA_GUN_ATTACK};

extern const uint8_t DEFENSE_UNIT_LAND[LAND_UNIT_TYPES];
const uint8_t DEFENSE_UNIT_LAND[LAND_UNIT_TYPES] = {
    FIGHTER_DEFENSE,   BOMBER_DEFENSE, INFANTRY_DEFENSE,
    ARTILLERY_DEFENSE, TANK_DEFENSE,   AA_GUN_DEFENSE};

extern const uint8_t MAX_MOVE_LAND[LAND_UNIT_TYPES];
const uint8_t MAX_MOVE_LAND[LAND_UNIT_TYPES] = {
    FIGHTER_MOVES_MAX,   BOMBER_MOVES_MAX, INFANTRY_MOVES_MAX,
    ARTILLERY_MOVES_MAX, TANK_MOVES_MAX,   AA_GUN_MOVES_MAX};

extern const uint8_t STATES_MOVE_LAND[LAND_UNIT_TYPES];
const uint8_t STATES_MOVE_LAND[LAND_UNIT_TYPES] = {
    FIGHTER_STATES,   BOMBER_LAND_STATES, INFANTRY_STATES,
    ARTILLERY_STATES, TANK_STATES,        AA_GUN_STATES};

extern const uint8_t COST_UNIT_LAND[LAND_UNIT_TYPES];
const uint8_t COST_UNIT_LAND[LAND_UNIT_TYPES] = {FIGHTER_COST,  BOMBER_COST,
                                                 INFANTRY_COST, ARTILLERY_COST,
                                                 TANK_COST,     AA_GUN_COST};

extern const char* NAMES_UNIT_SEA[SEA_UNIT_TYPES];
const char* NAMES_UNIT_SEA[SEA_UNIT_TYPES] = {
    FIGHTER_NAME,    TRANSPORT_NAME,  TRANS_1I_NAME,    TRANS_1A_NAME,
    TRANS_1T_NAME,   TRANS_2I_NAME,   TRANS_1I_1A_NAME, TRANS_1I_1T_NAME,
    SUB_NAME,        DESTROYER_NAME,  CRUISER_NAME,     CARRIER_NAME,
    BATTLESHIP_NAME, BS_DAMAGED_NAME, BOMBER_NAME};

extern const uint8_t ATTACK_UNIT_SEA[SEA_UNIT_TYPES];
const uint8_t ATTACK_UNIT_SEA[SEA_UNIT_TYPES] = {
    FIGHTER_ATTACK,    TRANSPORT_ATTACK,  TRANSPORT_ATTACK, TRANSPORT_ATTACK,
    TRANSPORT_ATTACK,  TRANSPORT_ATTACK,  TRANSPORT_ATTACK, TRANSPORT_ATTACK,
    SUB_ATTACK,        DESTROYER_ATTACK,  CRUISER_ATTACK,   CARRIER_ATTACK,
    BATTLESHIP_ATTACK, BATTLESHIP_ATTACK, BOMBER_ATTACK};

extern const uint8_t DEFENSE_UNIT_SEA[SEA_UNIT_TYPES];
const uint8_t DEFENSE_UNIT_SEA[SEA_UNIT_TYPES] = {
    FIGHTER_DEFENSE,    TRANSPORT_DEFENSE,  TRANSPORT_DEFENSE,
    TRANSPORT_DEFENSE,  TRANSPORT_DEFENSE,  TRANSPORT_DEFENSE,
    TRANSPORT_DEFENSE,  TRANSPORT_DEFENSE,  SUB_DEFENSE,
    DESTROYER_DEFENSE,  CRUISER_DEFENSE,    CARRIER_DEFENSE,
    BATTLESHIP_DEFENSE, BATTLESHIP_DEFENSE, BOMBER_DEFENSE};

extern const uint8_t MAX_MOVE_SEA[SEA_UNIT_TYPES];
const uint8_t MAX_MOVE_SEA[SEA_UNIT_TYPES] = {
    FIGHTER_MOVES_MAX,    TRANSPORT_MOVES_MAX,  TRANSPORT_MOVES_MAX,
    TRANSPORT_MOVES_MAX,  TRANSPORT_MOVES_MAX,  TRANSPORT_MOVES_MAX,
    TRANSPORT_MOVES_MAX,  TRANSPORT_MOVES_MAX,  SUB_MOVES_MAX,
    DESTROYER_MOVES_MAX,  CRUISER_MOVES_MAX,    CARRIER_MOVES_MAX,
    BATTLESHIP_MOVES_MAX, BATTLESHIP_MOVES_MAX, BOMBER_MOVES_MAX};

extern const uint8_t STATES_MOVE_SEA[SEA_UNIT_TYPES];
const uint8_t STATES_MOVE_SEA[SEA_UNIT_TYPES] = {
    FIGHTER_STATES,     TRANS_EMPTY_STATES, TRANS_1I_STATES,
    TRANS_1A_STATES,    TRANS_1T_STATES,    TRANS_2I_STATES,
    TRANS_1I_1A_STATES, TRANS_1I_1T_STATES, SUB_STATES,
    DESTROYER_STATES,   CRUISER_STATES,     CARRIER_STATES,
    BATTLESHIP_STATES,  BATTLESHIP_STATES,  BOMBER_SEA_STATES};

extern const uint8_t COST_UNIT_SEA[SEA_UNIT_TYPES];
const uint8_t COST_UNIT_SEA[SEA_UNIT_TYPES] = {
    FIGHTER_COST,    TRANSPORT_COST,  TRANSPORT_COST, TRANSPORT_COST,
    TRANSPORT_COST,  TRANSPORT_COST,  TRANSPORT_COST, TRANSPORT_COST,
    SUB_COST,        DESTROYER_COST,  CRUISER_COST,   CARRIER_COST,
    BATTLESHIP_COST, BATTLESHIP_COST, BOMBER_COST};

extern const uint8_t STATES_STAGING[SEA_UNIT_TYPES];
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

extern const uint8_t STATES_UNLOADING[SEA_UNIT_TYPES];
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

extern const uint8_t LOAD_TANK[SEA_UNIT_TYPES];
const uint8_t LOAD_TANK[SEA_UNIT_TYPES] = {255,
                                                TRANS_1T,
                                                TRANS_1I_1T,
                                                255,
                                                255,
                                                255,
                                                255,
                                                255,
                                                255,
                                                255,
                                                255,
                                                255,
                                                255,
                                                255,
                                                255};

extern const uint8_t LOAD_ARTILLERY[SEA_UNIT_TYPES];
const uint8_t LOAD_ARTILLERY[SEA_UNIT_TYPES] = {255,
                                                TRANS_1A,
                                                TRANS_1I_1A,
                                                255,
                                                255,
                                                255,
                                                255,
                                                255,
                                                255,
                                                255,
                                                255,
                                                255,
                                                255,
                                                255,
                                                255};

extern const uint8_t LOAD_INFANTRY[SEA_UNIT_TYPES];
const uint8_t LOAD_INFANTRY[SEA_UNIT_TYPES] = {255,
                                                TRANS_1I,
                                                TRANS_2I,
                                                TRANS_1I_1A,
                                                TRANS_1I_1T,
                                                255,
                                                255,
                                                255,
                                                255,
                                                255,
                                                255,
                                                255,
                                                255,
                                                255,
                                                255};



#endif
