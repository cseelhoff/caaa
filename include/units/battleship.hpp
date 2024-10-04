#pragma once

#include <string_view>
#include <sys/types.h>

constexpr std::string_view BATTLESHIP_NAME = "battleship";
constexpr uint BATTLESHIP_ATTACK = 4;
constexpr uint BATTLESHIP_DEFENSE = 4;
constexpr uint BATTLESHIP_MOVES_MAX = 2;
constexpr uint BATTLESHIP_COST = 22;
constexpr uint BATTLESHIP_DONE_MOVING = 1;
constexpr uint BATTLESHIP_BOMBARD = 1;
constexpr uint BATTLESHIP_STATES = BATTLESHIP_MOVES_MAX + BATTLESHIP_BOMBARD;
constexpr std::string_view BS_DAMAGED_NAME = "damaged_bs";