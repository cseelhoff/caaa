#ifndef CANAL_H
#define CANAL_H

#include <array>

#define CANALS_COUNT 2
#define CANAL_STATES 4

typedef struct {
    char* name;
    int seas[2];
    int lands[2];
} Canal;

extern const Canal CANALS[CANALS_COUNT];

#endif