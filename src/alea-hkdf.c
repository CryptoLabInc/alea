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

#include "alea-hkdf.h"
#include "fips202.h"

#include <stdlib.h>
#include <string.h>

#define SHA3_256_OUTLEN 32
#define SHA3_256_BLOCK_SIZE SHA3_256_RATE // 136

void hmac_sha3_256(const uint8_t *key, size_t key_len, const uint8_t *data,
                   size_t data_len, uint8_t *out) {
  uint8_t k_ipad[SHA3_256_BLOCK_SIZE];
  uint8_t k_opad[SHA3_256_BLOCK_SIZE];
  uint8_t key_pad[SHA3_256_BLOCK_SIZE];
  uint8_t temp[SHA3_256_OUTLEN];
  size_t i;

  // TODO: Can be optimized by hashing only if key_len > SHA3_256_BLOCK_SIZE
  // if (key_len > SHA3_256_BLOCK_SIZE) {
  //    sha3_256(key_pad, key, key_len);
  //    memset(key_pad + SHA3_256_OUTLEN, 0, SHA3_256_BLOCK_SIZE -
  //    SHA3_256_OUTLEN);
  //} else {
  //    memcpy(key_pad, key, key_len);
  //    memset(key_pad + key_len, 0, SHA3_256_BLOCK_SIZE - key_len);
  //}
  sha3_256(key_pad, key, key_len);
  memset(key_pad + SHA3_256_OUTLEN, 0, SHA3_256_BLOCK_SIZE - SHA3_256_OUTLEN);

  for (i = 0; i < SHA3_256_BLOCK_SIZE; i++) {
    k_ipad[i] = key_pad[i] ^ 0x36;
    k_opad[i] = key_pad[i] ^ 0x5c;
  }

  // inner hash
  uint8_t *inner = malloc(SHA3_256_BLOCK_SIZE + data_len);
  memcpy(inner, k_ipad, SHA3_256_BLOCK_SIZE);
  memcpy(inner + SHA3_256_BLOCK_SIZE, data, data_len);
  sha3_256(temp, inner, SHA3_256_BLOCK_SIZE + data_len);
  free(inner);

  // outer hash
  uint8_t outer[SHA3_256_BLOCK_SIZE + SHA3_256_OUTLEN];
  memcpy(outer, k_opad, SHA3_256_BLOCK_SIZE);
  memcpy(outer + SHA3_256_BLOCK_SIZE, temp, SHA3_256_OUTLEN);
  sha3_256(out, outer, SHA3_256_BLOCK_SIZE + SHA3_256_OUTLEN);
}

void hkdf_extract(const uint8_t *ikm, size_t ikm_len, const uint8_t *salt,
                  size_t salt_len, uint8_t *prk) {
  hmac_sha3_256(salt, salt_len, ikm, ikm_len, prk);
}

void hkdf_expand(const uint8_t *prk, size_t prk_len, const uint8_t *info,
                 size_t info_len, uint8_t *okm, size_t okm_len) {
  size_t hash_len = SHA3_256_OUTLEN; // SHA3-256 output size
  size_t n = (okm_len + hash_len - 1) / hash_len;
  uint8_t t[SHA3_256_OUTLEN];
  uint8_t prev[SHA3_256_OUTLEN];
  size_t pos = 0;
  uint8_t *tmp = malloc(SHA3_256_OUTLEN + info_len + 1);
  for (size_t i = 1; i <= n; ++i) {
    // Concatenate: prev | info | counter
    size_t tmplen = 0;
    if (i != 1) {
      memcpy(tmp, prev, hash_len);
      tmplen += hash_len;
    }
    memcpy(tmp + tmplen, info, info_len);
    tmplen += info_len;
    tmp[tmplen] = (uint8_t)i;
    tmplen += 1;

    hmac_sha3_256(prk, prk_len, tmp, tmplen, t);
    memcpy(prev, t, hash_len);

    size_t copylen = (pos + hash_len > okm_len) ? (okm_len - pos) : hash_len;
    memcpy(okm + pos, t, copylen);
    pos += copylen;
  }
  free(tmp);
}

void hkdf(const uint8_t *ikm, size_t ikm_len, const uint8_t *salt,
          size_t salt_len, const uint8_t *info, size_t info_len, uint8_t *okm,
          size_t okm_len) {
  uint8_t prk[SHA3_256_OUTLEN];
  hkdf_extract(ikm, ikm_len, salt, salt_len, prk);
  hkdf_expand(prk, sizeof(prk), info, info_len, okm, okm_len);
}
