#ifndef SEA_DATA_H
#define SEA_DATA_H

#include "config.h"
#include "sea.h"
//  ATLANTIC         BALTIC         ATLANTIC
//  PACIFIC | USA | ATLANTIC | ENG | BALTIC | GER | RUS | JAP | PAC

Sea Seas[SEAS_COUNT] = {
    "Pacific",  1, 2, {1, 0, 0, 0, 0, 0, 0}, {0, 4, 0, 0, 0, 0},
    "Atlantic", 2, 2, {0, 2, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0},
    "Baltic",   1, 2, {1, 0, 0, 0, 0, 0, 0}, {0, 4, 0, 0, 0, 0}};

#endif
