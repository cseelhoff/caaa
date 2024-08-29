#pragma once
#define TRANSPORT_ATTACK 0
#define TRANSPORT_DEFENSE 0
#define TRANSPORT_MOVES_MAX 2
#define TRANSPORT_COST 8
#define TRANSPORT_CARRY_LAND 5

#define TRANSPORT_NAME "tran_empty"
#define TRANS_EMPTY_UNLOADING_STATES 0
#define TRANS_EMPTY_STAGING_STATES 1
typedef enum {    
    TRANS_EMPTY_DONE_MOVING,
    TRANS_EMPTY_1_MOVE_LEFT,
    TRANS_EMPTY_UNMOVED,
    TRANS_EMPTY_STAGING,
    TRANS_EMPTY_STATES
} TransEmptyState;

#define TRANS_1I_NAME "tran_1i"
#define TRANS_1I_UNLOADING_STATES 1
#define TRANS_1I_STAGING_STATES 1
typedef enum {    
    TRANS_1I_DONE_MOVING,
    TRANS_1I_UNLOADING,
    TRANS_1I_1_MOVE_LEFT,
    TRANS_1I_UNMOVED,
    TRANS_1I_STAGING,
    TRANS_1I_STATES
} Trans1IState;

#define TRANS_1A_NAME "tran_1a"
#define TRANS_1A_UNLOADING_STATES 1
#define TRANS_1A_STAGING_STATES 1
typedef enum {
    TRANS_1A_DONE_MOVING,
    TRANS_1A_UNLOADING,
    TRANS_1A_1_MOVE_LEFT,
    TRANS_1A_UNMOVED,
    TRANS_1A_STAGING,
    TRANS_1A_STATES
} Trans1AState;

#define TRANS_1T_NAME "tran_1t"
#define TRANS_1T_UNLOADING_STATES 1
#define TRANS_1T_STAGING_STATES 1
typedef enum {
    TRANS_1T_DONE_MOVING,
    TRANS_1T_UNLOADING,
    TRANS_1T_1_MOVE_LEFT,
    TRANS_1T_UNMOVED,
    TRANS_1T_STAGING,
    TRANS_1T_STATES
} Trans1TState;

#define TRANS_2I_NAME "tran_2i"
#define TRANS_2I_UNLOADING_STATES 1
#define TRANS_2I_STAGING_STATES 0
typedef enum {
    TRANS_2I_DONE_MOVING,
    TRANS_2I_UNLOADING,
    TRANS_2I_1_MOVE_LEFT,
    TRANS_2I_UNMOVED,
    TRANS_2I_STATES
} Trans2IState;

#define TRANS_1I_1A_NAME "tran_1i_1a"
#define TRANS_1I_1A_UNLOADING_STATES 1
#define TRANS_1I_1A_STAGING_STATES 0
typedef enum {
    TRANS_1I_1A_DONE_MOVING,
    TRANS_1I_1A_UNLOADING,
    TRANS_1I_1A_1_MOVE_LEFT,
    TRANS_1I_1A_UNMOVED,
    TRANS_1I_1A_STATES
} Trans1I1AState;

#define TRANS_1I_1T_NAME "tran_1i_1t"
#define TRANS_1I_1T_UNLOADING_STATES 1
#define TRANS_1I_1T_STAGING_STATES 0
typedef enum {
    TRANS_1I_1T_DONE_MOVING,
    TRANS_1I_1T_UNLOADING,
    TRANS_1I_1T_1_MOVE_LEFT,
    TRANS_1I_1T_UNMOVED,
    TRANS_1I_1T_STATES
} Trans1I1TState;
