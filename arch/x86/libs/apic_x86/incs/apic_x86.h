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

#ifndef ARCH_X86_APIC_X86_INCS_APIC_X86_H
#define ARCH_X86_APIC_X86_INCS_APIC_X86_H

#include <msr_x86.h>

#include <ac_inttypes.h>

/**
 * TSC-Deadline msr
 */
#define IA32_TSC_DEADLINE 0x6E0

/**
 * APIC linear address
 */
extern void* apic_lin_addr;

/**
 * Spurious Vector
 */
#define APIC_SPURIOUS_VECTOR 255

/**
 * APIC timer local vector table entry.
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 10.5.3 "APIC Timer"
 * Figure 10-10. "Divide Configuration Register"
 */
struct apic_timer_lvt_fields {
  ac_u8 vector:8;               // Interrupt vector
  ac_u8 reserved_0:4;
  ac_bool status:1;             // Delivery status, 0 == idle, 1 == Send Pending
  ac_u8 reserved_1:3;
  ac_bool disable:1;            // Interrupt mask, 0 == enabled, 1 == disabled
  ac_u8 mode:2;                 // Mode, 0 == one-shot, 1 == Periodic, 2 = TSC-Deadline
  ac_u32 reserved_2:13;
} __attribute__((__packed__));

_Static_assert(sizeof(struct apic_timer_lvt_fields) == 4,
    L"apic_timer_lvt_fields is not 4 bytes");

union apic_timer_lvt_fields_u {
  ac_u32 raw;
  struct apic_timer_lvt_fields fields;
};

_Static_assert(sizeof(union apic_timer_lvt_fields_u) == 4,
    L"apic_timer_lvt_fields_u is not 4 bytes");


/**
 * APIC spurious interrupt vector
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 10.9 "SPURIOUS INTERRUPT"
 * Figure 10-23. "Suprious-Interrupt Vector Register (SVR)"
 */
struct apic_spurious_vector_fields {
  ac_u8 vector:8;               // Spurious interrupt vector
  ac_bool apic_enable:1;        // APIC enable, 0 == disabled, 1 = enabled
  ac_bool fpc_disable:1;        // Focus processor cheching, 0 == enabled, 1 = disabled
  ac_u8 reserved_0:2;
  ac_bool eoi_broadcast_disable:1; // EOI-Broadcast suppression, 0=disabled, 1 = enabled
  ac_u32 reserved_2:19;
} __attribute__((__packed__));

_Static_assert(sizeof(struct apic_spurious_vector_fields) == 4,
    L"apic_spurious_vector_fields is not 4 bytes");

union apic_spurious_vector_fields_u {
  ac_u32 raw;
  struct apic_spurious_vector_fields fields;
};

_Static_assert(sizeof(union apic_timer_lvt_fields_u) == 4,
    L"apic_spurious_vector_fields_u is not 4 bytes");


/**
 * Initialize APIC
 *
 * @return 0 if initialized, !0 if an error
 */
ac_uint apic_early_init(void);

/**
 * @return 0 if present, !0 if not
 */
ac_bool apic_present(void);

/**
 * @return id of the local apic
 */
ac_u32 get_apic_id(void);

/**
 * @return physical address of local apic
 */
ac_u64 get_apic_physical_addr(void);

/**
 * @return linear address of local apic
 */
static __inline__ void* get_apic_linear_addr(void) {
  return apic_lin_addr;
}

/**
 * Send apic the End Of Interrupt (EOI) message
 *
 * Needs to be preformed at the end of the interrupt service routine
 * for all APIC interrupts except those delivered with NMI, SMI, INIT,
 * ExtINT, the start-up, or INIT_Deassert.
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 10.8.5 "Signaling Interrupt Service Completion"
 * Figure 10-21. "EOI Register"
 */
static __inline__ void send_apic_eoi(void) {
  ac_u32* p = (ac_u32*)(apic_lin_addr + 0xB0);
  *p = 0;
}

/**
 * Set apic_spurious vector
 *
 * Note there must be no EOI sent at the end of an spurious isr.
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 10.9 "Spurious Interrupt"
 * Figure 10-23. "Spurious-Interrupt vector Register"
 */
static __inline__ void set_apic_spurious_vector(
    struct apic_spurious_vector_fields svf) {
  union apic_spurious_vector_fields_u* p =
    (union apic_spurious_vector_fields_u*)(apic_lin_addr + 0xF0);
  p->fields = svf;
}

/**
 * @return spurious vector
 */
static __inline__ struct apic_spurious_vector_fields get_apic_spurious_vector(void) {
  union apic_spurious_vector_fields_u* p =
    (union apic_spurious_vector_fields_u*)(apic_lin_addr + 0xF0);
  return p->fields;
}

/**
 * Set apic timer local vector table entry.
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 10.5.3 "APIC Timer"
 * Figure 10-10. "Divide Configuration Register"
 */
static __inline__ void set_apic_timer_lvt(struct apic_timer_lvt_fields fields) {
  union apic_timer_lvt_fields_u* p =
    (union apic_timer_lvt_fields_u*)(apic_lin_addr + 0x320);
  p->fields = fields;
}

/**
 * Get apic timer local vector table entry.
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 10.5.3 "APIC Timer"
 * Figure 10-10. "Divide Configuration Register"
 */
static __inline__ struct apic_timer_lvt_fields get_apic_timer_lvt(void) {
  union apic_timer_lvt_fields_u* p =
    (union apic_timer_lvt_fields_u*)(apic_lin_addr + 0x320);
  return p->fields;
}

/**
 * Set apic timer divider.
 *
 * param: val is 0..7 this code will move bit 2 to bit 3.
 *        0 = divide by 2
 *        1 = divide by 4
 *        2 = divide by 8
 *        3 = divide by 16
 *        4 = divide by 32
 *        5 = divide by 64
 *        6 = divide by 128
 *        7 = divide by 1
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 10.5.3 "APIC Timer"
 * Figure 10-10. "Divide Configuration Register"
 */
static __inline__ void set_apic_timer_divider(ac_u32 val) {
  val = (val & 0x3) | ((val & 0x4) << 1);
  *(ac_u32*)(apic_lin_addr + 0x3E0) = val;
}

/**
 * Get apic timer divider.
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 10.5.3 "APIC Timer"
 * Figure 10-10. "Divide Configuration Register"
 */
static __inline__ ac_u32 get_apic_timer_divide_config(void) {
  ac_u32 val = *(ac_u32*)(apic_lin_addr + 0x3E0);
  return (val & 0x3) | ((val & 0x8) >> 1);
}

/**
 * Set initial count
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 10.5.3 "APIC Timer"
 * Figure 10-11. "Initial Count and Current Count Registers"
 */
static __inline__ void set_apic_timer_initial_count(ac_u32 val) {
  *(ac_u32*)(apic_lin_addr + 0x380) = val;
}

/**
 * Get initial count
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 10.5.3 "APIC Timer"
 * Figure 10-11. "Initial Count and Current Count Registers"
 */
static __inline__ ac_u32 get_apic_timer_initial_count(void) {
  return *(ac_u32*)(apic_lin_addr + 0x380);
}

/**
 * Set current count
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 10.5.3 "APIC Timer"
 * Figure 10-11. "Initial Count and Current Count Registers"
 */
static __inline__ void set_apic_timer_current_count(ac_u32 val) {
  *(ac_u32*)(apic_lin_addr + 0x390) = val;
}

/**
 * Get current count
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 10.5.3 "APIC Timer"
 * Figure 10-11. "Initial Count and Current Count Registers"
 */
static __inline__ ac_u32 get_apic_timer_current_count(void) {
  return *(ac_u32*)(apic_lin_addr + 0x390);
}

/**
 * Set apic tsr deadline
 */
static __inline__ void set_apic_timer_tsc_deadline(ac_u64 val) {
  set_msr(IA32_TSC_DEADLINE, val);
}

/**
 * Set apic tsr deadline
 */
static __inline__ ac_u64 get_apic_timer_tsc_deadline(void) {
  return get_msr(IA32_TSC_DEADLINE);
}

#endif
