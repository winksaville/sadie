/*
 * Copyright 2016 Wink Saville
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

#include <test.h>

#include <native_x86.h>
#include <thread_x86.h>

#include <ac_thread.h>
#include <ac_intmath.h>
#include <ac_inttypes.h>
#include <ac_memmgr.h>
#include <ac_receptor.h>
#include <ac_tsc.h>
#include <ac_test.h>


#define TRIES 10 
#define REQUIRED_SUCCESSES 7

typedef struct {
  ac_u64 time;
  ac_u64 start;
  ac_u64 stop;
  AcReceptor* done;
} waiter_params_t;

void* wait_ns(void* p) {
  waiter_params_t* params = (waiter_params_t*)p;
  //ac_printf("wait_ns:+nanosecs=%ld\n", params->time);

  params->start = ac_tscrd();
  ac_thread_wait_ns(params->time);
  params->stop = ac_tscrd();

  AcReceptor_signal(params->done);

  //ac_printf("wait_ns:-time=%ld ticks=%ld\n", params->time, params->stop - params->start);
  return AC_NULL;
}

void* wait_ticks(void* p) {
  waiter_params_t* params = (waiter_params_t*)p;
  //ac_printf("wait_ticks:+time=%ld\n", params->time);

  params->start = ac_tscrd();
  ac_thread_wait_ticks(params->time);
  params->stop = ac_tscrd();

  AcReceptor_signal_yield_if_waiting(params->done);

  //ac_printf("wait_ticks:-time=%ld ticks=%ld\n", params->time, params->stop - params->start);
  return AC_NULL;
}

/**
 * Test waiting tick on 1 or more threads.
 *
 * @params simultaneous_thread is the number of threads
 * @params ns is true if to use thread_wait_ns and false for thread_wait_ticks
 */
ac_bool test_thread_wait(ac_uint simultaneous_threads, ac_bool ns) {
  ac_bool error = AC_FALSE;
  char* name = ns ? "ns" : "ticks";
  ac_printf("test_thread_wait(%d, %s):+flags=%x\n", simultaneous_threads, name, get_flags());

  waiter_params_t* params = ac_malloc(sizeof(waiter_params_t) * simultaneous_threads);

  ac_u64 ticks;
  ac_u64 timems;
  ac_u64 wait_timems;
  ac_u64 waiting_ticks = 0;
  ac_uint successes = 0;
  ac_u64 AcReceptor_waiting_ticks = 0;
  ac_uint AcReceptor_waiting_successes = 0;

  // Test 2 threads ber of multiple threads waiting simultaneiously
  // since we need to test both left and right subtrees of the binary
  // heap.
  successes = 0;
  waiting_ticks = 0;
  AcReceptor_waiting_successes = 0;
  AcReceptor_waiting_ticks = 0;
  for (ac_uint tries = TRIES; tries > 0; tries--) {
    wait_timems = 10;

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

      params[i].done = AcReceptor_get();
      error |= AC_TEST(params[0].done != AC_NULL);

      ac_thread_rslt_t trslt = ac_thread_create(0, thread_entry, (void*)&params[i]);
      error |= AC_TEST(trslt.status == 0);
    }

    // Wait until waiting is done.
    ac_u64 start = ac_tscrd();
    for (ac_uint i = 0; i < simultaneous_threads; i++) {
      ac_uint rslt = AcReceptor_wait(params[i].done);
      error |= AC_TEST(rslt == 0);
    }
    ac_u64 stop = ac_tscrd();

    // Partial calcuation of avg
    ticks = stop - start;
    timems = AC_U64_DIV_ROUND_UP(ticks * 1000ll, ac_tsc_freq());
    if (timems == wait_timems) {
      AcReceptor_waiting_ticks += ticks;
      AcReceptor_waiting_successes += 1;
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

      AcReceptor_ret(params[i].done);
    }
  }
  ac_printf("test_thread_wait(%d, %s): waiting %s avg ticks=%ld AcReceptor_wait avg ticks=%ld\n",
      simultaneous_threads, name, name,
      AC_U64_DIV_ROUND_UP(waiting_ticks, successes == 0 ? 1 : successes),
      AC_U64_DIV_ROUND_UP(AcReceptor_waiting_ticks,
        AcReceptor_waiting_successes == 0 ? 1 : AcReceptor_waiting_successes));
  // We have to allow some failures as we can't demand 100% success
  // rate because on Linux the time stamp counts are not synchronized
  // across CPU's.
  error |= AC_TEST(successes >= REQUIRED_SUCCESSES);
  error |= AC_TEST(AcReceptor_waiting_successes >= REQUIRED_SUCCESSES);

  ac_uint zombies = remove_zombies();
  ac_printf("test_thread_wait(%d, %s): zombies removed=%d\n", simultaneous_threads, name, zombies);
  ac_free(params);

  ac_printf("test_thread_wait(%d, %s):-error=%d\n", simultaneous_threads, name, error);
  return error;
}

