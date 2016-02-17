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

#include <descriptors_x86.h>
#include <descriptors_x86_print.h>

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
  union seg_desc_u val;
  ac_u64 seg_limit_lo;
  ac_u64 base_addr_lo;
  ac_u64 type;
  ac_u64 s;
  ac_u64 dpl;
  ac_u64 p;
  ac_u64 seg_limit_hi;
  ac_u64 avl;
  ac_u64 l;
  ac_u64 d_b;
  ac_u64 g;
  ac_u64 base_addr_hi;
};

static struct test_case test_case_array[] = {
  { .val.raw=0x1, .seg_limit_lo=0x1, },
  { .val.raw=0x8000, .seg_limit_lo=0x8000, },
  { .val.raw=0x10000, .base_addr_lo=0x1, },
  { .val.raw=0x8000000000, .base_addr_lo=0x800000, },
  { .val.raw=0x10000000000, .type=0x1, },
  { .val.raw=0x80000000000, .type=0x8, },
  { .val.raw=0x100000000000, .s=0x1, },
  { .val.raw=0x200000000000, .dpl=0x1, },
  { .val.raw=0x400000000000, .dpl=0x2, },
  { .val.raw=0x800000000000, .p=0x1, },
  { .val.raw=0x1000000000000, .seg_limit_hi=0x1, },
  { .val.raw=0x8000000000000, .seg_limit_hi=0x8, },
  { .val.raw=0x10000000000000, .avl=0x1, },
  { .val.raw=0x20000000000000, .l=0x1, },
  { .val.raw=0x40000000000000, .d_b=0x1, },
  { .val.raw=0x80000000000000, .g=0x1, },
  { .val.raw=0x100000000000000, .base_addr_hi=0x1, },
  { .val.raw=0x8000000000000000, .base_addr_hi=0x80, },
};

static ac_bool test_seg_desc(struct test_case* test) {
  ac_bool error = AC_FALSE;

  print_seg_desc("seg_desc", &test->val.fields);

  error |= AC_TEST(test->val.fields.seg_limit_lo == test->seg_limit_lo);
  error |= AC_TEST(test->val.fields.base_addr_lo == test->base_addr_lo);
  error |= AC_TEST(test->val.fields.type == test->type);
  error |= AC_TEST(test->val.fields.s == test->s);
  error |= AC_TEST(test->val.fields.dpl == test->dpl);
  error |= AC_TEST(test->val.fields.p == test->p);
  error |= AC_TEST(test->val.fields.seg_limit_hi == test->seg_limit_hi);
  error |= AC_TEST(test->val.fields.avl == test->avl);
  error |= AC_TEST(test->val.fields.l == test->l);
  error |= AC_TEST(test->val.fields.d_b == test->d_b);
  error |= AC_TEST(test->val.fields.g == test->g);
  error |= AC_TEST(test->val.fields.base_addr_hi == test->base_addr_hi);

  return error;
}

ac_bool test_seg_desc_fields() {
  ac_bool error = AC_FALSE;

  // Execute the test cases
  for (ac_uint i = 0; i < AC_ARRAY_COUNT(test_case_array); i++) {
    error |= test_seg_desc(&test_case_array[i]);
  }

  return error;
}

ac_bool test_gdt_ldt(void) {
  desc_ptr dp1;
  desc_ptr dp2;
  ac_bool error = AC_FALSE;

  // Get current GDT and verifiy we can write it
  // and read back the same value. Not a great test
  // but in the short term anything else would be
  // fatal.
  get_gdt(&dp1);
  set_gdt(&dp1);
  get_gdt(&dp2);

  error |= AC_TEST_EM(dp1.limit == dp2.limit,
      "Unable to get/set/get GDT register desc_ptr.limit");

  error |= AC_TEST_EM(dp1.sd == dp2.sd,
      "Unable to get/set/get GDT register desc_ptr.sd");

  // Get current LDT and verifiy we can write it
  // and read back the same value. Not a great test
  // but in the short term anything else would be
  // fatal.
  get_ldt(&dp1);
  set_ldt(&dp1);
  get_ldt(&dp2);

  error |= AC_TEST_EM(dp1.limit == dp2.limit,
      "Unable to get/set/get LDT register desc_ptr.limit");

  error |= AC_TEST_EM(dp1.sd == dp2.sd,
      "Unable to get/set/get LDT register desc_ptr.sd");

  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  desc_ptr dp;
  get_gdt(&dp);
  print_desc_table("gdt", dp);

  error |= test_seg_desc_fields();
  error |= test_gdt_ldt();

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
