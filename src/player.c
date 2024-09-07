#include "player.h"
#include "typedefs.h"

const Player PLAYERS[PLAYERS_COUNT] = {
    // "Rus", "Rus", "\033[1;31m", 3, 0, {true, false, true, false, true}, true,
    // "Ger", "Ger", "\033[1;34m", 2, 1, {false, true, true, false, false}, true,
    // "Eng", "Eng", "\033[1;95m", 1, 0, {false, true, true, true, false}, true,
    // "Jap", "Jap", "\033[1;33m", 4, 1, {false, true, false, true, false}, true,
    // "USA", "USA", "\033[1;32m", 0, 0, {true, false, true, false, true}, true
    "Rus", "Rus", "\033[1;31m", 3, 0, {true, false, true, false, true},  false,
    "Ger", "Ger", "\033[1;34m", 2, 1, {false, true, false, true, false}, false,
    "Eng", "Eng", "\033[1;95m", 1, 0, {true, false, true, false, true},  false,
    "Jap", "Jap", "\033[1;33m", 4, 1, {false, true, false, true, false}, false,
    "USA", "USA", "\033[1;32m", 0, 0, {true, false, true, false, true},  false};
inline bool are_players_allied(PlayerIndex player_idx, PlayerIndex ally_idx) {
  return PLAYERS[player_idx].is_allied[ally_idx];
}
// inline Player get_player(PlayerIndex player_idx) {
//   return PLAYERS[player_idx];
// }
inline bool is_player_human(PlayerIndex player_idx) {
  return PLAYERS[player_idx].is_human;
}
inline LandIndex get_player_capital(PlayerIndex player_idx) {
  return PLAYERS[player_idx].capital_territory_index;
}
inline char* get_player_name(PlayerIndex player_idx) {
  return PLAYERS[player_idx].name;
}
inline char* get_player_color(PlayerIndex player_idx) {
  return PLAYERS[player_idx].color;
}