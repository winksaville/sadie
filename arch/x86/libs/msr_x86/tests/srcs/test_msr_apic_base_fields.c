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

#include <test_msr_x86.h>

#include <msr_x86.h>
#include <print_msr.h>

#include <ac_bits.h>
#include <ac_inttypes.h>
#include <ac_printf.h>
#include <ac_test.h>

/**
 * Test case array filled with the val followed
 * by the expected field values. The array initialization
 * fields that are NOT explicitly initialized are zeros.
 *
 * This is a shortened walking 1 bit test.
 */
struct test_case {
  union msr_apic_base_u val;
  ac_u64 reserved_0;
  ac_u64 bsp;
  ac_u64 reserved_1;
  ac_u64 extd;
  ac_u64 e;
  ac_u64 base_addr;
};

static struct test_case test_case_array[] = {
  { .val.raw=0x1, .reserved_0=0x1, },
  { .val.raw=0x80, .reserved_0=0x80, },
  { .val.raw=0x100, .bsp=1, },
  { .val.raw=0x200, .reserved_1=1, },
  { .val.raw=0x400, .extd=1, },
  { .val.raw=0x800, .e=1, },
  { .val.raw=0x1000, .base_addr=1, },
  { .val.raw=0x80000000, .base_addr=0x80000, },
};

static ac_bool test_msr_apic_base_field(struct test_case* test) {
  ac_bool error = AC_FALSE;

  print_msr(MSR_APIC_BASE, test->val.raw);

  error |= AC_TEST(test->val.fields.reserved_0 == test->reserved_0);
  error |= AC_TEST(test->val.fields.bsp == test->bsp);
  error |= AC_TEST(test->val.fields.reserved_1 == test->reserved_1);
  error |= AC_TEST(test->val.fields.extd == test->extd);
  error |= AC_TEST(test->val.fields.e == test->e);
  error |= AC_TEST(test->val.fields.base_addr == test->base_addr);

  return error;
}

ac_bool test_msr_apic_base_fields() {
  ac_bool error = AC_FALSE;

  // Execute the test cases
  for (ac_uint i = 0; i < AC_ARRAY_COUNT(test_case_array); i++) {
    error |= test_msr_apic_base_field(&test_case_array[i]);
  }

  // Verify we can read the MSR_APIC_BASE and expect it to be non-zero
  ac_u64 msr_apic_base = get_msr(MSR_APIC_BASE);

  error |= AC_TEST(msr_apic_base != 0);

  return error;
}
