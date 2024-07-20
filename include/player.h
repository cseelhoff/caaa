#ifndef PLAYER_H
#define PLAYER_H

#include "territory.h"
#include <stdbool.h>

typedef enum {
    Axis,
    Allies
} Team;

#define TOTAL_PLAYERS 2

typedef struct {
    const char* name;
    Team team;
    bool is_human;
    Territory capital;
    unsigned char capital_index;
    bool is_allied[TOTAL_PLAYERS - 1];
    unsigned char index;
    int money;
    int reserved_money;
} Player;

// Function prototypes
Player* player_new(const char* name, unsigned char team, unsigned char index);
void player_free(Player* player);

#endif // PLAYER_H