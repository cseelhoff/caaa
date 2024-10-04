#pragma once
#include <sys/types.h>
#pragma once

#include <string_view>

constexpr std::string_view SUB_NAME = "subs";
constexpr uint SUB_ATTACK = 2;
constexpr uint SUB_DEFENSE = 1;
constexpr uint SUB_MOVES_MAX = 2;
constexpr uint SUB_COST = 8;
constexpr uint SUB_DONE_MOVING = 0;
constexpr uint SUB_BOMBARD = 0;
constexpr uint SUB_STATES = SUB_MOVES_MAX + SUB_BOMBARD;
