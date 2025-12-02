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

#include "dudect_wrapper.h"
#include <stdio.h>  // for debug print
#include <stdlib.h> // for rand()
#include <string.h> // for memset

// global variable to hold the current RNG API to test
current_rng_api_t g_current_rng_api_to_test = ALEA_RANDOM_BYTES; // default
alea_state *g_state = NULL; // global alea_state
const size_t NUM_FLIP = 21; // default number of flips for CBD
const size_t N = 1 << 12;   // default sample num, 4096
const int HWT = N * 2 / 3;  // default Hamming weight, 2/3 of N

// prepare inputs with respect to g_current_rng_api_to_test
void prepare_inputs(dudect_config_t *c, uint8_t *input_data, uint8_t *classes) {
  for (size_t i = 0; i < c->number_measurements; i++) {
    alea_get_random_bytes(g_state, classes + i, 1);
    classes[i] &= 1; // only support two classes (0 or 1)

    uint8_t *input_buf = input_data + (size_t)i * c->chunk_size;

    switch (g_current_rng_api_to_test) {
    case ALEA_RANDOM_BYTES:
    case ALEA_RANDOM_HWT_INT32:
    case ALEA_RANDOM_HWT_INT64:
    case ALEA_RANDOM_CBD_INT32:
    case ALEA_RANDOM_CBD_INT64:
    case ALEA_EXPECTED_FAIL:
      if (classes[i] & 1) {
        alea_get_random_bytes(g_state, input_buf, c->chunk_size);
      } else {
        memset(input_buf, 0x00, c->chunk_size); // set 0 for class 0
      }
      break;
    default:
      fprintf(stderr, "Error: Unknown RNG API selected for prepare_inputs.\n");
      exit(1);
    }
  }
}

// define the computation function that will be called by dudect
uint8_t do_one_computation(uint8_t *data) {
  uint8_t result = 0;
  size_t output_size = N;

  switch (g_current_rng_api_to_test) {
  case ALEA_RANDOM_BYTES:
    alea_get_random_bytes(g_state, data, output_size); // API call
    break;
  case ALEA_RANDOM_HWT_INT32:
    alea_sample_hwt_int32_array(g_state, (int32_t *)data, output_size,
                                HWT); // API call
    break;
  case ALEA_RANDOM_HWT_INT64:
    alea_sample_hwt_int64_array(g_state, (int64_t *)data, output_size,
                                HWT); // API call
    break;
  case ALEA_RANDOM_CBD_INT32:
    alea_sample_cbd_int32_array(g_state, (int32_t *)data, output_size,
                                NUM_FLIP); // API call
    break;
  case ALEA_RANDOM_CBD_INT64:
    alea_sample_cbd_int64_array(g_state, (int64_t *)data, output_size,
                                NUM_FLIP); // API call
    break;
  case ALEA_EXPECTED_FAIL:
    if (data[0] != 0) {
      alea_get_random_bytes(g_state, data, output_size); // API call
    } else {
      alea_get_random_bytes(g_state, data, output_size / 256 * 255); // API call
    }
    break;
  default:
    fprintf(stderr,
            "Error: Unknown RNG API selected for do_one_computation.\n");
    exit(1);
  }
  // run any computation to avoid optimization
  for (size_t i = 0; i < output_size; ++i) {
    result ^= data[i];
  }
  return result;
}
