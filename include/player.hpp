#ifndef PLAYER_H
#define PLAYER_H

#include <array>
#include <sys/types.h>

#define PLAYERS_COUNT 5
#define PLAYER_ALIGNMENT 64

using PlayerArray = std::array<uint, PLAYERS_COUNT>;

struct Player {
  const char* name;
  const char* capital_name;
  const char* color;
  uint capital_territory_index;
  PlayerArray is_allied; //alied with self
  bool is_human;
} __attribute__((aligned(PLAYER_ALIGNMENT)));

using PlayersStructs = std::array<Player, PLAYERS_COUNT>;

extern const PlayersStructs PLAYERS;

#endif
