#include "rand.h"
#include <stdlib.h>

void generate_random_numbers() {
  for (int i = 0; i < RANDOM_NUMBERS_SIZE; i++) {
    RANDOM_NUMBERS[i] = rand() % 256;
  }
}