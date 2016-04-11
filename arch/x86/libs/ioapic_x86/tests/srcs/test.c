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

#include <page_table_x86.h>
#include <ac_inttypes.h>
#include <ac_printf.h>
#include <ac_test.h>

void display_ioapic(void) {
  ac_uint count = ioapic_get_count();
  for (ac_uint ioapic = 0; ioapic < count; ioapic++) {
    ioapic_regs* regs = ioapic_get_addr(ioapic);
    for (ac_uint i = 0; i < 3; i++) {
      ac_printf("display_ioapic: 0x=%x 0x=%p\n", i, ioapic_read_u32(regs, i));
    }

    for (ac_uint i = 0x10; i < 0x40; i += 2) {
      ac_u32 l = ioapic_read_u32(regs, i);
      ac_u32 h = ioapic_read_u32(regs, i+1);
      ac_u64 d = ioapic_read_u64(regs, i);

      ac_printf("display_ioapic: 0x=%x 0x=%p\n", i, l);
      ac_printf("display_ioapic: 0x=%x 0x=%p\n", i + 1, h);
      ac_printf("display_ioapic: 0x=%x 0x=%p\n", i, d);
    }
  }
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
    ioapic_regs* regs = ioapic_get_addr(0);

    page_table_map_lin_to_phy(get_page_table_linear_addr(),
        regs, (ac_u64)regs, FOUR_K_PAGE_SIZE,
        PAGE_CACHING_STRONG_UNCACHEABLE); //PAGE_CACHING_WRITE_BACK);
    display_ioapic();
    error |= test_ioapic();
  } else {
    ac_printf("test IOAPIC: NO IOAPIC, skipping tests\n");
  }

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
