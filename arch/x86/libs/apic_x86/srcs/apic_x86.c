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

#include <page_table_x86.h>
#include <page_table_x86_print.h>
#include <msr_x86.h>

#include <ac_inttypes.h>
#include <ac_bits.h>
#include <cpuid_x86.h>

#include <ac_printf.h>


void* apic_lin_addr;

/**
 * Initialize APIC
 *
 * @return 0 if initialized, !0 if an error
 */
ac_uint initialize_apic(void) {
  ac_uint ret_val;

  // Get Processor info cpuid
  if (apic_present()) {
    // Apic is present, map its phusical page as uncachable.
    ac_u64 apic_phy_addr = apic_get_physical_addr();
    apic_lin_addr = (void*)apic_phy_addr; //0x00000001ffee00000;

    page_table_map_lin_to_phy(get_page_table_linear_addr(),
        apic_lin_addr, apic_phy_addr, FOUR_K_PAGE_SIZE,
        PAGE_CACHING_STRONG_UNCACHEABLE);

    ret_val = 0;
  } else {
    // No apic
    ret_val = 1;
  }

  return ret_val;
}

/**
 * @return 0 if present, !0 if not
 */
ac_uint apic_present(void) {
  ac_u32 out_eax, out_ebx, out_ecx, out_edx;

  get_cpuid(1, &out_eax, &out_ebx, &out_ecx, &out_edx);
  return AC_GET_BITS(ac_uint, out_ecx, 9, 1);
}

/**
 * @return id of the local apic
 */
ac_u32 apic_get_id(void) {
  ac_u32 out_eax, out_ebx, out_ecx, out_edx;

  get_cpuid(1, &out_eax, &out_ebx, &out_ecx, &out_edx);
  return AC_GET_BITS(ac_u32, out_ebx, 24, 8);
}

/**
 * @return physical address of local apic
 */
ac_u64 apic_get_physical_addr(void) {
  return msr_apic_base_physical_addr(get_msr(MSR_APIC_BASE));
}
