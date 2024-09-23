#include "player.hpp"

const PlayersStructs PLAYERS = {
    {{0, "Rus", "Rus", "\033[1;31m", 3, {true, false, true, false, true}, false},
     {1, "Ger", "Ger", "\033[1;34m", 2, {false, true, false, true, false}, false},
     {0, "Eng", "Eng", "\033[1;95m", 1, {true, false, true, false, true}, false},
     {1, "Jap", "Jap", "\033[1;33m", 4, {false, true, false, true, false}, false},
     {0, "USA", "USA", "\033[1;32m", 0, {true, false, true, false, true}, false}}};

const std::array<std::vector<uint>, TEAMS_COUNT> TEAMS = {std::vector<uint>{0U, 2U, 4U}, std::vector<uint>{1U, 3U}};
