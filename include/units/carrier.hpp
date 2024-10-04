#pragma once

#include <string_view>
#include <sys/types.h>

constexpr std::string_view CARRIER_NAME = "carriers";
constexpr uint CARRIER_ATTACK = 1;
constexpr uint CARRIER_DEFENSE = 2;
constexpr uint CARRIER_MOVES_MAX = 2;
constexpr uint CARRIER_COST = 14;
constexpr uint CARRIER_DONE_MOVING = 0;
constexpr uint CARRIER_BOMBARD = 0;
constexpr uint CARRIER_STATES = CARRIER_MOVES_MAX + CARRIER_BOMBARD;
