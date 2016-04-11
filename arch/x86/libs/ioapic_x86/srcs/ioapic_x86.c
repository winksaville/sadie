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
  // TODO: Get count from ACPI information
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
  return AC_GET_BITS(ac_u8, ioapic_read_u32(regs, 0), 24, 4);
}

/**
 * Set ioapic id
 *
 * @param regs is the address of the ioapic_regs
 * @param val is the id to write
 */
void ioapic_set_id(ioapic_regs* regs, ac_u8 val) {
}

/**
 * Get max redirection entry. First entry is 0, max is the last.
 *
 * @param regs is the address of the ioapic_regs
 *
 * @return max redirection entry
 */
ac_uint ioapic_get_max_redir_entry(ioapic_regs* regs) {
  return 23;
}

/**
 * Get IOAPIC_VER register
 *
 * @param regs is the address of the ioapic_regs
 *
 * @return IOAPIC_VER value
 */
ac_uint ioapic_get_ver(ioapic_regs* regs) {
  return 0;
}

/**
 * Get redirection register 0 is first 1 is the second 64 bit register.
 * This method will convert idx to a register address and will
 * read both 32bit registers to form the 64bit value.
 *
 * @param regs is the address of the ioapic_regs
 *
 * @return the ioapic_redir register
 */
ioapic_redir ioapic_get_redir(ioapic_regs* regs, ac_uint idx) {
  ioapic_redir redir = {};
  return redir;
}

/**
 * Set redirection register 0 is first 1 is the second 64 bit register.
 * This method will convert idx to a register address and will
 * read both 32bit registers to form the 64bit value.
 *
 * @return the ioapic_redir register
 */
void ioapic_set_redir(ioapic_regs* regs, ac_uint idx, ioapic_redir reg) {
}
