#pragma once
#include <array>
#include <sys/types.h>

#define CANALS_COUNT 2
#define CANAL_STATES 4
#define ALIGNMENT_32 32

using TwoSeas = std::array<uint, 2>;
using TwoLands = std::array<uint, 2>;

struct Canal {
    const char* name;
    TwoSeas seas;
    TwoLands lands;
} __attribute__((aligned(ALIGNMENT_32)));

using CanalStructs = std::array<Canal, CANALS_COUNT>;

extern const CanalStructs CANALS;
