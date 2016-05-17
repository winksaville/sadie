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

#define NDEBUG

#include <ac_receptor.h>

#include <ac_assert.h>
#include <ac_debug_printf.h>
#include <ac_inttypes.h>
#include <ac_printf.h>
#include <ac_test.h>
#include <ac_thread.h>
#include <ac_time.h>
#include <ac_tsc.h>

ac_u32 t1_count;

struct test_params {
  ac_receptor_t receptor;
  ac_uint loops;
  ac_uint counter;
  ac_receptor_t done_receptor;
};

void* t1(void *param) {
  struct test_params* params = (struct test_params*)param;
  //ac_printf("t1:+params->loops=%d\n", params->loops);
  for(ac_uint i = 0; i < params->loops; i++) {
    __atomic_add_fetch(&params->counter, 1, __ATOMIC_RELEASE);

    //ac_thread_yield();

    ac_receptor_wait(params->receptor);
  }

  ac_debug_printf("t1: signal done_receptor\n");
  ac_receptor_signal(params->done_receptor, AC_FALSE);
  return AC_NULL;
}

ac_uint test_receptor(void) {
  ac_printf("test_receptor:+\n");

  ac_uint error = AC_FALSE;
#if defined(Posix) || defined(pc_x86_64)
  struct test_params params;

  ac_printf("test_receptor: call ac_receptor_create\n");
  params.receptor = ac_receptor_create(AC_FALSE);
  params.done_receptor = ac_receptor_create(AC_FALSE);
  params.counter = 0;
#ifdef NDEBUG
  params.loops = 1000000;
#else
  params.loops = 10;
  ac_uint x = 0;
#endif

  ac_thread_rslt_t rslt = ac_thread_create(0, t1, (void*)&params);
  error |= AC_TEST(rslt.status == 0);

  ac_u64 start = ac_tscrd();

  ac_printf("test_receptor: loops=%ld\n", params.loops);
  for (ac_uint i = 0; i < params.loops; i++) {
#ifdef NDEBUG
    // Wait for params.counter to change, i.e. t1 and incremented the counter
    // and is waiting for the signal
    while (__atomic_load_n(&params.counter, __ATOMIC_ACQUIRE) <= i) {
    }
    ac_receptor_signal(params.receptor, AC_TRUE);
#else
    // Wait for params.counter to change, i.e. t1 and incremented the counter
    // and is waiting for the signal
    while (__atomic_load_n(&params.counter, __ATOMIC_ACQUIRE) <= i) {
      if ((++x % 20000000) == 0) {
        ac_printf("test_receptor: waiting for counter i=%d x=%d\n", i, x);
      }

      //ac_printf("test_receptor: call ac_thread_yield\n");
      //ac_thread_yield();
    }
    ac_u64 wait_until = ac_tscrd() + (ac_tsc_freq() / 1);
    while (ac_tscrd() < wait_until) {
      if ((++x % 20000000) == 0) {
        ac_printf("test_receptor: wait until=%ld cur tsc=%ld i=%d x=%d\n", wait_until, ac_tscrd(), i, x);
      }
      //ac_thread_yield();
    }

    //ac_printf("test_receptor: signal\n");
    ac_u64 signal_start = ac_tscrd();
    ac_receptor_signal(params.receptor, AC_TRUE);
    ac_u64 ticks = ac_tscrd() - signal_start;
    ac_printf("test_receptor: signal time=%.9t\n", ticks);
#endif
  }
  ac_debug_printf("test_receptor: wait on done_receptor x=%d\n", x);
  ac_receptor_wait(params.done_receptor);
  ac_debug_printf("test_receptor: continuing done_receptor\n");

  ac_u64 stop = ac_tscrd();
  ac_u64 ticks = stop - start;
  ac_printf("test_receptor: ticks=%ld %ld - %ld\n", ticks, stop, start);

  ac_u64 ticks_per_op = AC_U64_DIV_ROUND_UP(ticks, params.loops);
  ac_printf("test_receptor: ticks_per_op=%ld(%.9t)\n", ticks_per_op, ticks_per_op);

  ac_printf("test_receptor: time=%.9t\n", ticks);

  ac_receptor_destroy(params.receptor);
#endif

  ac_printf("test_receptor:-error=%d\n", error);
  return error;
}

int main(void) {
  ac_uint error = AC_FALSE;

  ac_thread_init(8);
  ac_receptor_init(256);

  ac_time_init();

  if (!error) {
    error |= test_receptor();
  }

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
