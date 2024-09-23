#pragma once

#include <algorithm>
#include <array>

constexpr uint ALIGNMENT_128 = 128;
constexpr uint ALIGNMENT_64 = 64;
constexpr uint ALIGNMENT_32 = 32;

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
template <std::size_t Rows, std::size_t Cols>
class Uint2DArray {
public:
    using ArrayType = std::array<uint, Rows * Cols>;

    [[nodiscard]] uint val(std::size_t row, std::size_t col) const {
        return data_[row * Cols + col];
    }

    uint& ref(std::size_t row, std::size_t col) {
        return data_[row * Cols + col];
    }

    void fill(const uint& value) {
        data_.fill(value);
    }

    std::array<uint, Cols>& arr(std::size_t row) {
        return &data_[row * Cols];
    }

private:
    ArrayType data_;
};