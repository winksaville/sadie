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

#include <ac_intmath.h>
#include <ac_memmgr.h>
#include <ac_printf.h>
#include <ac_receptor.h>
#include <ac_test.h>
#include <ac_time.h>
#include <ac_tsc.h>

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
  ac_thread_rslt_t rslt = ac_thread_create(0, t1, (void*)(ac_uptr)t1_count_increment);
  error |= AC_TEST(rslt.status == 0);

  if (rslt.status == 0) {
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

    rslt = ac_thread_create(0, t1, (void*)(ac_uptr)t1_count_increment);
    error |= AC_TEST(rslt.status == 0);

    if (rslt.status == 0) {
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
  ac_u64 time;
  ac_u64 start;
  ac_u64 stop;
  ac_receptor_t done;
} waiter_params_t;

void* wait_ns(void* p) {
  waiter_params_t* params = (waiter_params_t*)p;
  //ac_printf("wait_ns:+nanosecs=%ld\n", params->time);

  params->start = ac_tscrd();
  ac_thread_wait_ns(params->time);
  params->stop = ac_tscrd();

  ac_receptor_signal_yield_if_waiting(params->done);

  //ac_printf("wait_ns:-time=%ld ticks=%ld\n", params->time, params->stop - params->start);
  return AC_NULL;
}

void* wait_ticks(void* p) {
  waiter_params_t* params = (waiter_params_t*)p;
  //ac_printf("wait_ticks:+time=%ld\n", params->time);

  params->start = ac_tscrd();
  ac_thread_wait_ticks(params->time);
  params->stop = ac_tscrd();

  ac_receptor_signal_yield_if_waiting(params->done);

  //ac_printf("wait_ticks:-time=%ld ticks=%ld\n", params->time, params->stop - params->start);
  return AC_NULL;
}

/**
 * Test having a specified number of threads waiting a period of time.
 *
 * @params simultaneous_threads is the number of threads
 * @params ns is true if to use thread_wait_ns and false for thread_wait_ticks
 */
ac_bool test_thread_wait(ac_uint simultaneous_threads, ac_bool ns,
    ac_uint num_tries, ac_uint required_successes) {
  ac_bool error = AC_FALSE;
  char* name = ns ? "ns" : "ticks";
  ac_printf("test_thread_wait(%d, %s):+\n", simultaneous_threads, name);

  waiter_params_t* params = ac_malloc(sizeof(waiter_params_t) * simultaneous_threads);

  ac_u64 ticks;
  ac_u64 timems;
  ac_u64 wait_timems;
  ac_u64 waiting_ticks = 0;
  ac_uint successes = 0;
  ac_u64 ac_receptor_waiting_ticks = 0;
  ac_uint ac_receptor_waiting_successes = 0;

  // Number of attempts
  successes = 0;
  waiting_ticks = 0;
  ac_receptor_waiting_successes = 0;
  ac_receptor_waiting_ticks = 0;
  for (ac_uint tries = num_tries; tries > 0; tries--) {
    wait_timems = 10;

    // Test the requested number threads waiting simultaneously
    for (ac_uint i = 0; i < simultaneous_threads; i++) {
      ac_u64 time;
      void* (*thread_entry)(void*) = AC_NULL;

      if (ns) {
        time = wait_timems * 1000000ll;
        thread_entry = wait_ns;
      } else {
        time = AC_U64_DIV_ROUND_UP(ac_tsc_freq(), (1000ll / wait_timems));
        thread_entry = wait_ticks;
      }

      params[i].time = time;
      params[i].start = 0;
      params[i].stop = 0;

      params[i].done = ac_receptor_create();
      error |= AC_TEST(params[0].done != AC_NULL);

      ac_thread_rslt_t trslt = ac_thread_create(0, thread_entry, (void*)&params[i]);
      error |= AC_TEST(trslt.status == 0);
    }

    // Wait until waiting is done.
    ac_u64 start = ac_tscrd();
    for (ac_uint i = 0; i < simultaneous_threads; i++) {
      ac_uint rslt = ac_receptor_wait(params[i].done);
      error |= AC_TEST(rslt == 0);
    }
    ac_u64 stop = ac_tscrd();

    // Partial calcuation of avg
    ticks = stop - start;
    timems = AC_U64_DIV_ROUND_UP(ticks * 1000ll, ac_tsc_freq());
    if (timems == wait_timems) {
      ac_receptor_waiting_ticks += ticks;
      ac_receptor_waiting_successes += 1;
    }

    for (ac_uint i = 0; i < simultaneous_threads; i++) {
      ticks = params[i].stop - params[i].start;
      timems = AC_U64_DIV_ROUND_UP(ticks * 1000ll, ac_tsc_freq());
      if (timems == wait_timems) {
        waiting_ticks += ticks;
        successes += 1;
      }
      ac_printf("test_thread_wait(%d, %s): %d waiting %s=%ld ticks=%ld timems=%ld successes=%d\n",
          simultaneous_threads, name, i, name, params[i].time, ticks, timems, successes);

      ac_receptor_destroy(params[i].done);
    }
  }
  ac_printf("test_thread_wait(%d, %s): waiting %s avg ticks=%ld ac_receptor_wait avg ticks=%ld\n",
      simultaneous_threads, name, name,
      AC_U64_DIV_ROUND_UP(waiting_ticks, successes == 0 ? 1 : successes),
      AC_U64_DIV_ROUND_UP(ac_receptor_waiting_ticks,
        ac_receptor_waiting_successes == 0 ? 1 : ac_receptor_waiting_successes));
  // We have to allow some failures as we can't demand 100% success
  // rate because on Linux the time stamp counts are not synchronized
  // across CPU's.
  error |= AC_TEST(successes >= required_successes);
  error |= AC_TEST(ac_receptor_waiting_successes >= required_successes);

  ac_free(params);

  ac_printf("test_thread_wait(%d, %s):-error=%d\n", simultaneous_threads, name, error);
  return error;
}


typedef struct {
  ac_u64 loops;
  ac_u64 start;
  ac_u64 stop;
  ac_bool done;
} perf_yield_t;


void yield_loop(ac_u64 loops, ac_u64* start, ac_u64* stop) {
  if (start != AC_NULL) *start = ac_tscrd();
  for(ac_u64 i = 0; i < loops; i++) {
    ac_thread_yield();
  }
  if (stop != AC_NULL) *stop = ac_tscrd();
}

void* yt(void *param) {
  perf_yield_t* py = (perf_yield_t*)param;

  yield_loop(py->loops, &py->start, &py->stop);

  __atomic_store_n(&py->done, AC_TRUE, __ATOMIC_RELEASE);

  while (__atomic_load_n(&py->done, __ATOMIC_ACQUIRE) == AC_TRUE) {
    ac_thread_yield();
  }

  return AC_NULL;
}

ac_bool perf_yield(void) {
  ac_printf("py:+\n");
  ac_bool error = AC_FALSE;
  perf_yield_t py;

  ac_u64 warm_up_loops = 1000000;

  // Warm up cpu
  ac_printf("py: warm up loops=%ld\n", warm_up_loops);
  yield_loop(warm_up_loops, AC_NULL, AC_NULL);

  // Time a one yield loop, i.e. the thread we've been called on
  py.loops = 1000000;
  ac_printf("py: timed loops=%ld\n", py.loops);

  yield_loop(py.loops, &py.start, &py.stop);

  ac_printf("py: one thread 0 = %.9t %ld %ld - %ld\n",
      py.stop - py.start, py.stop, py.start);

  // Time two threads, the thread we're on plus another we're creating
  __atomic_store_n(&py.done, AC_FALSE, __ATOMIC_RELEASE);

  ac_thread_rslt_t rslt = ac_thread_create(0, yt, (void*)&py);
  error |= AC_TEST(rslt.status == 0);

  ac_u64 two_thread_start = ac_tscrd();

  ac_u64 start, stop;
  yield_loop(py.loops, &start, &stop);

  // Wait while yt set's py.done is AC_FALSE
  while (__atomic_load_n(&py.done, __ATOMIC_ACQUIRE) == AC_FALSE) {
    ac_thread_yield();
  }

  ac_u64 two_thread_stop = ac_tscrd();

  ac_printf("py: two thread 0 = %.9t %ld %ld - %ld\n",
      stop - start, stop - start, stop, start);
  ac_printf("               1 = %.9t %ld %ld - %ld\n",
      py.stop - py.start, py.stop - py.start, py.stop, py.start);
  ac_printf("           total = %.9t %ld %ld - %ld\n",
      two_thread_stop - two_thread_start, two_thread_stop - two_thread_start,
      two_thread_stop, two_thread_start);

  // Tell yt we're done
  __atomic_store_n(&py.done, AC_FALSE, __ATOMIC_RELEASE);

  // Give other threads one last slice to finish
  ac_thread_yield();

  ac_printf("py:-error=%d\n", error);
  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

#if AC_PLATFORM == VersatilePB
  ac_printf("py: threading not working on VersatilePB, skip\n");
#else

  // Start with only one thread
  ac_thread_init(1);
  ac_receptor_init(256);
  ac_time_init();

  error |= test_simple();

  #define TRIES 10
  #define REQUIRED_SUCCESSES 7

  // Increate to 32 threads
  ac_thread_init(32);
 
  // Test using ac_thread_wait_ticks
  error |= test_thread_wait(1, AC_FALSE, TRIES, REQUIRED_SUCCESSES);
  error |= test_thread_wait(2, AC_FALSE, TRIES, REQUIRED_SUCCESSES);
  error |= test_thread_wait(3, AC_FALSE, TRIES, REQUIRED_SUCCESSES);
  error |= test_thread_wait(4, AC_FALSE, TRIES, REQUIRED_SUCCESSES);

  // Test using ac_thread_wait_ns
  error |= test_thread_wait(2, AC_TRUE, TRIES, REQUIRED_SUCCESSES);

  error |= perf_yield();
#endif

  if (!error) {
    // Succeeded
    ac_printf("OK\n");
  }

  return error;
}
