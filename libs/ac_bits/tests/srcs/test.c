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

#include <ac_bits.h>

#include <ac_test.h>
#include <ac_printf.h>

ac_bool test_bits() {
  ac_bool error = AC_FALSE;

  ac_printf("AC_BIT(ac_u8, 0)=%b\n", AC_BIT(ac_u8, 0));
  ac_printf("AC_BIT(ac_u8, 0)=%b\n", AC_BIT(ac_u8, 7));

  error |= AC_TEST(AC_BIT(ac_u8, 0) == 0x1);
  error |= AC_TEST(AC_BIT(ac_u16, 0) == 0x1);
  error |= AC_TEST(AC_BIT(ac_u32, 0) == 0x1);
  error |= AC_TEST(AC_BIT(ac_u64, 0) == 0x1);
  error |= AC_TEST(AC_BIT(ac_uptr, 0) == 0x1);
  error |= AC_TEST(AC_BIT(ac_uint, 0) == 0x1);

  error |= AC_TEST(AC_BIT(ac_s8, 0) == 0x1);
  error |= AC_TEST(AC_BIT(ac_s16, 0) == 0x1);
  error |= AC_TEST(AC_BIT(ac_s32, 0) == 0x1);
  error |= AC_TEST(AC_BIT(ac_s64, 0) == 0x1);
  error |= AC_TEST(AC_BIT(ac_sint, 0) == 0x1);

  error |= AC_TEST(AC_BIT(ac_int, 0) == 0x1);

  return error;
}


int main(void) {
  ac_bool error = AC_FALSE;

  error |= test_bits();

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
