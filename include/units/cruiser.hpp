#pragma once

#include <string_view>
#include <sys/types.h>

constexpr std::string_view CRUISER_NAME = "cruisers";
constexpr uint CRUISER_ATTACK = 3;
constexpr uint CRUISER_DEFENSE = 3;
constexpr uint CRUISER_MOVES_MAX = 2;
constexpr uint CRUISER_COST = 10;
constexpr uint CRUISER_DONE_MOVING = 1;
constexpr uint CRUISER_BOMBARD = 1;
constexpr uint CRUISER_STATES = CRUISER_MOVES_MAX + CRUISER_BOMBARD;
