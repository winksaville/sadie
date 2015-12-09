/*
 * Copyright 2015 Wink Saville
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

#include <ac_memmgr.h>

#include <ac_test.h>

int main(void) {
  ac_bool error = AC_FALSE;

  // Must return AC_NULL although according to C99 standard a
  // malloc(0) may return either but undefined behavior happens
  // if the pointer is used. Therefore we'll defined it as always
  // returning AC_NULL
  error |= AC_TEST(ac_malloc(0) == AC_NULL);

  // Test conditions which attempt allocate too much memory
  error |= AC_TEST(ac_malloc(((ac_size_t)0) - 1) == AC_NULL);

  error |= AC_TEST(ac_malloc(((ac_size_t)0) - 2) == AC_NULL);

  error |= AC_TEST(ac_malloc(((ac_size_t)0) - 63) == AC_NULL);

  error |= AC_TEST(ac_malloc(((ac_size_t)0) - 64) == AC_NULL);

  error |= AC_TEST(ac_malloc(((ac_size_t)0) - 65) == AC_NULL);

  error |= AC_TEST(ac_malloc(((ac_size_t)0) - 66) == AC_NULL);

  // Test conditions which must succeed as we expect at
  // least being able to do a few small allocations
  void* p1 = ac_malloc(1);
  error |= AC_TEST(ac_malloc(1) != AC_NULL);
  ac_free(p1);

  void* p2 = ac_malloc(2);
  error |= AC_TEST(p2 != AC_NULL);
  ac_free(p2);

  void* p63 = ac_malloc(63);
  error |= AC_TEST(p63 != AC_NULL);
  ac_free(p63);

  void* p64 = ac_malloc(64);
  error |= AC_TEST(p64 != AC_NULL);
  ac_free(p64);

  void* p65 = ac_malloc(1);
  error |= AC_TEST(p65 != AC_NULL);
  ac_free(p65);

  if (!error) {
    // Succeeded
    ac_printf("OK\n");
  }

  return error;
}

