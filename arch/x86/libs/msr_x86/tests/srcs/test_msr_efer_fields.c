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
static struct test_case {
  union msr_efer_u val;
  ac_u64 sce;
  ac_u64 reserved_0;
  ac_u64 lme;
  ac_u64 reserved_1;
  ac_u64 lma;
  ac_u64 nxe;
  ac_u64 reserved_2;
} test_case_array[] = {
  { .val.raw=0x1, .sce=1, },
  { .val.raw=0x2, .reserved_0=1, },
  { .val.raw=0x80, .reserved_0=0x40, },
  { .val.raw=0x100, .lme=1, },
  { .val.raw=0x200, .reserved_1=1, },
  { .val.raw=0x400, .lma=1, },
  { .val.raw=0x800, .nxe=1, },
  { .val.raw=0x1000, .reserved_2=1, },
  { .val.raw=0x8000000000000000, .reserved_2=0x8000000000000, },
};

ac_bool test_msr_efer_field(struct test_case* test) {
  ac_bool error = AC_FALSE;

  print_msr(MSR_EFER, test->val.raw);

  error |= AC_TEST(test->val.fields.sce == test->sce);
  error |= AC_TEST(test->val.fields.reserved_0 == test->reserved_0);
  error |= AC_TEST(test->val.fields.lme == test->lme);
  error |= AC_TEST(test->val.fields.reserved_1 == test->reserved_1);
  error |= AC_TEST(test->val.fields.lma == test->lma);
  error |= AC_TEST(test->val.fields.nxe == test->nxe);
  error |= AC_TEST(test->val.fields.reserved_2 == test->reserved_2);

  return error;
}

ac_bool test_msr_efer_fields() {
  ac_bool error = AC_FALSE;

  // Execute the test cases
  for (ac_uint i = 0; i < AC_ARRAY_COUNT(test_case_array); i++) {
    error |= test_msr_efer_field(&test_case_array[i]);
  }

  // Verify we can read the MSR_APIC_BASE and expect it to be non-zero
  ac_u64 msr_efer = get_msr(MSR_MTRRCAP);
  print_msr(MSR_MTRRCAP, msr_efer);

  error |= AC_TEST(msr_efer != 0);

  return error;
}
