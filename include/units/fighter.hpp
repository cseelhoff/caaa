#pragma once

#include <string_view>
#include <sys/types.h>

constexpr std::string_view FIGHTER_NAME = "fighters";
constexpr uint FIGHTER_ATTACK = 3;
constexpr uint FIGHTER_DEFENSE = 4;
constexpr uint FIGHTER_MOVES_MAX = 4;
constexpr uint FIGHTER_COST = 10;
constexpr uint FIGHTER_DONE_MOVING = 0;
constexpr uint FIGHTER_UNMOVED = FIGHTER_MOVES_MAX;
constexpr uint FIGHTER_STATES = FIGHTER_MOVES_MAX + 1;