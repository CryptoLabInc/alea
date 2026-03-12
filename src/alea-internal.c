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

/* Request C11 Annex K interfaces (memset_s) before any standard header. */
#if !defined(__STDC_WANT_LIB_EXT1__)
#define __STDC_WANT_LIB_EXT1__ 1
#endif

#include "alea-internal.h"

#ifdef _MSC_VER
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE __attribute__((noinline))
#endif

/* Compile-time detection of secure memory-zeroing primitive.
 * Priority:
 *  1. explicit_bzero  – glibc >= 2.25, OpenBSD, FreeBSD >= 11, NetBSD, macOS
 *  2. SecureZeroMemory – Windows
 *  3. memset_s        – C11 Annex K (implementation defines __STDC_LIB_EXT1__)
 *  4. volatile loop   – fallback
 *
 * On glibc without _GNU_SOURCE, string.h does not declare explicit_bzero even
 * though the symbol exists in libc; supply a forward declaration in that case.
 */
#if defined(__GLIBC__) &&                                                      \
    (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 25))
#define ALEA_HAVE_EXPLICIT_BZERO 1
#ifndef _GNU_SOURCE
extern void explicit_bzero(void *, size_t);
#endif
#elif defined(__OpenBSD__) || (defined(__FreeBSD__) && __FreeBSD__ >= 11) ||   \
    defined(__NetBSD__) || defined(__APPLE__)
#define ALEA_HAVE_EXPLICIT_BZERO 1
#elif defined(_WIN32)
#include <windows.h>
#define ALEA_HAVE_SECURE_ZERO_MEMORY 1
#elif defined(__STDC_LIB_EXT1__)
#define ALEA_HAVE_MEMSET_S 1
#endif

NOINLINE void safe_memzero(void *ptr, size_t ptr_len) {
  if (ptr == NULL || ptr_len == 0) {
    return;
  }

#if defined(ALEA_HAVE_EXPLICIT_BZERO)
  explicit_bzero(ptr, ptr_len);
#elif defined(ALEA_HAVE_SECURE_ZERO_MEMORY)
  SecureZeroMemory(ptr, ptr_len);
#elif defined(ALEA_HAVE_MEMSET_S)
  memset_s(ptr, ptr_len, 0, ptr_len);
#else
  /* volatile fallback */
  volatile unsigned char *p = (volatile unsigned char *)ptr;
  while (ptr_len--) {
    *p++ = 0;
  }
#endif
}
