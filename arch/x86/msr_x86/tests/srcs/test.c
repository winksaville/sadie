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

#include <msr_x86.h>
#include <cpuid_x86.h>

#include <ac_bits.h>
#include <ac_printf.h>
#include <ac_test.h>

ac_u32 max_physical_address_bits() {
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

ac_u32 max_linear_address_bits() {
  ac_u32 out_eax, out_ebx, out_ecx, out_edx;
  get_cpuid(0x80000008, &out_eax, &out_ebx, &out_ecx, &out_edx);
  if (out_eax == 0) {
    // As per "Intel 64 and IA-32 Architectures Software Devemoper's Manual, Volume 3A"
    // Chapter 10.4.4 Local APIC Status and Location" foot note 1.
    return 36;
  } else {
    return AC_GET_BITS(ac_u32, out_eax, 8, 8);
  }
}

ac_bool test_rdwrmsr() {
  ac_bool error = AC_FALSE;

  ac_u64 msr_apic_base = get_msr(MSR_IA32_APIC_BASE);
  ac_printf("msr_apic_base=%llx\n",msr_apic_base);
  ac_printf(" bsp=%b\n", AC_GET_BITS(ac_u32, msr_apic_base, 8, 1));
  ac_printf(" ign=%b\n", AC_GET_BITS(ac_u32, msr_apic_base, 9, 1));
  ac_printf(" extd=%b\n", AC_GET_BITS(ac_u32, msr_apic_base, 10, 1));
  ac_printf(" e=%b\n", AC_GET_BITS(ac_u32, msr_apic_base, 11, 1));
  ac_printf(" max_physical_address_bits=%d\n", max_physical_address_bits());
  ac_printf(" max_linear_address_bits=%d\n", max_linear_address_bits());
  ac_printf(" base=%llx\n", AC_GET_BITS(ac_u64, msr_apic_base, 12, max_physical_address_bits() - 12) << 12);

  error |= AC_TEST(msr_apic_base != 0);

  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  error |= test_rdwrmsr();

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
