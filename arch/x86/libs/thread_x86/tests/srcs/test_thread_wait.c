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
#include <ac_receptor.h>
#include <ac_tsc.h>
#include <ac_test.h>




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

  ac_receptor_signal(params->done, AC_FALSE);

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

/**
 * Test waiting
 */
ac_bool test_thread_wait(void) {
  ac_bool error = AC_FALSE;
  ac_printf("test_thread_wait:+flags=%x\n", get_flags());

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
    params.time = 1000000; // 1 millisecond
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
    if (timems == 1) {
      ac_receptor_waiting_ticks += ticks;
      ac_receptor_waiting_successes += 1;
    }

    ticks = params.stop - params.start;
    timems = AC_U64_DIV_ROUND_UP(ticks * 1000ll, ac_tsc_freq());
    if (timems == 1) {
      waiting_ticks += ticks;
      successes += 1;
    }
    ac_printf("test_thread_wait: waiting    ns=%lld ticks=%ld timems=%ld successes=%d\n",
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
    if (timems == 1) {
      ac_receptor_waiting_ticks += ticks;
      ac_receptor_waiting_successes += 1;
    }

    ticks = params.stop - params.start;
    timems = AC_U64_DIV_ROUND_UP(ticks * 1000ll, ac_tsc_freq());
    if (timems == 1) {
      waiting_ticks += ticks;
      successes += 1;
    }
    ac_printf("test_thread_wait: waiting ticks=%ld ticks=%ld timems=%ld successes=%d\n",
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

