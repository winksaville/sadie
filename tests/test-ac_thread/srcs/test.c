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
#include <ac_printf.h>
#include <ac_receptor.h>
#include <ac_test.h>
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

  ac_receptor_signal(params->done, AC_TRUE);

  //ac_printf("wait_ns:-time=%ld ticks=%ld\n", params->time, params->stop - params->start);
  return AC_NULL;
}

void* wait_ticks(void* p) {
  waiter_params_t* params = (waiter_params_t*)p;
  //ac_printf("wait_ticks:+time=%ld\n", params->time);

  params->start = ac_tscrd();
  ac_thread_wait_ticks(params->time);
  params->stop = ac_tscrd();

  ac_receptor_signal(params->done, AC_TRUE);

  //ac_printf("wait_ticks:-time=%ld ticks=%ld\n", params->time, params->stop - params->start);
  return AC_NULL;
}

ac_bool test_thread_wait(void) {
  ac_bool error = AC_FALSE;
  ac_printf("test_thread_wait:+\n");

  waiter_params_t params;

  // Test ac_thread_wait_ns
  #define TRIES 10
  #define REQUIRED_SUCCESSES 7
  ac_u64 ticks;
  ac_u64 timems;
  ac_u64 waiting_ticks = 0;
  ac_uint successes = 0;
  ac_u64 ac_receptor_waiting_ticks = 0;
  ac_uint ac_receptor_waiting_successes = 0;
  for (ac_uint tries = TRIES; tries > 0; tries--) {
    params.time = 1000000;
    params.start = 0;
    params.stop = 0;

    params.done = ac_receptor_create(AC_FALSE);
    error |= AC_TEST(params.done != AC_NULL);

    ac_thread_rslt_t trslt = ac_thread_create(0, wait_ns, (void*)&params);
    error |= AC_TEST(trslt.status == 0);

    // Wait until waiting is done.
    ac_u64 start = ac_tscrd();
    ac_uint rslt = ac_receptor_wait(params.done);
    ac_u64 stop = ac_tscrd();
    error |= AC_TEST(rslt == 0);

    // Partial calcuation of avg
    ticks = stop - start;
    timems = AC_U64_DIV_ROUND_UP(ticks * 1000ll, ac_tsc_freq());
    if (AC_TRUE) { //(timems == 1) {
      ac_receptor_waiting_ticks += ticks;
      ac_receptor_waiting_successes += 1;
    }

    ticks = params.stop - params.start;
    timems = AC_U64_DIV_ROUND_UP(ticks * 1000ll, ac_tsc_freq());
    if (AC_TRUE) { //(timems == 1) {
      waiting_ticks += ticks;
      successes += 1;
    }
    ac_printf("test_thread_wait: waiting ns time=%lld ticks=%ld timems=%ld successes=%d\n",
        params.time, ticks, timems, successes);

    ac_receptor_destroy(params.done);
  }
  ac_printf("test_thread_wait: waiting ns avg    ticks=%ld ac_receptor_wait avg ticks=%ld\n",
      AC_U64_DIV_ROUND_UP(waiting_ticks, successes == 0 ? 1 : successes),
      AC_U64_DIV_ROUND_UP(ac_receptor_waiting_ticks,
        ac_receptor_waiting_successes == 0 ? 1 : ac_receptor_waiting_successes));
  // We have to allow some failures as we can't demand 100% success
  // rate because on Linux the time stamp counts are not synchronized
  // across CPU's.
  error |= AC_TEST(successes >= REQUIRED_SUCCESSES);
  error |= AC_TEST(ac_receptor_waiting_successes >= REQUIRED_SUCCESSES);

  // Test ac_thread_wait_ticks
  // Wait one millisecond
  successes = 0;
  waiting_ticks = 0;
  ac_receptor_waiting_successes = 0;
  ac_receptor_waiting_ticks = 0;
  for (ac_uint tries = TRIES; tries > 0; tries--) {
    params.time = AC_U64_DIV_ROUND_UP(ac_tsc_freq(), 1000ll);
    params.start = 0;
    params.stop = 0;

    params.done = ac_receptor_create(AC_FALSE);
    error |= AC_TEST(params.done != AC_NULL);

    ac_thread_rslt_t trslt = ac_thread_create(0, wait_ticks, (void*)&params);
    error |= AC_TEST(trslt.status == 0);

    // Wait until waiting is done.
    ac_u64 start = ac_tscrd();
    ac_uint rslt = ac_receptor_wait(params.done);
    ac_u64 stop = ac_tscrd();
    error |= AC_TEST(rslt == 0);

    // Partial calcuation of avg
    ticks = stop - start;
    timems = AC_U64_DIV_ROUND_UP(ticks * 1000ll, ac_tsc_freq());
    if (AC_TRUE) { //(timems == 1) {
      ac_receptor_waiting_ticks += ticks;
      ac_receptor_waiting_successes += 1;
    }

    ticks = params.stop - params.start;
    timems = AC_U64_DIV_ROUND_UP(ticks * 1000ll, ac_tsc_freq());
    if (AC_TRUE) { //(timems == 1) {
      waiting_ticks += ticks;
      successes += 1;
    }
    ac_printf("test_thread_wait: waiting ticks time=%ld ticks=%ld timems=%ld successes=%d\n",
        params.time, ticks, timems, successes);

    ac_receptor_destroy(params.done);
  }
  ac_printf("test_thread_wait: waiting ticks avg ticks=%ld ac_receptor_wait avg ticks=%ld\n",
      AC_U64_DIV_ROUND_UP(waiting_ticks, successes == 0 ? 1 : successes),
      AC_U64_DIV_ROUND_UP(ac_receptor_waiting_ticks,
        ac_receptor_waiting_successes == 0 ? 1 : ac_receptor_waiting_successes));
  // We have to allow some failures as we can't demand 100% success
  // rate because on Linux the time stamp counts are not synchronized
  // across CPU's.
  error |= AC_TEST(successes >= REQUIRED_SUCCESSES);
  error |= AC_TEST(ac_receptor_waiting_successes >= REQUIRED_SUCCESSES);

  ac_printf("test_thread_wait:-error=%d\n", error);
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

  // Time a one yield loop
  py.loops = 1000000;
  ac_printf("py: timed loops=%ld\n", py.loops);

  yield_loop(py.loops, &py.start, &py.stop);

  ac_printf("py: one thread 0 = %ld %ld - %ld\n",
      py.stop - py.start, py.stop, py.start);

  // Time  two threads running the yield loop
  __atomic_store_n(&py.done, AC_FALSE, __ATOMIC_RELEASE);

  ac_thread_rslt_t rslt = ac_thread_create(0, yt, (void*)&py);
  error |= AC_TEST(rslt.status == 0);

  ac_u64 two_thread_start = ac_tscrd();

  ac_u64 start, stop;
  yield_loop(py.loops, &start, &stop);

  while (__atomic_load_n(&py.done, __ATOMIC_ACQUIRE) == AC_FALSE) {
    ac_thread_yield();
  }

  ac_u64 two_thread_stop = ac_tscrd();

  ac_printf("py: two thread 0 = %ld %ld - %ld\n",
      stop - start, stop, start);
  ac_printf("               1 = %ld %ld - %ld\n",
      py.stop - py.start, py.stop, py.start);
  ac_printf("           total = %ld %ld - %ld\n",
      two_thread_stop - two_thread_start, two_thread_stop, two_thread_start);

  __atomic_store_n(&py.done, AC_FALSE, __ATOMIC_RELEASE);

  // Give other threads one last slice to finish
  ac_thread_yield();

  ac_printf("py:-error=%d\n", error);
  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

#ifdef VersatilePB
  ac_printf("py: threading not working on VersatilePB, skip\n");
#else
  ac_thread_init(32);
  ac_receptor_init(256);

  error |= test_simple();
  error |= test_thread_wait();
  error |= perf_yield();
#endif

  if (!error) {
    // Succeeded
    ac_printf("OK\n");
  }

  return error;
}

