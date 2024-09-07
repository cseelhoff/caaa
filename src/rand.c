#include "rand.h"
#include <stdlib.h>

#define RANDOM_NUMBERS_SIZE 65536
Action RANDOM_NUMBERS[RANDOM_NUMBERS_SIZE] = {0};

void generate_random_numbers() {
  for (int i = 0; i < RANDOM_NUMBERS_SIZE; i++) {
    RANDOM_NUMBERS[i] = rand() % 256;
  }
}

inline Action get_random_number(Action idx) {
  return RANDOM_NUMBERS[idx];
}
