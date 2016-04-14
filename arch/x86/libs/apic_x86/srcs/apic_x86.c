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

#include <apic_x86.h>

#include <interrupts_x86.h>
#include <io_x86.h>
#include <msr_x86.h>
#include <page_table_x86.h>
#include <page_table_x86_print.h>

#include <ac_inttypes.h>
#include <ac_bits.h>
#include <cpuid_x86.h>

#include <ac_printf.h>


void* apic_lin_addr;

/* From OSDev.org http://wiki.osdev.org/PIC */

#define PIC1            0x20      // IO base address for master PIC
#define PIC2            0xA0      // IO base address for slave PIC
#define PIC1_COMMAND    PIC1
#define PIC1_DATA       (PIC1+1)
#define PIC2_COMMAND    PIC2
#define PIC2_DATA       (PIC2+1)

#define ICW1_ICW4	0x01	  // ICW4 (not) needed
#define ICW1_SINGLE	0x02	  // Single (cascade) mode
#define ICW1_INTERVAL4	0x04	  // Call address interval 4 (8)
#define ICW1_LEVEL	0x08	  // Level triggered (edge) mode
#define ICW1_INIT	0x10	  // Initialization - required!

#define ICW4_8086	0x01	  // 8086/88 (MCS-80/85) mode
#define ICW4_AUTO	0x02	  // Auto (normal) EOI
#define ICW4_BUF_SLAVE	0x08	  // Buffered mode/slave
#define ICW4_BUF_MASTER	0x0C	  // Buffered mode/master
#define ICW4_SFNM	0x10	  // Special fully nested (not)

/**
 * Remap pic irq's.
 *
 * param: master_vector_base is the master pic's base interrupt vector
 * param: slave_vector_base is the slave pic's base interrupt vector
*/
static void remap_pic_vectors(ac_u8 master_vector_base, ac_u8 slave_vector_base) {
  unsigned char a1, a2;

  // Save masks
  a1 = inb_port(PIC1_DATA);
  a2 = inb_port(PIC2_DATA);

  // Starts the initialization sequence (in cascade mode)
  outb_port_value(PIC1_COMMAND, ICW1_INIT+ICW1_ICW4);
  io_wait();
  outb_port_value(PIC2_COMMAND, ICW1_INIT+ICW1_ICW4);
  io_wait();

  // Set Master PIC vector base
  outb_port_value(PIC1_DATA, master_vector_base);
  io_wait();

  // Set Slave PIC vector offset
  outb_port_value(PIC2_DATA, slave_vector_base);
  io_wait();

  // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
  outb_port_value(PIC1_DATA, 4);
  io_wait();

  // ICW3: tell Slave PIC its cascade identity (0000 0010)
  outb_port_value(PIC2_DATA, 2);
  io_wait();

  outb_port_value(PIC1_DATA, ICW4_8086);
  io_wait();
  outb_port_value(PIC2_DATA, ICW4_8086);
  io_wait();

  // restore saved masks.
  outb_port_value(PIC1_DATA, a1);   // restore saved masks.
  outb_port_value(PIC2_DATA, a2);
}

/**
 * Handle spurious interrupts
 *
 * Note there must be no EOI sent at the end of an spurious isr.
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 10.9 "Spurious Interrupt"
 * Figure 10-23. "Spurious-Interrupt vector Register"
 */
INTERRUPT_HANDLER
static void apic_spurious_interrupt_isr(struct intr_frame *frame) {
  (void)frame;
  // TODO: Maybe add a counter.
}

/**
 * Early APIC Initialization
 *
 * @return 0 if initialized, !0 if an error
 */
ac_uint apic_early_init(void) {
  ac_uint ret_val;
  ac_printf("apic_early_init:+present=%b\n", apic_present());

  // Get Processor info cpuid
  if (apic_present()) {
    // Remap the PIC vectors to 0x20 .. 0x2F
    remap_pic_vectors(0x20, 0x28);

    // Disable PIC
    outb_port_value(0xa1, 0xff);
    outb_port_value(0x21, 0xff);

    // Apic is present, map its physical page as uncachable.
    ac_u64 apic_phy_addr = get_apic_physical_addr();
    apic_lin_addr = (void*)apic_phy_addr; //0x00000001ffee00000;

    page_table_map_lin_to_phy(get_page_table_linear_addr(),
        apic_lin_addr, apic_phy_addr, FOUR_K_PAGE_SIZE,
        PAGE_CACHING_STRONG_UNCACHEABLE);

    // Below we enable the APIC, which can be done two different ways,
    // See "Intel 64 and IA-32 Architectures Software Developer's Manual"
    // Volume 3 chapter 10.4.3 "Enabling or Disabling the Local APIC"
    // Figure 10-21. "EOI Register".
    //
    // I'm choosing to enable spurious_vector since we set the vector anyway.
    struct apic_spurious_vector_fields svf = get_apic_spurious_vector();
    svf.vector = APIC_SPURIOUS_VECTOR;
#if 1
    // Enable apic in spurious_vector
    svf.apic_enable = AC_TRUE;
#else
    // Enable apic in msr
    struct msr_apic_base_fields abf = msr_get_apic_base();
    abf.e = AC_TRUE;
    msr_set_apic_base(abf);
#endif
    set_apic_spurious_vector(svf);
    set_intr_handler(APIC_SPURIOUS_VECTOR, apic_spurious_interrupt_isr);

    // Print tpr and ppr
    ac_printf("apic_early_init: tpr=0x%x ppr=0x%x\n",
        get_apic_tpr(), get_apic_ppr());

    ret_val = 0;
  } else {
    // No apic
    ret_val = 1;
  }

  ac_printf("apic_early_init:-ret_val=%d\n", ret_val);
  return ret_val;
}

/**
 * @return 0 if present, !0 if not
 */
ac_bool apic_present(void) {
  ac_u32 out_eax, out_ebx, out_ecx, out_edx;

  get_cpuid(1, &out_eax, &out_ebx, &out_ecx, &out_edx);
  return AC_GET_BITS(ac_uint, out_edx, 9, 1) == 1;
}

/**
 * @return id of the local apic
 */
ac_u32 get_apic_id(void) {
  ac_u32 out_eax, out_ebx, out_ecx, out_edx;

  get_cpuid(1, &out_eax, &out_ebx, &out_ecx, &out_edx);
  return AC_GET_BITS(ac_u32, out_ebx, 24, 8);
}

/**
 * @return physical address of local apic
 */
ac_u64 get_apic_physical_addr(void) {
  return msr_get_apic_base_physical_addr(msr_get_apic_base());
}
