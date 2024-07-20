#ifndef TERRITORY_H
#define TERRITORY_H

#include "connection.h"
#include "player.h"
#include "inactive_unit_stack.h"
#include "active_unit_stack.h"

#define TOTAL_ACTIVE_UNIT_STATUS_TYPES 1
#define TOTAL_INACTIVE_UNIT_STATUS_TYPES 1
#define TOTAL_PLAYERS 2

typedef struct {
    int a;
    /*
    const char* name;
    unsigned char land_value;
    bool is_water;
    Player original_owner;
    Connection connections[10]; // Simplified to fixed-size array
    Connection water_connections[10];
    Connection land_connections[10];
    // Skipping adjacent territories arrays due to complexity in C
    unsigned int index;
    Player owner;
    bool is_owned;
    bool is_ally_owned;
    unsigned char factory_max;
    unsigned char factory_health;
    unsigned char construction_remaining;
    bool recently_conquered;
    ActiveUnitStack my_unit_stacks[TOTAL_ACTIVE_UNIT_STATUS_TYPES]; 
    InactiveUnitStack inactive_armies[TOTAL_PLAYERS - 1][TOTAL_INACTIVE_UNIT_STATUS_TYPES]; // 2 total players, simplified representation
    //inactive_armies: [[InactiveUnitStack{ unit_health: None, quantity: 0, stack_after_hit: None }; 1]; 2]   // 2 total players, 1 unit status type
    */
} Territory;

void territory_init(Territory* territory, const char* name, unsigned char land_value, unsigned char original_owner_id, unsigned int index);
void territory_build_factory(Territory* territory);
void territory_reset_factory(Territory* territory);

#endif // TERRITORY_H