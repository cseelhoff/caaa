#ifndef ACTIVE_UNIT_STACK_H
#define ACTIVE_UNIT_STACK_H

#include "unit_status.h"
#include "player.h"

#define TOTAL_ACTIVE_UNIT_STATUS_TYPES 1

typedef struct ActiveUnitStack {
    UnitStatus unit_status;
    Player owner;
    unsigned char unit_quantity;
    struct ActiveUnitStack* unitStatusAfterMove;
    struct ActiveUnitStack* unitStatusAfterHit;
    struct ActiveUnitStack* unitStatusAfterTurn;
} ActiveUnitStack;

// Function to initialize a UnitStack
// Note: This simplified version does not automatically create the after-move/hit/turn statuses to avoid recursive calls
void active_unit_stack_init(ActiveUnitStack* stack, UnitStatus unit_status, Player owner, unsigned char unit_quantity);

// Function to create arrays of UnitStack for unmoved and moved unit statuses
// Note: The implementation details for this function are omitted due to the complexity and need for dynamic memory management
void create_unit_stacks(Player* players, int players_count, UnitStatus* unmoved_unit_statuses, int unmoved_count, UnitStatus* moved_unit_statuses, int moved_count, ActiveUnitStack** unmoved_unit_stacks, int* unmoved_stacks_count, ActiveUnitStack** moved_unit_stacks, int* moved_stacks_count);

#endif