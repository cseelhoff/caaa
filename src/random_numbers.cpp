#include "random_numbers.hpp"
#include <cstdlib>
void initialize_random_numbers() {
  for (uint i = 0; i < RANDOM_MAX; i++) {
    // NOLINTNEXTLINE(cert-msc50-cpp, concurrency-mt-unsafe, cert-msc30-c)
    RANDOM_NUMBERS[i] = (static_cast<uint>(rand()) % ACTION_COUNT);
  }
}