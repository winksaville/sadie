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

  ac_printf("AC_BIT(ac_bool, 0)=0x%x\n", AC_BIT(ac_bool, 0));
  ac_printf("AC_BIT(ac_u8, 0)=0x%x\n", AC_BIT(ac_u8, 0));
  ac_printf("AC_BIT(ac_u32, 7)=0x%x\n", AC_BIT(ac_u32, 7));
  ac_printf("AC_BIT(ac_u64, 63)=0x%llx\n", AC_BIT(ac_u64, 63));

  error |= AC_TEST(AC_BIT(ac_u8, 0) == 0x1);
  error |= AC_TEST(AC_BIT(ac_u16, 0) == 0x1);
  error |= AC_TEST(AC_BIT(ac_u32, 0) == 0x1);
  error |= AC_TEST(AC_BIT(ac_u64, 0) == 0x1);
  error |= AC_TEST(AC_BIT(ac_uptr, 0) == 0x1);
  error |= AC_TEST(AC_BIT(ac_uint, 0) == 0x1);

  error |= AC_TEST(AC_BIT(ac_u8, 7) == 0x80);
  error |= AC_TEST(AC_BIT(ac_u16, 15) == 0x8000);
  error |= AC_TEST(AC_BIT(ac_u32, 31) == 0x80000000);
  error |= AC_TEST(AC_BIT(ac_u64, 63) == 0x8000000000000000);

  error |= AC_TEST(AC_BIT(ac_s8, 0) == 0x1);
  error |= AC_TEST(AC_BIT(ac_s16, 0) == 0x1);
  error |= AC_TEST(AC_BIT(ac_s32, 0) == 0x1);
  error |= AC_TEST(AC_BIT(ac_s64, 0) == 0x1);

  error |= AC_TEST(AC_BIT(ac_bool, 0) == 0x1);
  error |= AC_TEST(AC_BIT(ac_uint, 0) == 0x1);
  error |= AC_TEST(AC_BIT(ac_sint, 0) == 0x1);
  error |= AC_TEST(AC_BIT(ac_int, 0) == 0x1);

  ac_u32 u32 = 0x80000000;
  error |= AC_TEST(AC_GET_BITS(ac_u32, u32, 30, 2) == 0b10);
  ac_printf("AC_GET_BITS(ac_u32, u32=0x80000000, 30, 2)=0b%b\n",
      AC_GET_BITS(ac_u32, u32, 30, 2));

  ac_u32 u32_1 = AC_SET_BITS(ac_u32, (ac_u32)0, 0b10, 30, 2);
  error |= AC_TEST(u32_1 == 0x80000000);
  u32_1 = AC_SET_BITS(ac_u32, u32_1, 0b1, 30, 1);
  error |= AC_TEST(u32_1 == 0xC0000000);

  ac_u64 u64_1 = AC_SET_BITS(ac_u64, (ac_u64)0, 0b10, 62, 2);
  ac_printf("AC_SET_BITS(ac_u64, 0lu, 0b10, 62, 2)=0x%lx\n", u64_1);

  error |= AC_TEST(u64_1 == 0x8000000000000000);
  u64_1 = AC_SET_BITS(ac_u64, u64_1, 0b01, 62, 1);
  ac_printf("AC_SET_BITS(ac_u64, 0lu, 0b01, 62, 1)=0x%lx\n", u64_1);
  error |= AC_TEST(u64_1 == 0xC000000000000000);

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
