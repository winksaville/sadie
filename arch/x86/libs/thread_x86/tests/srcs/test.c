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

ac_uint t1_counter;

void* t1(void* p) {
  t1_counter += 1;
  ac_printf("t1: p=%p t1_counter=%d\n", p, t1_counter);
  return AC_NULL;
}

ac_bool test_ac_thread_create() {
  ac_bool error = AC_FALSE;

  error |= AC_TEST(ac_thread_create(AC_THREAD_STACK_MIN, t1, (void*)1) == 0);

  ac_thread_yield();

  ac_u64 test_ac_thread_create_timer_loops = 0;
  sti();
  for (ac_u64 i = 0; (i < 1000000000) && (t1_counter < 1); i++) {
    test_ac_thread_create_timer_loops += 1;
  }
  cli();
  ac_printf("test_ac_thread_create: t1_counter=%d test_ac_thread_create_timer_loops=%d\n",
     t1_counter, test_ac_thread_create_timer_loops);

  error |= AC_TEST(t1_counter == 1);

  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;


  // Initialize interrupt descriptor table and apic since
  // they are not done by default, yet.
  initialize_intr_descriptor_table();
  error =  AC_TEST(initialize_apic() == 0);

  ac_thread_early_init();
  ac_thread_init(32);


  if (!error) {
    error |= test_ac_thread_create();
  } else {
    ac_printf("test thread_x86: NO APIC\n");
  }

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
