/*
 * Copyright 2025 CryptoLab, Inc.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "alea/alea.h"
#include "alea/algorithms.h"

#include "unity.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TEST_SIZE 100000
#define TEST_RANGE_32 100
#define TEST_RANGE_64 (1UL << 33)
#define TEST_HWT (TEST_SIZE * 2 / 3)
#define TEST_CBD 21
#define TEST_STD 3.2

#define VERIFY_SIGMA_FACTOR                                                    \
  3.0 // 99.7% of the values should be within this factor of the expected count
#define VERIFY_SIGMA_TOLER 0.03 // Tolerance for the sigma verification

#define FUNCTIONALITY_TEST_LIST                                                \
  X(random_range_32, get_random_uint32_array_in_range, uint32_t, TEST_SIZE,    \
    TEST_RANGE_32)                                                             \
  X(random_range_64, get_random_uint64_array_in_range, uint64_t, TEST_SIZE,    \
    TEST_RANGE_64)                                                             \
  X(random_hwt_8, sample_hwt_int8_array, int8_t, TEST_SIZE, TEST_HWT)          \
  X(random_hwt_32, sample_hwt_int32_array, int32_t, TEST_SIZE, TEST_HWT)       \
  X(random_hwt_64, sample_hwt_int64_array, int64_t, TEST_SIZE, TEST_HWT)       \
  X(random_cbd_32, sample_cbd_int32_array, int32_t, TEST_SIZE, TEST_CBD)       \
  X(random_cbd_64, sample_cbd_int64_array, int64_t, TEST_SIZE, TEST_CBD)       \
  X(random_gaussian_32, sample_gaussian_int32_array, int32_t, TEST_SIZE,       \
    TEST_STD)                                                                  \
  X(random_gaussian_64, sample_gaussian_int64_array, int64_t, TEST_SIZE,       \
    TEST_STD)

#define DEFINE_FUNCTIONALITY_TEST(NAME, API, TYPE, SIZE, OPT)                  \
  static void test_##NAME(void) {                                              \
    TYPE dst[SIZE];                                                            \
    alea_##API(g_state_128, dst, SIZE, OPT);                                   \
    check_##NAME(dst, SIZE, OPT);                                              \
    alea_##API(g_state_256, dst, SIZE, OPT);                                   \
    check_##NAME(dst, SIZE, OPT);                                              \
  }

#define CHECK_RANGE(bit)                                                       \
  void check_random_range_##bit(uint##bit##_t *dst, size_t size,               \
                                uint##bit##_t range) {                         \
    for (size_t i = 0; i < size; ++i) {                                        \
      TEST_ASSERT_LESS_THAN(range, dst[i]);                                    \
    }                                                                          \
    if (range > size)                                                          \
      return;                                                                  \
    int count[range];                                                          \
    memset(count, 0, sizeof(count));                                           \
    for (size_t i = 0; i < size; ++i) {                                        \
      count[dst[i]] += 1;                                                      \
    }                                                                          \
    double expected_count = (double)size / range;                              \
    double sigma = sqrt(expected_count * (1 - 1.0 / range));                   \
    int count_out_of_range = 0;                                                \
    for (size_t i = 0; i < range; ++i) {                                       \
      if (abs(count[i] - expected_count) > VERIFY_SIGMA_FACTOR * sigma) {      \
        count_out_of_range++;                                                  \
      }                                                                        \
    }                                                                          \
    TEST_ASSERT_LESS_OR_EQUAL(range *VERIFY_SIGMA_TOLER, count_out_of_range);  \
  }

#define CHECK_HWT(bit)                                                         \
  void check_random_hwt_##bit(int##bit##_t *dst, size_t size, int hwt) {       \
    int count[3] = {0, 0, 0};                                                  \
    for (size_t i = 0; i < size; ++i) {                                        \
      if (dst[i] < -1 || dst[i] > 1) {                                         \
        TEST_FAIL_MESSAGE("HWT value out of range");                           \
      }                                                                        \
      count[dst[i] + 1]++;                                                     \
    }                                                                          \
    TEST_ASSERT_EQUAL(size - hwt, count[1]);                                   \
    double diff = abs(count[0] - count[2]);                                    \
    TEST_ASSERT_EQUAL(1, (VERIFY_SIGMA_FACTOR * sqrt(hwt * 0.5) >= diff));     \
  }

#define CHECK_CBD(bit)                                                         \
  void check_random_cbd_##bit(int##bit##_t *dst, size_t size,                  \
                              size_t cbd_num_flips) {                          \
    double mean = 0;                                                           \
    for (size_t i = 0; i < size; ++i) {                                        \
      if (abs(dst[i]) > cbd_num_flips) {                                       \
        TEST_FAIL_MESSAGE("CBD value out of range");                           \
      }                                                                        \
      mean += dst[i];                                                          \
    }                                                                          \
    mean /= size;                                                              \
    double variance = 0;                                                       \
    for (size_t i = 0; i < size; ++i) {                                        \
      variance += (dst[i] - mean) * (dst[i] - mean);                           \
    }                                                                          \
    variance /= size;                                                          \
    double stdev = sqrt(variance);                                             \
    double sigma = sqrt(cbd_num_flips / 2.0);                                  \
    TEST_ASSERT_EQUAL(1, (sigma * (1 + VERIFY_SIGMA_TOLER) >= stdev));         \
    TEST_ASSERT_EQUAL(1, (sigma * (1 - VERIFY_SIGMA_TOLER) <= stdev));         \
  }

#define CHECK_GAUSSIAN(bit)                                                    \
  void check_random_gaussian_##bit(int##bit##_t *dst, size_t size,             \
                                   double sigma) {                             \
    double mean = 0;                                                           \
    for (size_t i = 0; i < size; ++i) {                                        \
      mean += dst[i];                                                          \
    }                                                                          \
    mean /= size;                                                              \
    double variance = 0;                                                       \
    for (size_t i = 0; i < size; ++i) {                                        \
      variance += (dst[i] - mean) * (dst[i] - mean);                           \
    }                                                                          \
    variance /= size;                                                          \
    double stdev = sqrt(variance);                                             \
    TEST_ASSERT_EQUAL(1, (sigma * (1 + VERIFY_SIGMA_TOLER) >= stdev));         \
    TEST_ASSERT_EQUAL(1, (sigma * (1 - VERIFY_SIGMA_TOLER) <= stdev));         \
  }

alea_state *g_state_128;
alea_state *g_state_256;

void setUp(void) {
  uint8_t initial_seed[ALEA_SEED_SIZE_SHAKE256];

  srand(time(NULL)); // Seed the random number generator
  for (size_t i = 0; i < ALEA_SEED_SIZE_SHAKE256; ++i) {
    initial_seed[i] = rand() % 256; // Random byte
  }

  g_state_128 = alea_init(initial_seed, ALEA_ALGORITHM_SHAKE128);
  g_state_256 = alea_init(initial_seed, ALEA_ALGORITHM_SHAKE256);
}

void tearDown(void) {
  alea_free(g_state_128);
  alea_free(g_state_256);
}

#define CHECK_FUNTION_LIST                                                     \
  Y(RANGE)                                                                     \
  Y(HWT)                                                                       \
  Y(CBD)                                                                       \
  Y(GAUSSIAN)

#define Y(NAME) CHECK_##NAME(32) CHECK_##NAME(64)
CHECK_FUNTION_LIST
CHECK_HWT(8)
#undef Y

#define X(NAME, API, TYPE, SIZE, OPT)                                          \
  DEFINE_FUNCTIONALITY_TEST(NAME, API, TYPE, SIZE, OPT)
FUNCTIONALITY_TEST_LIST
#undef X

int main() {
  UNITY_BEGIN();
#define X(NAME, API, TYPE, SIZE, OPT) RUN_TEST(test_##NAME);
  FUNCTIONALITY_TEST_LIST
#undef X

  return UNITY_END();
}
