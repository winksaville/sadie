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

#include <cpuid_x86.h>

#include <ac_bits.h>
#include <ac_inttypes.h>
#include <ac_printf.h>

ac_uint cpuid_max_physical_address_bits() {
  ac_u32 out_eax, out_ebx, out_ecx, out_edx;
  get_cpuid(0x80000008, &out_eax, &out_ebx, &out_ecx, &out_edx);
  if (out_eax == 0) {
    // As per "Intel 64 and IA-32 Architectures Software Devemoper's Manual, Volume 3A"
    // Chapter 10.4.4 Local APIC Status and Location" foot note 1.

    return 36;
  } else {
    return AC_GET_BITS(ac_u32, out_eax, 0, 8);
  }
}

ac_uint cpuid_max_linear_address_bits() {
  ac_u32 out_eax, out_ebx, out_ecx, out_edx;
  get_cpuid(0x80000008, &out_eax, &out_ebx, &out_ecx, &out_edx);
  if (out_eax == 0) {
    // If unknown we'll return max_physical address
    return cpuid_max_physical_address_bits();
  } else {
    return AC_GET_BITS(ac_uint, out_eax, 8, 8);
  }
}
