#pragma once

#include <string_view>
#include <sys/types.h>

constexpr std::string_view INFANTRY_NAME = "infantry";
constexpr uint INFANTRY_ATTACK = 1;
constexpr uint INFANTRY_DEFENSE = 2;
constexpr uint INFANTRY_MOVES_MAX = 1;
constexpr uint INFANTRY_COST = 3;
constexpr uint INFANTRY_WEIGHT = 2;
constexpr uint INFANTRY_STATES = INFANTRY_MOVES_MAX + 1;