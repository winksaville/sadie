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

#include <apic_x86.h>

#include <cpuid_x86.h>
#include <interrupts_x86.h>
#include <msr_x86.h>
#include <print_msr.h>

#include <ac_bits.h>
#include <ac_inttypes.h>
#include <ac_printf.h>
#include <ac_test.h>

ac_bool test_apic() {
  ac_bool error = AC_FALSE;

  ac_u64 msr_apic_base = get_msr(MSR_APIC_BASE);
  error |= AC_TEST(msr_apic_base != 0);

  print_msr(MSR_APIC_BASE, msr_apic_base);

  ac_u64 phy_addr = apic_get_physical_addr();
  ac_printf(" phy_addr=0x%llx\n", phy_addr);
  error |= AC_TEST(phy_addr != 0);

  ac_u32 local_id = apic_get_id();
  ac_printf(" local_id=0x%x\n", local_id);

  return error;
}

ac_bool test_apic_version() {
  ac_bool error = AC_FALSE;

  ac_printf("test_apic_version\n");
  ac_u64 msr_apic_base = get_msr(MSR_APIC_BASE);
  print_msr(MSR_APIC_BASE, msr_apic_base);
  ac_u32* ver_reg = (ac_u32*)((ac_uptr)msr_apic_base_physical_addr(msr_apic_base) + 0x30);
  ac_printf("ver_reg=0x%p\n", ver_reg);
  ac_printf("*ver_reg=0x%x\n", *ver_reg);

#ifdef CPU_X86_64
  // Not sure if this is always true will probably
  // need to be altered in the future.
  error |= AC_TEST(*ver_reg == 0x01050014);
#else
  // Not sure what X86_32 should be
#endif

  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  // Initialize interrupt dt for now since its not done by default yet
  // as this test suite has failed :)
  initialize_intr_descriptor_table();

  initialize_apic();

  error |= test_apic();
  error |= test_apic_version();

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
