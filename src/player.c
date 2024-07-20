#include "player.h"

// Player functions
Player* player_new(const char* name, unsigned char team, unsigned char index) {
    Player* player = (Player*)calloc(1, sizeof(Player));
    player->name = name;
    player->team = team;
    player->is_human = false;
    player->capital = "";
    return player;
}

void player_free(Player* player) {
    free(player);
}