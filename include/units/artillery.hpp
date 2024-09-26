#pragma once

#include <string_view>
#include <sys/types.h>

constexpr std::string_view ARTILLERY_NAME = "artillery";
constexpr uint ARTILLERY_ATTACK = 2;
constexpr uint ARTILLERY_DEFENSE = 2;
constexpr uint ARTILLERY_MOVES_MAX = 1;
constexpr uint ARTILLERY_COST = 4;
constexpr uint ARTILLERY_WEIGHT = 3;
constexpr uint ARTILLERY_STATES = ARTILLERY_MOVES_MAX + 1;