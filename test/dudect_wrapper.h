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

#ifndef DUDECT_WRAPPER_C_H
#define DUDECT_WRAPPER_C_H

#include "alea/alea.h"
#include "dudect.h"

typedef enum {
  ALEA_RANDOM_BYTES,
  ALEA_RANDOM_HWT_INT32,
  ALEA_RANDOM_HWT_INT64,
  ALEA_RANDOM_CBD_INT32,
  ALEA_RANDOM_CBD_INT64,
  ALEA_EXPECTED_FAIL
} current_rng_api_t;

// Set by Unity test cases.
extern current_rng_api_t g_current_rng_api_to_test;
extern alea_state *g_state;
extern const size_t NUM_FLIP; // number of flips for CBD sampling
extern const size_t N;        // chunk size for dudect
extern const int HWT;         // Hamming weight for HWT sampling

// API actually called by dudect
void prepare_inputs(dudect_config_t *c, uint8_t *input_data, uint8_t *classes);
uint8_t do_one_computation(uint8_t *data);

#endif // DUDECT_WRAPPER_C_H
