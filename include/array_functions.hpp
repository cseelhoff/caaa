#pragma once

#include <algorithm>

#define COPY_FULL_ARRAY(src, dest) std::copy(std::begin(src), std::end(src), std::begin(dest));
#define COPY_SUB_ARRAY(src, dest, count) std::copy_n((src).begin(), count, (dest).begin());
#define FILL_ARRAY(arr, value) std::fill(std::begin(arr), std::end(arr), value);
#define FILL_2D_ARRAY(arr, value)                                                                  \
  for (auto& row : arr) {                                                                          \
    std::fill(std::begin(row), std::end(row), value);                                              \
  }
#define FILL_3D_ARRAY(array, value)                                                                \
  for (auto& matrix : array) {                                                                     \
    for (auto& row : matrix) {                                                                     \
      std::fill(std::begin(row), std::end(row), value);                                            \
    }                                                                                              \
  }
