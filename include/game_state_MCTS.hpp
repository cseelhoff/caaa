#pragma once
#include "game_state_json.hpp"

struct GameStateMCTS {
  uint current_turn{};
  uint seed{};
  PlayersArray money{};
  LandStateArray land_states;
  SeaStateArray sea_states;
} __attribute__((aligned(ALIGNMENT_128)));
