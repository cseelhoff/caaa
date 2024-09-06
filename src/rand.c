#include "rand.h"
#include <stdlib.h>

Action RANDOM_NUMBERS[RANDOM_NUMBERS_SIZE] = {0};

void generate_random_numbers() {
  for (int i = 0; i < RANDOM_NUMBERS_SIZE; i++) {
    RANDOM_NUMBERS[i] = rand() % 256;
  }
}