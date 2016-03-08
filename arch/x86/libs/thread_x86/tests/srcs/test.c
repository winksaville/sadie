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

#include <thread_x86.h>

#include <apic_x86.h>
#include <interrupts_x86.h>

#include <ac_thread.h>
#include <ac_inttypes.h>
#include <ac_printf.h>
#include <ac_putchar.h>
#include <ac_test.h>

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
  ac_uint error = AC_FALSE;

  __atomic_store_n(&t1_counter, 0, __ATOMIC_RELEASE);
  __atomic_store_n(&t1_done, AC_FALSE, __ATOMIC_RELEASE);

  ac_u64 expected_t1_counter = 10; //1000000000;
  error |= AC_TEST(ac_thread_create(AC_THREAD_STACK_MIN, t1,
        (void*)expected_t1_counter) == 0);

  ac_u64 i = 0;
  ac_u64 ic = 0;
  ac_u64 tc = __atomic_load_n(&t1_counter, __ATOMIC_ACQUIRE);
  sti();
  for (i = 0; i < 0x0000010000000000; i++) {
    tc = __atomic_load_n(&t1_counter, __ATOMIC_ACQUIRE);
    if (tc >= expected_t1_counter) {
      break;
    }
    //ac_thread_yield();
  }
  cli();
  ic = get_timer_reschedule_isr_counter();

  ac_printf("test_ac_thread_create: t1_counter=%ld exptected_t1_counter=%ld ic=%ld\n",
      tc, expected_t1_counter, ic);

  error |= AC_TEST(((tc >= expected_t1_counter) &&
      (tc <= (expected_t1_counter + 1))));

  ac_printf("error=0x%x\n", error);

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

  return error;
}

ac_uint test_timer() {
  ac_uint error = AC_FALSE;

  error |= AC_TEST(get_timer_reschedule_isr_counter() == 0);

  ac_u64 isr_counter = 0;
  ac_u64 expected_isr_count = 20;
  ac_u64 test_timer_loops = 0;
  sti();
  for (ac_u64 i = 0; (i < 1000000000) &&
      (get_timer_reschedule_isr_counter() < expected_isr_count); i++) {
    test_timer_loops += 1;
  }
  isr_counter = get_timer_reschedule_isr_counter();
  cli();

  ac_printf("test_timer: isr_counter=%ld test_timer_loops=%ld\n",
     isr_counter, test_timer_loops);

  ac_u64 ic = get_timer_reschedule_isr_counter();
  error |= AC_TEST(((ic >= expected_isr_count) &&
      (ic <= (expected_isr_count + 1))));

  ac_thread_yield();

  ac_printf("error=0x%x\n", error);
  return error;
}

int main(void) {
  ac_uint error = AC_FALSE;


  // Initialize interrupt descriptor table and apic since
  // they are not done by default, yet.
  initialize_intr_descriptor_table();
  error =  AC_TEST(initialize_apic() == 0);

  ac_thread_early_init();
  ac_thread_init(32);

  if (!error) {
    error |= test_timer();
    error |= test_ac_thread_create();
  }

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
