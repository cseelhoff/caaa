#include "rand.h"
#include "typedefs.h"
#include <stdlib.h>
#include <sys/types.h>

#define RANDOM_NUMBERS_SIZE 65536
Action RANDOM_NUMBERS[RANDOM_NUMBERS_SIZE] = {0};

void generate_random_numbers() {
#pragma unroll 4
  for (int i = 0; i < RANDOM_NUMBERS_SIZE; i++) {
    RANDOM_NUMBERS[i] = (Action)(rand_r(0) % MAX_UINT8_T);
  }
}

inline Action get_random_number(ushort idx) { return RANDOM_NUMBERS[idx]; }
