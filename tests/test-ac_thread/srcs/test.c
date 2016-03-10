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

#include <ac_tsc.h>

#include <ac_printf.h>
#include <ac_test.h>

ac_u32 t1_count;

void* t1(void *param) {
  ac_u32 value = (ac_u32)(ac_uptr)param;
  ac_printf("t1: param=%d\n", value);
  __atomic_add_fetch(&t1_count, value, __ATOMIC_RELEASE);
  return AC_NULL;
}

ac_bool test_simple(void) {
  ac_bool error = AC_FALSE;
  ac_u32 t1_count_initial = -1;
  ac_u32 t1_count_increment = 2;
  ac_u32 result = t1_count_initial;

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
      } else {
        ac_thread_yield();
      }
    }
    ac_printf("test-ac_thread: loops=%d result=%d\n", loops, result);
    error |= AC_TEST(loops < max_loops);
    error |= AC_TEST(result == t1_count_initial + t1_count_increment);

    // Since we don't have a 'ac_thread_join' we're racing, so
    // this delay give greater assurance the tests works.
    // TODO: Add ac_thread_join although that means blocking
    // which I don't like so we'll see.
    for (int i = 0; i < 1000000; i++) {
      __atomic_load_n(&t1_count, __ATOMIC_ACQUIRE);
    }

    created = ac_thread_create(0, t1, (void*)(ac_uptr)t1_count_increment);
    error |= AC_TEST(created == 0);

    if (created == 0) {
      const ac_u32 max_loops = 1000000000;
      ac_u32 loops;
      for (loops = 0; loops < max_loops; loops++) {
        result = __atomic_load_n(&t1_count, __ATOMIC_ACQUIRE);
        if (result != t1_count_initial + t1_count_increment) {
          break;
        } else {
          ac_thread_yield();
        }
      }
      ac_printf("test-ac_thread: loops=%d result=%d\n", loops, result);
      error |= AC_TEST(loops < max_loops);
      error |= AC_TEST(result == t1_count_initial + (2 * t1_count_increment));
    }
  }

  // Give other threads one last slice to finish
  ac_thread_yield();

  return error;
}

typedef struct {
  ac_u64 loops;
  ac_u64 first;
  ac_u64 second;
  ac_bool done;
} perf_yield_t;


void* yt(void *param) {
  perf_yield_t* py = (perf_yield_t*)param;
  ac_u64 loops = py->loops;
  ac_printf("yt: loops=%ld\n", loops);

  ac_u64 first = ac_tscrd();
  for(ac_u64 i = 0; i < loops; i++) {
    ac_thread_yield();
  }
  ac_u64 second = ac_tscrd();

  py->first = first;
  py->second = second;

  __atomic_store_n(&py->done, AC_TRUE, __ATOMIC_RELEASE);

  while (__atomic_load_n(&py->done, __ATOMIC_ACQUIRE) == AC_TRUE) {
  }

  ac_printf("ty:-exiting loops=%ld\n", py->loops);

  return AC_NULL;
}

ac_bool perf_yield(void) {
  ac_printf("py:+\n");
  ac_bool error = AC_FALSE;
  perf_yield_t py;

  py.loops = 1000000;
  __atomic_store_n(&py.done, AC_FALSE, __ATOMIC_RELEASE);

  ac_u64 start = ac_tscrd();

  ac_u32 created = ac_thread_create(0, yt, (void*)&py);
  error |= AC_TEST(created == 0);

  ac_u64 loops = py.loops;
  ac_printf("py: loops=%ld\n", loops);

  ac_u64 first = ac_tscrd();
  for(ac_u64 i = 0; i < loops; i++) {
    ac_thread_yield();
  }
  ac_u64 second = ac_tscrd();
  ac_printf("py: done, waiting\n");

  while (__atomic_load_n(&py.done, __ATOMIC_ACQUIRE) == AC_FALSE) {
  }

  ac_u64 finish = ac_tscrd();

  ac_printf("py: finished py = %ld %ld - %ld\n",
      second - first, second, first);
  ac_printf("             yt = %ld %ld - %ld\n",
      py.second - py.first, py.second, py.first);
  ac_printf("          total = %ld %ld - %ld\n",
      finish - start, finish, start);

  __atomic_store_n(&py.done, AC_FALSE, __ATOMIC_RELEASE);

  ac_printf("py:-exiting loops=%ld\n", loops);

  // Give other threads one last slice to finish
  ac_thread_yield();

  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

#ifdef VersatilePB
  ac_printf("py: threading not working on VersatilePB, skip\n");
#else
  ac_thread_init(32);

  error |= test_simple();
  error |= perf_yield();
#endif

  if (!error) {
    // Succeeded
    ac_printf("OK\n");
  }

  return error;
}

