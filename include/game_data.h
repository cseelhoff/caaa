#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "config.h"

void getUserInput();
void getAIInput();
void initializeGameData();
void setPrintableStatus();
void buildCache();
void setPrintableStatusLands();
void setPrintableStatusSeas();
void stage_transport_units();
void move_land_units();
void move_transport_units();
void move_sea_units();
void move_fighter_units();
void move_bomber_units();
void resolve_sea_battles();
void unload_transports();
void bomb_factories();
void bombard_shores();
void fire_aa_guns();
void resolve_land_battles();
void land_air_units();
void move_aa_guns();
void reset_units_fully();
void buy_units();
void crash_air_units();
void collect_money();
#endif
