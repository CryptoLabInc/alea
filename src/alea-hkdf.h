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

#ifndef ALEA_ALEA_HKDF_H
#define ALEA_ALEA_HKDF_H

#include "alea/alea.h"

void hmac_sha3_256(const uint8_t *key, size_t key_len, const uint8_t *data,
                   size_t data_len, uint8_t *out);

void hkdf_extract(const uint8_t *ikm, size_t ikm_len, const uint8_t *salt,
                  size_t salt_len, uint8_t *prk);

void hkdf_expand(const uint8_t *prk, size_t prk_len, const uint8_t *info,
                 size_t info_len, uint8_t *okm, size_t okm_len);

void hkdf(const uint8_t *ikm, size_t ikm_len, const uint8_t *salt,
          size_t salt_len, const uint8_t *info, size_t info_len, uint8_t *okm,
          size_t okm_len);

#endif // ALEA_ALEA_HKDF_H
