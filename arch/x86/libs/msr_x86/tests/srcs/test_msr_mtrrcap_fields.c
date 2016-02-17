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
  union msr_mtrrcap_u val;
  ac_u64 vcnt;
  ac_u64 fix;
  ac_u64 reserved_0;
  ac_u64 wc;
  ac_u64 smrr;
  ac_u64 reserved_1;
} test_case_array[] = {
  { .val.raw=0x1, .vcnt=0x1, },
  { .val.raw=0x80, .vcnt=0x80, },
  { .val.raw=0x100, .fix=1, },
  { .val.raw=0x200, .reserved_0=1, },
  { .val.raw=0x400, .wc=1, },
  { .val.raw=0x800, .smrr=1, },
  { .val.raw=0x1000, .reserved_1=1, },
  { .val.raw=0x80000000, .reserved_1=0x80000, },
};

ac_bool test_msr_mtrrcap_field(struct test_case* test) {
  ac_bool error = AC_FALSE;

  print_msr(MSR_MTRRCAP, test->val.raw);

  error |= AC_TEST(test->val.fields.vcnt == test->vcnt);
  error |= AC_TEST(test->val.fields.fix == test->fix);
  error |= AC_TEST(test->val.fields.reserved_0 == test->reserved_0);
  error |= AC_TEST(test->val.fields.wc == test->wc);
  error |= AC_TEST(test->val.fields.smrr == test->smrr);
  error |= AC_TEST(test->val.fields.reserved_1 == test->reserved_1);

  return error;
}

ac_bool test_msr_mtrrcap_fields() {
  ac_bool error = AC_FALSE;

  // Execute the test cases
  for (ac_uint i = 0; i < AC_ARRAY_COUNT(test_case_array); i++) {
    error |= test_msr_mtrrcap_field(&test_case_array[i]);
  }

  // Verify we can read the MSR_APIC_BASE and expect it to be non-zero
  ac_u64 msr_mtrrcap = get_msr(MSR_MTRRCAP);
  print_msr(MSR_MTRRCAP, msr_mtrrcap);

  error |= AC_TEST(msr_mtrrcap != 0);

  return error;
}
