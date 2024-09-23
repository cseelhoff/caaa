#pragma once
#include <array>
#include <sys/types.h>
#include <vector>

#define PLAYERS_COUNT 5
#define PLAYER_ALIGNMENT 64
constexpr uint TEAMS_COUNT = 2;

using BoolPlayerArray = std::array<bool, PLAYERS_COUNT>;

struct Player {
  uint team;
  const char* name;
  const char* capital_name;
  const char* color;
  uint capital_territory_index;
  BoolPlayerArray is_allied; // alied with self
  bool is_human;
} __attribute__((aligned(PLAYER_ALIGNMENT)));

using PlayersStructs = std::array<Player, PLAYERS_COUNT>;

extern const PlayersStructs PLAYERS;
extern const std::array<std::vector<uint>, 2> TEAMS;
