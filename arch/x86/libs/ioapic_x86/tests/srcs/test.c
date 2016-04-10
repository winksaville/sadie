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

#include <ioapic_x86.h>

#include <ac_inttypes.h>
#include <ac_printf.h>
#include <ac_test.h>

ac_bool test_ioapic(void) {
  ac_bool error = AC_FALSE;

  ac_uint count = ioapic_get_count();
  error |= AC_TEST(count != 0);

  for (ac_uint i = 0; i < count; i++) {
    ioapic_regs* pregs = ioapic_get_addr(i);
    ac_printf("test_ioapic: ioapci %d: %p\n", i, pregs);
    error |= AC_TEST(pregs != AC_NULL);
  }

  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  ac_uint count = ioapic_get_count();
  if (count != 0) {
    error |= test_ioapic();
  } else {
    ac_printf("test IOAPIC: NO IOAPIC, skipping tests\n");
  }

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
