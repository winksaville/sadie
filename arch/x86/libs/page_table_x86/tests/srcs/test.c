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

#include <page_table_x86.h>
#include <print_page_table_x86.h>
#include <cr_x86.h>
#include <cr_x86_print.h>

#include <interrupts_x86.h>

#include <ac_architecture.h>
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
  union cr3_u val;
  ac_uint reserved_0;
  ac_uint pwt;
  ac_uint pcd;
  ac_uint reserved_1;
  ac_uint page_directory_base;
} test_case_array[] = {
  { .val.raw=0x1, .reserved_0=0x1, },
  { .val.raw=0x4, .reserved_0=0x4, },
  { .val.raw=0x8, .pwt=0x1, },
  { .val.raw=0x10, .pcd=0x1, },
  { .val.raw=0x20, .reserved_1=0x1, },
  { .val.raw=0x800, .reserved_1=0x40, },
#ifdef CPU_X86_64
  { .val.raw=0x1000, .page_directory_base=0x1, },
  { .val.raw=0x8000000000000000, .page_directory_base=0x8000000000000, },
  { .val.raw=0xC000000000000000, .page_directory_base=0xC000000000000, },
#else
  { .val.raw=0x1000, .page_directory_base=0x1, },
  { .val.raw=0x80000000, .page_directory_base=0x80000, },
#endif
};

static ac_bool test_cr3_fields(struct test_case* test) {
  ac_bool error = AC_FALSE;

  print_cr3("cr3", test->val.raw);

  error |= AC_TEST(test->val.fields.reserved_0 == test->reserved_0);
  error |= AC_TEST(test->val.fields.pwt == test->pwt);
  error |= AC_TEST(test->val.fields.pcd == test->pcd);
  error |= AC_TEST(test->val.fields.reserved_1 == test->reserved_1);
  error |= AC_TEST(test->val.fields.page_directory_base == test->page_directory_base);

  return error;
}

static ac_bool test_cr3(void) {
  ac_bool error = AC_FALSE;

  for (ac_uint i = 0; i < AC_ARRAY_COUNT(test_case_array); i++) {
    error |= test_cr3_fields(&test_case_array[i]);
  }

  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  initialize_intr_descriptor_table();

  print_page_table(get_page_table(), get_page_mode());

  error |= test_cr3();

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
