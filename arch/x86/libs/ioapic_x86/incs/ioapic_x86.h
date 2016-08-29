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

/**
 * Intel Doc for "82093AA I/O ADVANCED PROGRAMMABLE INTERRUPT CONTROLLER (IOAPIC)"
 * availble at https://goo.gl/X1lkkW. This document is for version 0x11 but
 * currently I'm seeing verion 0x20 but have found no documentation for this
 * later version.
 */

#ifndef ARCH_X86_LIBS_IOAPIC_X86_INCS_IOAPIC_X86_H
#define ARCH_X86_LIBS_IOAPIC_X86_INCS_IOAPIC_X86_H

#include <ac_assert.h>
#include <ac_attributes.h>
#include <ac_bits.h>
#include <ac_inttypes.h>
#include <ac_printf.h>

#define IOAPIC_ID       0       // ID
#define IOAPIC_VER      1       // Version
#define IOAPIC_ARB      2       // Arbitration register
#define IOAPIC_REDTBL   0x10    // Redirection table

struct AC_ATTR_PACKED ioapic_regs {
  volatile ac_u32 index;        // Index to a 32 bit register 0 == first reg, 1 = second ...
  volatile ac_u32 resv[3];
  volatile ac_u32 data;         // Data to read/write to the addressed register
};

typedef struct ioapic_regs ioapic_regs;

struct AC_ATTR_PACKED ioapic_redir {
  union {
    ac_u64  raw;
    struct {
      ac_u8 intr_vec:8;             // Interrupt Vector (0x10 .. 0xFE) (RW)
      ac_u8 delivery_mode:3;        // 0x0 = Fixed, 0x1 = Lowest priority, 0x2 SMI
                                    // 0x3 = Resv, 0x4 = NMI, 0x5 = INIT,
                                    // 0x6 = Resv, 0x7 = External Interrupt (RW)
      ac_u8 dest_mode:1;            // 0 = Physical mode and dest_field == APIC ID
                                    // 1 = Logical mode and dest_field == logical dest address
                                    // From 3.2.4 of 82093AA documenation (https://goo.gl/X1lkkW)
                                    // "When DESTMOD=1 (logical mode), destinations are identified
                                    // by matching on the logical destination under the control of
                                    // the Destination Format Register and Logical Destination
                                    // Register in each Local APIC"
      ac_u8 delivery_status:1;      // 0 = Idle, 1 = Send pending (RO)
      ac_u8 intr_polarity:1;        // 0 = High active, 1 = Low active (RW)
      ac_u8 remote_irr:1;           // 0 = EOI received, 1 = local APIC accepted (RO)
      ac_u8 trigger:1;              // 0 = Edge sensitive, 1 = Level sensitive (RW)
      ac_u8 intr_mask:1;            // 0 = Interrupt is NOT masked, 1 interrupt masked (RW)
      ac_u64 resv:38;
      ac_u64 dest_field:7;          // If dest_mode == 0 then dest_field == APIC ID
                                    // else dest_field == 1 logical destination address
                                    // which can be one or more CPU's.
    };
  };
};

typedef struct ioapic_redir ioapic_redir;

ac_static_assert(sizeof(ioapic_redir) == 8, L"ioapic_redir is not 8 bytes");

/**
 * @return an ioapic 32 bit register
 */
static inline ac_u32 ioapic_read_u32(ioapic_regs* regs, ac_uint idx) {
  regs->index = idx;
  return regs->data;
}

/**
 * @return a pair of ioapic 32 bit registers and as ac_u64
 */
static inline ac_u64 ioapic_read_u64(ioapic_regs* regs, ac_uint idx) {
  return (ac_u64)ioapic_read_u32(regs, idx) |
    ((ac_u64)ioapic_read_u32(regs, idx+1) << 32);
}

/**
 * Write a u32
 */
static inline void ioapic_write_u32(ioapic_regs* regs, ac_uint idx, ac_u32 val) {
  regs->index = idx;
  regs->data = val;
}

/**
 * Write a u64 to two ioapic 32 bit regs
 */
static inline void ioapic_write_u64(ioapic_regs* regs, ac_uint idx, ac_u64 val) {
  ioapic_write_u32(regs, idx, AC_GET_BITS(ac_u32, val, 32, 0));
  ioapic_write_u32(regs, idx+1, AC_GET_BITS(ac_u32, val, 32, 32));
}

/**
 * @return the number of ioapic's.
 */
ac_uint ioapic_get_count(void);

/**
 * Get the address of an ioapic
 *
 * @param idx is the index to the ioapic 0..n where
 *        n is the value returned by ioapic_get_count().
 *
 * @return the address of an ioapic
 */
ioapic_regs* ioapic_get_addr(ac_uint idx);

/**
 * Get the ioapic id as an byte
 *
 * @param regs is the address of the ioapic_regs
 *
 * @return id value
 */
ac_u8 ioapic_get_id(ioapic_regs* regs);

/**
 * Set ioapic id
 *
 * @param regs is the address of the ioapic_regs
 * @param val is the id to write
 */
void ioapic_set_id(ioapic_regs* regs, ac_u8 val);

/**
 * Get ioapic version
 *
 * @param regs is the address of the ioapic_regs
 *
 * @return version
 */
ac_uint ioapic_get_ver(ioapic_regs* regs);

/**
 * Get ioapic arbitration register
 *
 * @param regs is the address of the ioapic_regs
 *
 * @return arbitration register
 */
ac_u32 ioapic_get_arb(ioapic_regs* regs);

/**
 * Get max redirection entry. First entry is 0, max is the last.
 *
 * @param regs is the address of the ioapic_regs
 *
 * @return max redirection entry
 */
ac_uint ioapic_get_redir_max_entry(ioapic_regs* regs);

/**
 * Get redirection register 0 is first 1 is the second 64 bit register.
 * This method will convert idx to a register address and will
 * read both 32bit registers to form the 64bit value.
 *
 * @param regs is the address of the ioapic_regs
 *
 * @return the ioapic_redir register
 */
ioapic_redir ioapic_get_redir(ioapic_regs* regs, ac_uint idx);

/**
 * Set redirection register 0 is first 1 is the second 64 bit register.
 * This method will convert idx to a register address and will
 * read both 32bit registers to form the 64bit value.
 *
 * @return the ioapic_redir register
 */
void ioapic_set_redir(ioapic_regs* regs, ac_uint idx, ioapic_redir reg);

/**
 * ioapic early initialization
 */
void ioapic_early_init(void);

#endif
