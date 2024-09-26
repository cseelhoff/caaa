#pragma once

#include <string_view>
#include <sys/types.h>

constexpr std::string_view TANK_NAME = "tanks";
constexpr uint TANK_ATTACK = 3;
constexpr uint TANK_DEFENSE = 3;
constexpr uint TANK_MOVES_MAX = 2;
constexpr uint TANK_COST = 5;
constexpr uint TANK_WEIGHT = 3;
constexpr uint TANK_STATES = TANK_MOVES_MAX + 1;