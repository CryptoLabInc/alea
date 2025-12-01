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

#define ALEA_STATE_IMPLEMENTATION

#include "alea/algorithms.h"
#include "fips202.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  alea_algo algorithm;
  uint8_t *data;
  size_t len;
  size_t loc;
  keccak_state *state;
} alea_state;

#include "alea-builtin.h"
#include "alea-internal.h"
#include "alea/alea.h"

static alea_state *alea_init_builtin_SHAKE128(const uint8_t *seed) {
  alea_state *new = malloc(sizeof(alea_state));
  if (new == NULL)
    return NULL;

  new->algorithm = ALEA_ALGORITHM_SHAKE128;
  new->len = SHAKE128_RATE;
  new->loc = 0;
  new->data = malloc(new->len * sizeof(*(new->data)));
  if (new->data == NULL) {
    free(new);
    return NULL;
  }
  new->state = malloc(sizeof(keccak_state));
  if (new->state == NULL) {
    free(new->data);
    free(new);
    return NULL;
  }

  shake128_absorb_once(new->state, seed, ALEA_SEED_SIZE_SHAKE128);
  shake128_squeezeblocks(new->data, 1, new->state);

  return new;
}

static alea_state *alea_init_builtin_SHAKE256(const uint8_t *seed) {
  alea_state *new = malloc(sizeof(alea_state));
  if (new == NULL)
    return NULL;

  new->algorithm = ALEA_ALGORITHM_SHAKE256;
  new->len = SHAKE256_RATE;
  new->loc = 0;
  new->data = malloc(new->len * sizeof(*(new->data)));
  if (new->data == NULL) {
    free(new);
    return NULL;
  }
  new->state = malloc(sizeof(keccak_state));
  if (new->state == NULL) {
    free(new->data);
    free(new);
    return NULL;
  }

  shake256_absorb_once(new->state, seed, ALEA_SEED_SIZE_SHAKE256);
  shake256_squeezeblocks(new->data, 1, new->state);

  return new;
}

alea_state *alea_init_builtin(const uint8_t *const seed,
                              const alea_algo algorithm) {
  if (algorithm == ALEA_ALGORITHM_SHAKE128)
    return alea_init_builtin_SHAKE128(seed);
  else if (algorithm == ALEA_ALGORITHM_SHAKE256)
    return alea_init_builtin_SHAKE256(seed);

  return NULL;
}

alea_return alea_free_builtin(alea_state *state) {
  safe_free(state->data, state->len);
  safe_free(state->state, sizeof(keccak_state));
  safe_free(state, sizeof(alea_state));

  return ALEA_RETURN_OK;
}

alea_return alea_reseed_builtin(alea_state *state, const uint8_t *const seed) {
  if (state->algorithm == ALEA_ALGORITHM_SHAKE128) {
    shake128_absorb_once(state->state, seed, ALEA_SEED_SIZE_SHAKE128);
    shake128_squeezeblocks(state->data, 1, state->state);
    state->loc = 0;
  } else if (state->algorithm == ALEA_ALGORITHM_SHAKE256) {
    shake256_absorb_once(state->state, seed, ALEA_SEED_SIZE_SHAKE256);
    shake256_squeezeblocks(state->data, 1, state->state);
    state->loc = 0;
  }

  return ALEA_RETURN_OK;
}

static void resqueeze(alea_state *state) {
  if (state->algorithm == ALEA_ALGORITHM_SHAKE128) {
    shake128_squeezeblocks(state->data, 1, state->state);
    state->loc = 0;
  } else if (state->algorithm == ALEA_ALGORITHM_SHAKE256) {
    shake256_squeezeblocks(state->data, 1, state->state);
    state->loc = 0;
  }
}

alea_return alea_get_random_bytes_builtin(alea_state *state, uint8_t *const dst,
                                          const size_t dst_len) {

  if (state->loc == state->len) {
    resqueeze(state);
  }

  if (state->loc + dst_len <= state->len) {
    memcpy(dst, state->data + state->loc, dst_len);
    state->loc += dst_len;
    return ALEA_RETURN_OK;
  }

  const size_t diff = state->len - state->loc;
  memcpy(dst, state->data + state->loc, diff);
  state->loc = state->len;

  return alea_get_random_bytes_builtin(state, dst + diff, dst_len - diff);
}
