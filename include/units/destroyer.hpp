#pragma once

#include <string_view>
#include <sys/types.h>

constexpr std::string_view DESTROYER_NAME = "destroyers";
constexpr uint DESTROYER_ATTACK = 2;
constexpr uint DESTROYER_DEFENSE = 2;
constexpr uint DESTROYER_MOVES_MAX = 2;
constexpr uint DESTROYER_COST = 8;
constexpr uint DESTROYER_DONE_MOVING = 0;
constexpr uint DESTROYER_BOMBARD = 0;
constexpr uint DESTROYER_STATES = DESTROYER_MOVES_MAX + DESTROYER_BOMBARD;