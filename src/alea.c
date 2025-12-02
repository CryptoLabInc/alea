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
#include "alea-hkdf.h"
#include "alea-internal.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined ALEA_BUILTIN

#include "alea-builtin.h"

alea_state *alea_init(const uint8_t *const seed, const alea_algo algorithm) {
  return alea_init_builtin(seed, algorithm);
}

alea_return alea_free(alea_state *state) { return alea_free_builtin(state); }

alea_return alea_reseed(alea_state *state, const uint8_t *const seed) {
  return alea_reseed_builtin(state, seed);
}

alea_return alea_get_random_bytes(alea_state *state, uint8_t *const dst,
                                  const size_t dst_len) {
  return alea_get_random_bytes_builtin(state, dst, dst_len);
}

#else
#error "Not supported"
#endif

uint64_t alea_get_random_uint64(alea_state *state) {
  uint64_t res;
  alea_get_random_bytes(state, (uint8_t *)&res, sizeof(res));

  return res;
}

uint32_t alea_get_random_uint32(alea_state *state) {
  uint32_t res;
  alea_get_random_bytes(state, (uint8_t *)&res, sizeof(res));

  return res;
}

uint64_t alea_get_random_uint64_in_range(alea_state *state,
                                         const uint64_t range) {
  assert(range >= 2);

  uint64_t res;
  // min = 2^64 % range = (2^64 - range) % range
  const uint64_t min = (-range) % range;

  while (1) {
    res = alea_get_random_uint64(state);
    if (res >= min)
      break;
  }

  return res % range;
}

uint32_t alea_get_random_uint32_in_range(alea_state *state,
                                         const uint32_t range) {
  assert(range >= 2);

  uint32_t res;
  // min = 2^32 % range = (2^32 - range) % range
  const uint32_t min = (-range) % range;

  while (1) {
    res = alea_get_random_uint32(state);
    if (res >= min)
      break;
  }

  return res % range;
}

alea_return alea_get_random_uint64_array(alea_state *state, uint64_t *const dst,
                                         const size_t dst_len) {
  return alea_get_random_bytes(state, (uint8_t *)dst,
                               dst_len * sizeof(uint64_t));
}

alea_return alea_get_random_uint32_array(alea_state *state, uint32_t *const dst,
                                         const size_t dst_len) {
  return alea_get_random_bytes(state, (uint8_t *)dst,
                               dst_len * sizeof(uint32_t));
}

alea_return alea_get_random_uint64_array_in_range(alea_state *state,
                                                  uint64_t *const dst,
                                                  const size_t dst_len,
                                                  const uint64_t range) {
  assert(range >= 2);

  // min = 2^64 % range = (2^64 - range) % range
  const uint64_t min = (-range) % range;

  uint64_t *it = dst;
  while (it != dst + dst_len) {
    *it = alea_get_random_uint64(state);
    if (*it >= min) {
      *it %= range;
      it++;
    }
  }

  return ALEA_RETURN_OK;
}

alea_return alea_get_random_uint32_array_in_range(alea_state *state,
                                                  uint32_t *const dst,
                                                  const size_t dst_len,
                                                  const uint32_t range) {
  assert(range >= 2);

  // min = 2^32 % range = (2^32 - range) % range
  const uint32_t min = (-range) % range;

  uint32_t *it = dst;
  while (it != dst + dst_len) {
    *it = alea_get_random_uint32(state);
    if (*it >= min) {
      *it %= range;
      it++;
    }
  }

  return ALEA_RETURN_OK;
}

// See the paper: Efficient isochronous fixed-weight sampling with applications
// to NTRU (https://eprint.iacr.org/2024/548) for more details on fixed-weight
// sampling
inline static alea_return alea_rejection_sampling_mod(alea_state *state,
                                                      int32_t *const si,
                                                      const size_t dst_len) {
  assert(dst_len > 0);
  // Choosing L involves a trade-off between the cost of generating random
  // numbers and the rate of sample rejections. A larger L reduces the rejection
  // probability and thus improves overall sampling efficiency. Conversely, an
  // excessively large L—or a too small L that triggers frequent
  // rejections—increases random-number generation overhead and can introduce
  // timing side-channels.
  //
  // The author choose L = 16, for n = 509, 677 and 821.
  // We choose L = 30, under our assumption that the input `dst_len` is less
  // than or equal to 2^17.
  uint64_t L = 30;
  uint64_t two_to_L = UINT32_C(1) << L;
  assert(dst_len <= two_to_L);

  memset(si, 0, dst_len * sizeof(int32_t));
  uint64_t n = (uint64_t)dst_len;
  uint64_t rnd, m, l, t, s;

  for (uint64_t i = 0; i < n - 1; i++) {
    s = n - 1 - i;
    t = two_to_L % s;
    // Unbiased uniform sampling from [0, n - 1 - i)
    // Rejection sampling ensures that ⌊x · s/2L⌋ is unbiasedly
    // sampled from [0, s)
    do {
      // uniform sampling from [0, 2^L - 1]
      alea_get_random_bytes(state, (uint8_t *)&rnd, (L + 7) / 8);
      rnd &= (two_to_L - 1);
      m = rnd * s;
      l = m & (two_to_L - 1);
    } while (l < t);
    si[i] = (int)(m >> L);
  }

  return ALEA_RETURN_OK;
}

alea_return alea_sample_hwt_int64_array(alea_state *state, int64_t *const dst,
                                        const size_t dst_len, const int hwt) {
  assert(hwt > 0);

  memset(dst, 0, dst_len * sizeof(int64_t));

  int32_t *si = malloc(dst_len * sizeof(int32_t));
  if (si == NULL) {
    return ALEA_RETURN_BAD_MALLOC_FAILURE;
  }
  alea_rejection_sampling_mod(state, si, dst_len);

  int c0 = (int)dst_len - hwt;
  int t0;
  uint8_t rnd;
  for (size_t i = 0; i < dst_len; i++) {
    t0 = -(si[i] < c0); // (si[i] - c0 >= 0) ? 0 : -1
    c0 += t0;
    dst[i] = 1 + t0;
    alea_get_random_bytes(state, &rnd, 1);
    dst[i] = (-dst[i]) & (1 - ((rnd & 1) << 1));
  }

  safe_free(si, dst_len * sizeof(int32_t));
  return ALEA_RETURN_OK;
}

alea_return alea_sample_hwt_int32_array(alea_state *state, int32_t *const dst,
                                        const size_t dst_len, const int hwt) {
  assert(hwt > 0);

  memset(dst, 0, dst_len * sizeof(int32_t));

  int32_t *si = malloc(dst_len * sizeof(int32_t));
  if (si == NULL) {
    return ALEA_RETURN_BAD_MALLOC_FAILURE;
  }
  alea_rejection_sampling_mod(state, si, dst_len);

  int c0 = (int)dst_len - hwt;
  int t0;
  uint8_t rnd;
  for (size_t i = 0; i < dst_len; i++) {
    t0 = -(si[i] < c0); // (si[i] - c0 >= 0) ? 0 : -1
    c0 += t0;
    dst[i] = 1 + t0;
    alea_get_random_bytes(state, &rnd, 1);
    dst[i] = (-dst[i]) & (1 - ((rnd & 1) << 1));
  }

  safe_free(si, dst_len * sizeof(int32_t));
  return ALEA_RETURN_OK;
}

alea_return alea_sample_hwt_int8_array(alea_state *state, int8_t *const dst,
                                       const size_t dst_len, const int hwt) {
  assert(hwt > 0);

  memset(dst, 0, dst_len * sizeof(int8_t));

  int32_t *si = malloc(dst_len * sizeof(int32_t));
  if (si == NULL) {
    return ALEA_RETURN_BAD_MALLOC_FAILURE;
  }
  alea_rejection_sampling_mod(state, si, dst_len);

  int c0 = (int)dst_len - hwt;
  int t0;
  uint8_t rnd;
  for (size_t i = 0; i < dst_len; i++) {
    t0 = -(si[i] < c0); // (si[i] - c0 >= 0) ? 0 : -1
    c0 += t0;
    dst[i] = (int8_t)(1 + t0);
    alea_get_random_bytes(state, &rnd, 1);
    dst[i] = (-dst[i]) & (1 - ((rnd & 1) << 1));
  }

  safe_free(si, dst_len * sizeof(int32_t));
  return ALEA_RETURN_OK;
}

#ifndef __has_builtin
#define __has_builtin(arg) 0
#endif
inline static int32_t alea_popcount(uint64_t x) {
#if __has_builtin(__builtin_popcountll)
  return __builtin_popcountll(x);
#elif defined(_MSC_VER)
  return (int32_t)__popcnt64(x);
#else
#define BITWIDTH sizeof(uint64_t) * 8 // in bits
  int32_t count = 0;
  for (int i = 0; i < BITWIDTH; i++) {
    count += x & 1;
    x >>= 1;
  }
  return count;
#undef BITWIDTH
#endif
}

alea_return alea_sample_cbd_int64_array(alea_state *state, int64_t *const dst,
                                        const size_t dst_len,
                                        const size_t cbd_num_flips) {
  const uint64_t CBD_MASK = (UINT64_C(1) << cbd_num_flips) - 1;
  const uint64_t CBD_NUM_FLIPS_BYTES =
      (cbd_num_flips + 7) / 8; // Round up to nearest byte
  uint64_t rnd1 = 0, rnd2 = 0;
  for (size_t i = 0; i < dst_len; i++) {
    alea_get_random_bytes(state, (uint8_t *)&rnd1, CBD_NUM_FLIPS_BYTES);
    alea_get_random_bytes(state, (uint8_t *)&rnd2, CBD_NUM_FLIPS_BYTES);
    rnd1 &= CBD_MASK;
    rnd2 &= CBD_MASK;
    dst[i] = alea_popcount(rnd1) - alea_popcount(rnd2);
  }

  return ALEA_RETURN_OK;
}

alea_return alea_sample_cbd_int32_array(alea_state *state, int32_t *const dst,
                                        const size_t dst_len,
                                        const size_t cbd_num_flips) {
  const uint64_t CBD_MASK = (UINT64_C(1) << cbd_num_flips) - 1;
  const uint64_t CBD_NUM_FLIPS_BYTES =
      (cbd_num_flips + 7) / 8; // Round up to nearest byte
  uint64_t rnd1 = 0, rnd2 = 0;
  for (size_t i = 0; i < dst_len; i++) {
    alea_get_random_bytes(state, (uint8_t *)&rnd1, CBD_NUM_FLIPS_BYTES);
    alea_get_random_bytes(state, (uint8_t *)&rnd2, CBD_NUM_FLIPS_BYTES);
    rnd1 &= CBD_MASK;
    rnd2 &= CBD_MASK;
    dst[i] = alea_popcount(rnd1) - alea_popcount(rnd2);
  }

  return ALEA_RETURN_OK;
}

#define ALEA_TWO_PI 6.28318530717958647692

alea_return alea_sample_gaussian_int64_array(alea_state *state,
                                             int64_t *const dst,
                                             const size_t dst_len,
                                             const double stdev) {
  assert(dst_len % 2 == 0);

  for (size_t i = 0; i < dst_len; i += 2) {
    const uint64_t rnd = alea_get_random_uint64(state);

    // Box-Muller Transform
    const uint64_t rn1 = rnd >> 32;
    const uint64_t rn2 = rnd & UINT64_C(0xFFFFFFFF);
    const double r1 = (double)rn1 / 4294967296.; // 2^32 = 4294967296
    const double r2 = ((double)rn2 + 1.0) / 4294967296.;
    const double theta = r1 * ALEA_TWO_PI;
    const double rr = sqrt(-2.0 * log(r2)) * stdev;

    dst[i] = llround(rr * cos(theta));
    dst[i + 1] = llround(rr * sin(theta));
  }

  return ALEA_RETURN_OK;
}

alea_return alea_sample_gaussian_int32_array(alea_state *state,
                                             int32_t *const dst,
                                             const size_t dst_len,
                                             const double stdev) {
  assert(dst_len % 2 == 0);

  for (size_t i = 0; i < dst_len; i += 2) {
    const uint64_t rnd = alea_get_random_uint64(state);

    // Box-Muller Transform
    const uint64_t rn1 = rnd >> 32;
    const uint64_t rn2 = rnd & UINT64_C(0xFFFFFFFF);
    const double r1 = (double)rn1 / 4294967296.; // 2^32 = 4294967296
    const double r2 = ((double)rn2 + 1.0) / 4294967296.;
    const double theta = r1 * ALEA_TWO_PI;
    const double rr = sqrt(-2.0 * log(r2)) * stdev;

    dst[i] = (int32_t)lround(rr * cos(theta));
    dst[i + 1] = (int32_t)lround(rr * sin(theta));
  }

  return ALEA_RETURN_OK;
}

alea_return alea_hkdf(const uint8_t *ikm, size_t ikm_len, const uint8_t *salt,
                      size_t salt_len, const uint8_t *info, size_t info_len,
                      uint8_t *okm, size_t okm_len) {
  assert(okm_len <= 8160); // 255 * SHA3_256_OUTLEN = 255 * 32 = 8160

  hkdf(ikm, ikm_len, salt, salt_len, info, info_len, okm, okm_len);
  return ALEA_RETURN_OK;
}

#undef ALEA_TWO_PI
