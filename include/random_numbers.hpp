#pragma once
#include "game_state_memory.hpp"
#include <array>
#include <sys/types.h>
constexpr uint RANDOM_MAX = 65536;

using RandomNumberArray = std::array<uint, RANDOM_MAX>;

extern RandomNumberArray RANDOM_NUMBERS;

void initialize_random_numbers();
