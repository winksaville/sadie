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

#include <ioapic_x86.h>
#include <ioapic_x86_print.h>

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
  ioapic_redir val;
  ac_u8 intr_vec;
  ac_u8 delivery_mode;
  ac_u8 dest_mode;
  ac_u8 delivery_status;
  ac_u8 intr_polarity;
  ac_u8 remote_irr;
  ac_u8 trigger;
  ac_u8 intr_mask;
  ac_u64 resv;
  ac_u64 dest_field;
};


static struct test_case test_case_array[] = {
  { .val.raw=0x0000000000000001, .intr_vec=0x1, },
  { .val.raw=0x0000000000000080, .intr_vec=0x80, },
  { .val.raw=0x0000000000000100, .delivery_mode=0x1, },
  { .val.raw=0x0000000000000400, .delivery_mode=0x4, },
  { .val.raw=0x0000000000000800, .dest_mode=0x1, },
  { .val.raw=0x0000000000001000, .delivery_status=0x1, },
  { .val.raw=0x0000000000002000, .intr_polarity=0x1, },
  { .val.raw=0x0000000000004000, .remote_irr=0x1, },
  { .val.raw=0x0000000000008000, .trigger=0x1, },
  { .val.raw=0x0000000000010000, .intr_mask=0x1, },
  { .val.raw=0x0000000000020000, .resv=0x1, },
  { .val.raw=0x0080000000000000, .resv=0x4000000000, },
  { .val.raw=0x0100000000000000, .dest_field=0x1, },
  { .val.raw=0x8000000000000000, .dest_field=0x80, },
};

static ac_bool test_ioapic_redir(struct test_case* test) {
  ac_bool error = AC_FALSE;

  error |= AC_TEST(test->val.intr_vec == test->intr_vec);

  if (error) {
    ac_printf("test_ioapic_redir: raw=0x%lx ", test->val.raw);
    ioapic_redir_print("", test->val, "\n");
  }

  return error;
}

ac_bool test_ioapic_redir_fields() {
  ac_bool error = AC_FALSE;

  // Execute the test cases
  for (ac_uint i = 0; i < AC_ARRAY_COUNT(test_case_array); i++) {
    error |= test_ioapic_redir(&test_case_array[i]);
  }

  return error;
}


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
    error |= test_ioapic_redir_fields();
  } else {
    ac_printf("test IOAPIC: NO IOAPIC, skipping tests\n");
  }

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
