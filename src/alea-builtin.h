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

#ifndef ALEA_ALEA_BUILTIN_H
#define ALEA_ALEA_BUILTIN_H

#include "alea/alea.h"

alea_state *alea_init_builtin(const uint8_t *const seed,
                              const alea_algo algorithm);
alea_return alea_free_builtin(alea_state *state);
alea_return alea_reseed_builtin(alea_state *state, const uint8_t *const seed);
alea_return alea_get_random_bytes_builtin(alea_state *state, uint8_t *const dst,
                                          const size_t dst_len);

#endif // ALEA_ALEA_BUILTIN_H
