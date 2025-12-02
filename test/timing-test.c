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

#define DUDECT_IMPLEMENTATION
#include "dudect_wrapper.h"
#include "unity.h"

#include <stdlib.h>
#include <string.h>

#define NUM_MEASURE 10000
#define MAX_CNT 100

// List and configurations for the tests
#define DUDECT_TEST_LIST                                                       \
  X(expected_fail, ALEA_EXPECTED_FAIL, sizeof(int64_t), DUDECT_LEAKAGE_FOUND)  \
  X(random_bytes, ALEA_RANDOM_BYTES, sizeof(uint8_t),                          \
    DUDECT_NO_LEAKAGE_EVIDENCE_YET)                                            \
  X(hwt_int32, ALEA_RANDOM_HWT_INT32, sizeof(int32_t),                         \
    DUDECT_NO_LEAKAGE_EVIDENCE_YET)                                            \
  X(hwt_int64, ALEA_RANDOM_HWT_INT64, sizeof(int64_t),                         \
    DUDECT_NO_LEAKAGE_EVIDENCE_YET)                                            \
  X(cbd_int32, ALEA_RANDOM_CBD_INT32, sizeof(int32_t),                         \
    DUDECT_NO_LEAKAGE_EVIDENCE_YET)                                            \
  X(cbd_int64, ALEA_RANDOM_CBD_INT64, sizeof(int64_t),                         \
    DUDECT_NO_LEAKAGE_EVIDENCE_YET)

// Define the test function generation macro
#define DEFINE_DUDECT_TEST(NAME, API, UNIT_BYTES, STATE)                       \
  static void test_##NAME(void) {                                              \
    g_current_rng_api_to_test = API;                                           \
    dudect_state_t state = test_base((dudect_config_t){                        \
        .chunk_size = N * UNIT_BYTES, .number_measurements = NUM_MEASURE});    \
    TEST_ASSERT_EQUAL(STATE, state);                                           \
  }

void setUp(void) {
  uint8_t initial_seed[ALEA_SEED_SIZE_SHAKE256];
  memset(initial_seed, 0, ALEA_SEED_SIZE_SHAKE256);
  g_state = alea_init(initial_seed, ALEA_ALGORITHM_SHAKE256);
}

void tearDown(void) { alea_free(g_state); }

static dudect_state_t test_base(dudect_config_t config) {
  dudect_ctx_t ctx;
  dudect_init(&ctx, &config);

  dudect_state_t state = DUDECT_NO_LEAKAGE_EVIDENCE_YET;
  size_t cnt = 0;
  uint8_t seed[ALEA_SEED_SIZE_SHAKE256];
  while (state == DUDECT_NO_LEAKAGE_EVIDENCE_YET && cnt < MAX_CNT) {
    // Generate a new seed for each iteration
    for (size_t i = 0; i < ALEA_SEED_SIZE_SHAKE256; i++) {
      seed[i] = rand() % 256; // Random byte
    }
    alea_reseed(g_state, seed);
    state = dudect_main(&ctx);
    cnt += 1;
  }
  dudect_free(&ctx);
  return state;
}

// Define the test function for each configuration
#define X(NAME, API, UNIT_BYTES, STATE)                                        \
  DEFINE_DUDECT_TEST(NAME, API, UNIT_BYTES, STATE)
DUDECT_TEST_LIST
#undef X

int main() {
  UNITY_BEGIN();
#define X(NAME, API, UNIT_BYTES, STATE) RUN_TEST(test_##NAME);
  DUDECT_TEST_LIST
#undef X
  return UNITY_END();
}
