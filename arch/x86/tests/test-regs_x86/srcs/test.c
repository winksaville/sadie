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

#include <regs_x86.h>
#include <descriptors_x86.h>

#include <ac_test.h>
#include <ac_printf.h>

ac_bool test_gdt() {
  descriptor_ptr dp1;
  descriptor_ptr dp2;
  ac_bool error = AC_FALSE;

  // Get current value and verifiy we can write it
  // and read back the same value. Not a great test
  // but in the short term anything else would be
  // fatal.
  get_gdt(&dp1);
  set_gdt(&dp1);
  get_gdt(&dp2);

  if (dp1.limit != dp2.limit) {
    ac_printf("Error: dp1.limit != dp2.limit");
    error |= AC_TRUE;
  }

  if (dp1.address != dp2.address) {
    ac_printf("Error: dp1.limit != dp2.limit");
    error |= AC_TRUE;
  }

  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  error |= AC_TEST(test_gdt() == 0);

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
