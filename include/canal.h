#ifndef CANAL_H
#define CANAL_H

#include <stdint.h>

#define CANALS_COUNT 2
#define CANAL_STATES 4

typedef struct {
    char* name;
    uint8_t seas[2];
    uint8_t lands[2];
} Canal;

extern const Canal CANALS[CANALS_COUNT];

#endif