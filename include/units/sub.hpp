#pragma once
#include <sys/types.h>
#pragma once

#include <string_view>

constexpr std::string_view SUB_NAME = "subs";
constexpr int SUB_ATTACK = 2;
constexpr int SUB_DEFENSE = 1;
constexpr int SUB_MOVES_MAX = 2;
constexpr int SUB_COST = 8;
constexpr int SUB_DONE_MOVING = 0;
constexpr int SUB_UNMOVED = 2;
constexpr uint SUB_STATES = 3;