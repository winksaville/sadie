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
#include <msr_x86.h>
#include <page_table_x86.h>
#include <page_table_x86_print.h>

#include <ac_inttypes.h>
#include <ac_bits.h>
#include <cpuid_x86.h>

#include <ac_printf.h>


void* apic_lin_addr;

/**
 * Handle spurious interrupts
 *
 * Note there must be no EOI sent at the end of an spurious isr.
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 10.9 "Spurious Interrupt"
 * Figure 10-23. "Spurious-Interrupt vector Register"
 */
__attribute__ ((__interrupt__))
static void apic_spurious_interrupt_isr(struct intr_frame *frame) {
  (void)frame;
  // TODO: Maybe add a counter.
}

/**
 * Initialize APIC
 *
 * @return 0 if initialized, !0 if an error
 */
ac_uint initialize_apic(void) {
  ac_uint ret_val;

  // Get Processor info cpuid
  if (apic_present()) {
    ac_printf("APIC present\n");

    // Apic is present, map its phusical page as uncachable.
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


    ret_val = 0;
  } else {
    // No apic
    ac_printf("APIC NOT present\n");
    ret_val = 1;
  }

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
