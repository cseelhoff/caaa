#pragma once

#include <string_view>
#include <sys/types.h>

constexpr std::string_view BOMBER_NAME = "bombers";
constexpr uint BOMBER_ATTACK = 4;
constexpr uint BOMBER_DEFENSE = 1;
constexpr uint BOMBER_MOVES_MAX = 6;
constexpr uint BOMBER_COST = 14;
constexpr uint BOMBER_RAID_STRENGTH = 1;
constexpr uint BOMBER_BOMBARD = 0;
constexpr uint BOMBER_LAND_STATES = BOMBER_MOVES_MAX + 1;
constexpr uint BOMBER_SEA_STATES = BOMBER_MOVES_MAX - 1;