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

#include <cr_x86.h>
#include <cr_x86_print.h>

#include <ac_bits.h>
#include <ac_printf.h>
#include <ac_test.h>

ac_bool test_crs(void) {
  ac_bool error = AC_FALSE;
  union cr0_u cr0 = { .raw = get_cr0() };
  // cr1 is reserved
  ac_uint cr2 = get_cr2();
  union cr3_u cr3 = { .raw = get_cr3() };
  union cr4_u cr4 = { .raw = get_cr4() };
  ac_uint cr8 = get_cr8();

  print_cr0("cr0", cr0.raw);
  ac_printf("cr2: 0x%p\n", cr2);
  print_cr3("cr3", cr3.raw);
  print_cr4("cr4", cr4.raw);
  ac_printf("cr8: 0x%p\n", cr8);

  set_cr0(cr0.raw);
  // cr2 is read only
  set_cr3(cr3.raw);
  set_cr4(cr4.raw);
  set_cr8(cr8);

  ac_uint cr0_1 = get_cr0();
  ac_uint cr3_1 = get_cr3();
  ac_uint cr4_1 = get_cr4();
  ac_uint cr8_1 = get_cr8();

  error |= AC_TEST(cr0.raw == cr0_1);
  error |= AC_TEST(cr3.raw == cr3_1);
  error |= AC_TEST(cr4.raw == cr4_1);
  error |= AC_TEST(cr8 == cr8_1);

  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  error |= test_crs();

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
