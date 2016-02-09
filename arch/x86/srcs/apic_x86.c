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

#include <ac_inttypes.h>
#include <ac_bits.h>
#include <cpuid_x86.h>
#include <msr_x86.h>

#include <ac_printf.h>


ac_uint initialize_apic(void) {
  ac_uint ret_val;

  // Get Processor info cpuid
  if (apic_present()) {
    // Apic is present
    ret_val = 0;
  } else {
    // No apic
    ret_val = 1;
  }

  return ret_val;
}

ac_uint apic_present(void) {
  ac_u32 out_eax, out_ebx, out_ecx, out_edx;
  get_cpuid(1, &out_eax, &out_ebx, &out_ecx, &out_edx);
  return AC_GET_BITS(ac_uint, out_ecx, 9, 1);
}

ac_u32 apic_get_id(void) {
  ac_u32 out_eax, out_ebx, out_ecx, out_edx;
  get_cpuid(1, &out_eax, &out_ebx, &out_ecx, &out_edx);
  return AC_GET_BITS(ac_u32, out_ebx, 24, 8);
}


ac_u64 apic_get_physical_addr(void) {
  ac_u64 apic_base = get_msr(MSR_IA32_APIC_BASE); 
  ac_u32 num_apic_base_bits = cpuid_max_physical_address_bits() - 12;
  ac_u64 phy_addr = AC_GET_BITS(ac_u64, apic_base, 12, num_apic_base_bits) << 12;
  return phy_addr;
}
