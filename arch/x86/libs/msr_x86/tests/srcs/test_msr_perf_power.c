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

#include <test_msr_x86.h>

#include <msr_x86.h>
#include <print_msr.h>

#include <ac_bits.h>
#include <ac_inttypes.h>
#include <ac_printf.h>
#include <ac_test.h>

#define IA32_ENERGY_PERF_BIAS   0x1B0
#define IA32_MISC_ENABLE        0x1A0
#define MSR_MISC_PWR_MGMT       0x1AA
#define IA32_PERF_STATUS        0x198
#define IA32_PERF_CTL           0x199

ac_bool test_msr_perf_power() {
  ac_bool error = AC_FALSE;

  ac_u32 out_eax, out_ebx, out_ecx, out_edx;

  get_cpuid(6, &out_eax, &out_ebx, &out_ecx, &out_edx);
  ac_printf("Thermal and Power Management bits eax=0x%x ebx=0x%x ecx=0x%x edx=0x%x\n",
     out_eax, out_ebx, out_ecx, out_edx);

  if ((out_ecx & 0x8) == 0x80) { 
    ac_u64 msr_misc_pwr_mgmt = get_msr(MSR_MISC_PWR_MGMT);
    ac_printf("MISC_PWR_MGMT         %x = 0x%lx\n",
        MSR_MISC_PWR_MGMT, msr_misc_pwr_mgmt);

    // Enable IA32_ENERGY_PERF_BIAS
    set_msr(MSR_MISC_PWR_MGMT, msr_misc_pwr_mgmt | 0x2);

    msr_misc_pwr_mgmt = get_msr(MSR_MISC_PWR_MGMT);
    ac_printf("MISC_PWR_MGMT         %x = 0x%lx\n",
        MSR_MISC_PWR_MGMT, msr_misc_pwr_mgmt);

    ac_printf("IA32_ENERGY_PERF_BIAS %x = 0x%lx\n",
        IA32_ENERGY_PERF_BIAS, get_msr(IA32_ENERGY_PERF_BIAS));
  } else {
    ac_printf("IA32_ENERGY_PERF_BIAS is not supported");
  }
  ac_printf("IA32_MISC_ENABLE       %x = 0x%lx\n",
      IA32_MISC_ENABLE, get_msr(IA32_MISC_ENABLE));
  ac_printf("IA32_PERF_STATUS       %x = 0x%lx\n",
      IA32_PERF_STATUS, get_msr(IA32_PERF_STATUS));
  ac_printf("IA32_PERF_CTL          %x = 0x%lx\n",
      IA32_PERF_CTL, get_msr(IA32_PERF_CTL));

  return error;
}
