#pragma once

#include <string_view>
#include <sys/types.h>

constexpr std::string_view AA_GUN_NAME = "aaguns";
constexpr uint AA_GUN_ATTACK = 0;
constexpr uint AA_GUN_DEFENSE = 0;
constexpr uint AA_GUN_MOVES_MAX = 1;
constexpr uint AA_GUN_COST = 5;
constexpr uint AA_GUN_AA_STRENGTH = 1;
constexpr uint AA_GUN_WEIGHT = 6;
constexpr uint AA_GUN_STATES = AA_GUN_MOVES_MAX + 1;