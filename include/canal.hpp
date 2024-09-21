#pragma once
#include <array>
#include <sys/types.h>

#define CANALS_COUNT 2
#define CANAL_STATES 4
#define ALIGNMENT_32 32

struct Canal {
    const char* name;
    std::array<uint, 2> seas;
    std::array<uint, 2> lands;
} __attribute__((aligned(ALIGNMENT_32)));

using CanalStructs = std::array<Canal, CANALS_COUNT>;

extern const CanalStructs CANALS;
