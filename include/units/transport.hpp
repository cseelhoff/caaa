#pragma once
#include <string>
#include <sys/types.h>
constexpr std::string_view TRANSPORT_NAME = "transempty";
constexpr uint TRANSPORT_ATTACK = 0;
constexpr uint TRANSPORT_DEFENSE = 0;
constexpr uint TRANSPORT_MOVES_MAX = 2;
constexpr uint TRANSPORT_COST = 8;
constexpr uint TRANSPORT_CARRY_LAND = 5;

constexpr uint TRANSEMPTY_UNLOADING_STATES = 0;
constexpr uint TRANSEMPTY_STAGING_STATES = 1;
constexpr uint TRANSEMPTY_STATES = TRANSPORT_MOVES_MAX + TRANSEMPTY_UNLOADING_STATES + TRANSEMPTY_STAGING_STATES + 1;
constexpr uint TRANSEMPTY_DONE_MOVING = 0;
constexpr uint TRANSEMPTY_UNMOVED = 1;

constexpr std::string_view TRANS1I_NAME = "trans1i";
constexpr uint TRANS1I_UNLOADING_STATES = 1;
constexpr uint TRANS1I_STAGING_STATES = 1;
constexpr uint TRANS1I_STATES = TRANSPORT_MOVES_MAX + TRANS1I_UNLOADING_STATES + TRANS1I_STAGING_STATES + 1;
constexpr uint TRANS1I_DONE_MOVING = 0;
constexpr uint TRANS1I_UNMOVED = 1;

constexpr std::string_view TRANS1A_NAME = "trans1a";
constexpr uint TRANS1A_UNLOADING_STATES = 1;
constexpr uint TRANS1A_STAGING_STATES = 1;
constexpr uint TRANS1A_STATES = TRANSPORT_MOVES_MAX + TRANS1A_UNLOADING_STATES + TRANS1A_STAGING_STATES + 1;
constexpr uint TRANS1A_DONE_MOVING = 0;
constexpr uint TRANS1A_UNMOVED = 1;

constexpr std::string_view TRANS1T_NAME = "trans1t";
constexpr uint TRANS1T_UNLOADING_STATES = 1;
constexpr uint TRANS1T_STAGING_STATES = 1;
constexpr uint TRANS1T_STATES = TRANSPORT_MOVES_MAX + TRANS1T_UNLOADING_STATES + TRANS1T_STAGING_STATES + 1;
constexpr uint TRANS1T_DONE_MOVING = 0;
constexpr uint TRANS1T_UNMOVED = 1;

constexpr std::string_view TRANS2I_NAME = "trans2i";
constexpr uint TRANS2I_UNLOADING_STATES = 1;
constexpr uint TRANS2I_STAGING_STATES = 0;
constexpr uint TRANS2I_STATES = TRANSPORT_MOVES_MAX + TRANS2I_UNLOADING_STATES + TRANS2I_STAGING_STATES + 1;
constexpr uint TRANS2I_DONE_MOVING = 0;
constexpr uint TRANS2I_UNMOVED = 1;

constexpr std::string_view TRANS1I1A_NAME = "trans1i1a";
constexpr uint TRANS1I1A_UNLOADING_STATES = 1;
constexpr uint TRANS1I1A_STAGING_STATES = 0;
constexpr uint TRANS1I1A_STATES = TRANSPORT_MOVES_MAX + TRANS1I1A_UNLOADING_STATES + TRANS1I1A_STAGING_STATES + 1;
constexpr uint TRANS1I1A_DONE_MOVING = 0;
constexpr uint TRANS1I1A_UNMOVED = 1;

constexpr std::string_view TRANS1I1T_NAME = "trans1i1t";
constexpr uint TRANS1I1T_UNLOADING_STATES = 1;
constexpr uint TRANS1I1T_STAGING_STATES = 0;
constexpr uint TRANS1I1T_STATES = TRANSPORT_MOVES_MAX + TRANS1I1T_UNLOADING_STATES + TRANS1I1T_STAGING_STATES + 1;
constexpr uint TRANS1I1T_DONE_MOVING = 0;
constexpr uint TRANS1I1T_UNMOVED = 1;
