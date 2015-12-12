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

#define NDEBUG

#include <ac_thread.h>

#include <ac_debug_printf.h>
#include <ac_test.h>

ac_u32 t1_count;

void* t1(void *param) {
  ac_u32 value = (ac_u32)(ac_uptr)param;
  ac_debug_printf("t1: param=%d\n", value);
  __atomic_add_fetch(&t1_count, value, __ATOMIC_RELEASE);
  return AC_NULL;
}

int main(void) {
  ac_bool error = AC_FALSE;
  ac_u32 t1_count_initial = -1;
  ac_u32 t1_count_increment = 2;
  ac_u32 result = t1_count_initial;

#if defined(VersatilePB)

  ac_printf("test_ac_thread: no tests for VersatilePB\n");
  //error = AC_TRUE;

#elif defined(Posix)

  ac_thread_init(1);

  t1_count = t1_count_initial;
  ac_u32 created = ac_thread_create(0, t1, (void*)(ac_uptr)t1_count_increment);
  error |= AC_TEST(created == 0);

  if (created == 0) {
    const ac_u32 max_loops = 1000000000;
    ac_u32 loops;
    for (loops = 0; loops < max_loops; loops++) {
      result = __atomic_load_n(&t1_count, __ATOMIC_ACQUIRE);
      if (result != t1_count_initial) {
        break;
      }
    }
    ac_debug_printf("test-ac_thread: loops=%d\n", loops);
    error |= AC_TEST(loops < max_loops);
    error |= AC_TEST(result != t1_count_initial);
    error |= AC_TEST(result == t1_count_initial + t1_count_increment);
  }

#else

  ac_printf("test_ac_thread: Unknown Platform\n");

#endif

  if (!error) {
    // Succeeded
    ac_printf("OK\n");
  }

  return error;
}

