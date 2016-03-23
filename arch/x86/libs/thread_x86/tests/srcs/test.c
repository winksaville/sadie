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

#include <thread_x86.h>

#include <apic_x86.h>
#include <cpuid_x86.h>
#include <interrupts_x86.h>
#include <msr_x86.h>

#include <ac_thread.h>
#include <ac_inttypes.h>
#include <ac_printf.h>
#include <ac_putchar.h>
#include <ac_test.h>
#include <ac_tsc.h>

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

ac_uint test_yield_no_other_threads(void) {
  ac_printf("test_yield_no_other_threads:+\n");
  ac_uint error = AC_FALSE;

  ac_thread_yield();

  ac_printf("test_yield_no_other_threads:-error=%d\n", error);
  return error;
}

volatile ac_u64 t0_counter;

void* t0(void* p) {
  AC_UNUSED(p);
  ac_printf("t0:+\n");

  __atomic_add_fetch(&t0_counter, 1, __ATOMIC_RELEASE);

  ac_printf("t0:-\n");

  return AC_NULL;
}

ac_uint test_yield_one_other_thread(void) {
  ac_uint error = AC_FALSE;
  ac_printf("test_yield_one_other_thread:+\n");

  error |= AC_TEST(ac_thread_create(AC_THREAD_STACK_MIN,
        t0, AC_NULL).status == 0);

  thread_x86_yield();

  error |= AC_TEST(t0_counter == 1);

  error |= AC_TEST(remove_zombies() == 1);

  ac_printf("test_yield_one_other_thread:-error=%d\n", error);
  return error;
}

volatile ac_u64 t1_counter;
volatile ac_u64 t1_done;

void* t1(void* p) {
  ac_uptr loops = (ac_u64)p;
  ac_u64 v = 0;
  ac_printf("t1:+loops=%d\n", loops);

  for (; loops != 0; loops--) {
    v = __atomic_add_fetch(&t1_counter, 1, __ATOMIC_RELEASE);
    //ac_printf("v=%ld loops=%d\n", v, loops);
    //ac_thread_yield();
  }

  ac_printf("t1:-v=%d\n", v);

  // The main loop needs to wait on this so we exit.
  // TODO: Add ac_thread_join.
  __atomic_store_n(&t1_done, AC_TRUE, __ATOMIC_RELEASE);

  // Wait until main joins then exit
  while(__atomic_load_n(&t1_done,  __ATOMIC_ACQUIRE) == AC_TRUE) {
    ;
  }

  return AC_NULL;
}

ac_uint test_ac_thread_create() {
  ac_printf("test_ac_thread_create:+\n");
  ac_uint error = AC_FALSE;

  __atomic_store_n(&t1_counter, 0, __ATOMIC_RELEASE);
  __atomic_store_n(&t1_done, AC_FALSE, __ATOMIC_RELEASE);

  ac_u64 expected_t1_counter = 1000000000;
  error |= AC_TEST(ac_thread_create(AC_THREAD_STACK_MIN,
        t1, (void*)expected_t1_counter).status == 0);

  ac_u64 i = 0;
  ac_u64 ic = 0;
  ac_u64 tc = __atomic_load_n(&t1_counter, __ATOMIC_ACQUIRE);
  ac_uint flags = enable_intr();
  for (i = 0; i < 0x0000010000000000; i++) {
    tc = __atomic_load_n(&t1_counter, __ATOMIC_ACQUIRE);
    if (tc >= expected_t1_counter) {
      break;
    }
    //ac_thread_yield();
  }
  restore_intr(flags);
  ic = get_timer_reschedule_isr_counter();

  ac_printf("test_ac_thread_create: t1_counter=%ld exptected_t1_counter=%ld ic=%ld\n",
      tc, expected_t1_counter, ic);

  error |= AC_TEST(((tc >= expected_t1_counter) &&
      (tc <= (expected_t1_counter + 1))));

  ac_printf("test_ac_thread_create: wait until t1 is done\n");
  // Wait until t1 is done
  // TODO: Add ac_thread_join
  for (i = 0; i < 0x0000000100000000; i++) {
    if (__atomic_load_n(&t1_done, __ATOMIC_ACQUIRE) != AC_FALSE) {
      break;
    }
    ac_thread_yield();
  }
  __atomic_store_n(&t1_done, AC_FALSE, __ATOMIC_RELEASE);
  ac_thread_yield();

  ac_printf("test_ac_thread_create:-error=%d\n", error);
  return error;
}

ac_uint test_timer() {
  ac_printf("test_timer:+\n");
  ac_uint error = AC_FALSE;

  set_timer_reschedule_isr_counter(0);

  error |= AC_TEST(get_timer_reschedule_isr_counter() == 0);

  ac_u64 isr_counter = 0;
  ac_u64 expected_isr_count = 2;
  ac_u64 test_timer_loops = 0;
  ac_uint flags = enable_intr();
  for (ac_u64 i = 0; (i < 10000000000) &&
      (get_timer_reschedule_isr_counter() < expected_isr_count); i++) {
    test_timer_loops += 1;
  }
  isr_counter = get_timer_reschedule_isr_counter();
  restore_intr(flags);

  ac_printf("test_timer: isr_counter=%ld expected_isr_count=%ld test_timer_loops=%ld\n",
     isr_counter, expected_isr_count, test_timer_loops);

  error |= AC_TEST(((isr_counter >= expected_isr_count) &&
      (isr_counter <= (expected_isr_count + 1))));

  ac_printf("test_timer:-error=%d\n", error);
  return error;
}

int main(void) {
  ac_uint error = AC_FALSE;

  ac_thread_init(32);

  if (!error) {
    error |= test_yield_no_other_threads();
    error |= test_yield_one_other_thread();
    error |= test_ac_thread_create();
    error |= test_timer();
  }

  ac_uint zombies = remove_zombies();
  ac_printf("py: zombies removed=%d\n", zombies);

  perf_yield();

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
