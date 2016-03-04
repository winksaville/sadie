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

ac_bool test_x() {
  ac_bool error = AC_FALSE;

  ac_printf("text_x: no tests yet\n");

  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  // Initialize interrupt descriptor table and apic since
  // they are not done by default, yet.
  initialize_intr_descriptor_table();
  error =  AC_TEST(initialize_apic() == 0);

  if (!error) {
    error |= test_x();
  } else {
    ac_printf("test thread_x86: NO APIC\n");
  }

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
