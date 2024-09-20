#ifndef CANAL_H
#define CANAL_H

#include <array>

#define CANALS_COUNT 2
#define CANAL_STATES 4

using Canal = struct {
    char* name;
    int seas[2];
    int lands[2];
};

extern const Canal CANALS[CANALS_COUNT];

#endif