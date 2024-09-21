#pragma once

#define TRANSPORT_NAME "tran_empty"
#define TRANSPORT_ATTACK 0
#define TRANSPORT_DEFENSE 0
#define TRANSPORT_MOVES_MAX 2
#define TRANSPORT_COST 8
#define TRANSPORT_CARRY_LAND 5

#define TRANS_EMPTY_UNLOADING_STATES 0
#define TRANS_EMPTY_STAGING_STATES 1
#define TRANS_EMPTY_STATES TRANSPORT_MOVES_MAX + TRANS_EMPTY_UNLOADING_STATES + TRANS_EMPTY_STAGING_STATES + 1
#define TRANS_EMPTY_DONE_MOVING 0
#define TRANS_EMPTY_UNMOVED 1

#define TRANS_1I_NAME "tran_1i"
#define TRANS_1I_UNLOADING_STATES 1
#define TRANS_1I_STAGING_STATES 1
#define TRANS_1I_STATES TRANSPORT_MOVES_MAX + TRANS_1I_UNLOADING_STATES + TRANS_1I_STAGING_STATES + 1
#define TRANS_1I_DONE_MOVING 0
#define TRANS_1I_UNMOVED 1

#define TRANS_1A_NAME "tran_1a"
#define TRANS_1A_UNLOADING_STATES 1
#define TRANS_1A_STAGING_STATES 1
#define TRANS_1A_STATES TRANSPORT_MOVES_MAX + TRANS_1A_UNLOADING_STATES + TRANS_1A_STAGING_STATES + 1
#define TRANS_1A_DONE_MOVING 0
#define TRANS_1A_UNMOVED 1

#define TRANS_1T_NAME "tran_1t"
#define TRANS_1T_UNLOADING_STATES 1
#define TRANS_1T_STAGING_STATES 1
#define TRANS_1T_STATES TRANSPORT_MOVES_MAX + TRANS_1T_UNLOADING_STATES + TRANS_1T_STAGING_STATES + 1
#define TRANS_1T_DONE_MOVING 0
#define TRANS_1T_UNMOVED 1

#define TRANS_2I_NAME "tran_2i"
#define TRANS_2I_UNLOADING_STATES 1
#define TRANS_2I_STAGING_STATES 0
#define TRANS_2I_STATES TRANSPORT_MOVES_MAX + TRANS_2I_UNLOADING_STATES + TRANS_2I_STAGING_STATES + 1
#define TRANS_2I_DONE_MOVING 0
#define TRANS_2I_UNMOVED 1

#define TRANS_1I_1A_NAME "tran_1i_1a"
#define TRANS_1I_1A_UNLOADING_STATES 1
#define TRANS_1I_1A_STAGING_STATES 0
#define TRANS_1I_1A_STATES TRANSPORT_MOVES_MAX + TRANS_1I_1A_UNLOADING_STATES + TRANS_1I_1A_STAGING_STATES + 1
#define TRANS_1I_1A_DONE_MOVING 0
#define TRANS_1I_1A_UNMOVED 1

#define TRANS_1I_1T_NAME "tran_1i_1t"
#define TRANS_1I_1T_UNLOADING_STATES 1
#define TRANS_1I_1T_STAGING_STATES 0
#define TRANS_1I_1T_STATES TRANSPORT_MOVES_MAX + TRANS_1I_1T_UNLOADING_STATES + TRANS_1I_1T_STAGING_STATES + 1
#define TRANS_1I_1T_DONE_MOVING 0
#define TRANS_1I_1T_UNMOVED 1
