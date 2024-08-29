#pragma once
#define CARRIER_NAME "carriers"
#define CARRIER_ATTACK 1
#define CARRIER_DEFENSE 2
#define CARRIER_MOVES_MAX 2
#define CARRIER_COST 14

typedef enum {
    CARRIER_DONE_MOVING,
    CARRIER_UNMOVED,
    CARRIER_STATES
} CarrierState;
