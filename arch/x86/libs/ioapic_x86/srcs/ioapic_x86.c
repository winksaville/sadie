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

#include <page_table_x86.h>

#include <ac_bits.h>

/**
 * @return the number of ioapic's.
 */
ac_uint ioapic_get_count(void) {
  // TODO: Get count from ACPI information
  return 1;
}

/**
 * Get the address of an ioapic
 *
 * @param idx is the index to the ioapic 0..n where
 *        n is the value returned by ioapci_get_count().
 *
 * @return the address of an ioapic
 */
ioapic_regs* ioapic_get_addr(ac_uint idx) {
  // TODO: Get address from ACPI information
  if (idx == 0) {
    return (ioapic_regs*)0xfec00000;
  } else {
    return AC_NULL;
  }
}

/**
 * Get the ioapic id as an byte
 *
 * @param regs is the address of the ioapic_regs
 *
 * @return id value
 */
ac_u8 ioapic_get_id(ioapic_regs* regs) {
  ac_u32 tmp = ioapic_read_u32(regs, 0);
  return AC_GET_BITS(ac_u8, tmp, 4, 24);
}

/**
 * Set ioapic id
 *
 * @param regs is the address of the ioapic_regs
 * @param val is the id to write
 */
void ioapic_set_id(ioapic_regs* regs, ac_u8 val) {
  ac_u32 tmp = ioapic_read_u32(regs, 0);
  tmp = AC_SET_BITS(ac_u32, tmp, val, 4, 24);
  ioapic_write_u32(regs, 0, tmp);
}

/**
 * Get IOAPIC_VER register
 *
 * @param regs is the address of the ioapic_regs
 *
 * @return IOAPIC_VER value
 */
ac_uint ioapic_get_ver(ioapic_regs* regs) {
  ac_u32 tmp = ioapic_read_u32(regs, 1);
  return AC_GET_BITS(ac_u32, tmp, 8, 0);
}

/**
 * Get ioapic arbitration register
 *
 * @param regs is the address of the ioapic_regs
 *
 * @return arbitration register
 */
ac_u32 ioapic_get_arb(ioapic_regs* regs) {
  return ioapic_read_u32(regs, 2);
}

/**
 * Get max redirection entry. First entry is 0, max is the last.
 *
 * @param regs is the address of the ioapic_regs
 *
 * @return max redirection entry
 */
ac_uint ioapic_get_redir_max_entry(ioapic_regs* regs) {
  ac_u32 tmp = ioapic_read_u32(regs, 1);
  return AC_GET_BITS(ac_u32, tmp, 8, 16);
}

/**
 * Get redirection register. 0 is first 64 redirection register,
 * 1 is the second 64 bit redirection register ...
 *
 * @param regs is the address of the ioapic_regs
 * @param idx to a 64 bit redirection register, 0 is first,
 *        1 is the second 64 bit register and so on.
 *
 * @return the ioapic_redir register
 */
ioapic_redir ioapic_get_redir(ioapic_regs* regs, ac_uint idx) {
  ioapic_redir redir;
  redir.raw = ioapic_read_u64(regs, 0x10 + (idx * 2));
  return redir;
}

/**
 * Set redirection register. 0 is first 64 redirection register,
 * 1 is the second 64 bit redirection register ...
 *
 * @param regs is the address of the ioapic_regs
 * @param idx to a 64 bit redirection register, 0 is first,
 *        1 is the second 64 bit register and so on.
 */
void ioapic_set_redir(ioapic_regs* regs, ac_uint idx, ioapic_redir reg) {
  ioapic_write_u64(regs, 0x10 + (idx * 2), reg.raw);
}

/**
 * ioapic early initialization
 */
void ioapic_early_init(void) {
  ac_uint count = ioapic_get_count();
  ac_printf("ioapic_early_init+count=%d\n", count);
  if (count != 0) {
    ioapic_regs* regs = ioapic_get_addr(0);

    // Map registers into address space
    page_table_map_lin_to_phy(get_page_table_linear_addr(),
        regs, (ac_u64)regs, FOUR_K_PAGE_SIZE,
        PAGE_CACHING_STRONG_UNCACHEABLE); //PAGE_CACHING_WRITE_BACK);

    // Initialize redirection registers.
    //
    // Zero everything except interrupt mask which
    // is set to 1 to disable interrupt forwarding.
    for (ac_uint i = 0; i <= ioapic_get_redir_max_entry(regs); i++) {
      ioapic_redir redir;
      redir.raw = 0;
      redir.intr_mask = 1;
      ioapic_set_redir(regs, i, redir);
    }

    ioapic_print();
  }

  ac_printf("ioapic_early_init-\n");
}
