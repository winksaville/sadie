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

#include <msr_x86.h>

#include <ac_inttypes.h>
#include <ac_printf.h>

#define IA32_ENERGY_PERF_BIAS   0x1B0
#define IA32_MISC_ENABLE        0x1A0
#define MSR_MISC_PWR_MGMT       0x1AA
#define IA32_PERF_STATUS        0x198
#define IA32_PERF_CTL           0x199


/**
 * Display perf power msr
 */
static void display_perf_power_msrs(void) {
  ac_u64 ia32_misc_enable;
  ac_u64 ia32_perf_status;
  ac_u64 ia32_perf_ctl;

  ia32_misc_enable = get_msr(IA32_MISC_ENABLE);
  ia32_perf_status = get_msr(IA32_PERF_STATUS);

  // For some reason get/set msr of IA32_PERF_CTL
  // causes a GP fault on qemu-system-x86_64 when
  // using kvm. Two solutions turn on ignore_msrs in
  // kvm See https://bugs.launchpad.net/qemu/+bug/1208540
  // which says do:
  //   sudo sh -c "echo 1 >/sys/module/kvm/parameters/ignore_msrs"
  //
  // What I did was change tests/meson.build to not use kvm host
  // as the cpu.
  ia32_perf_ctl = get_msr(IA32_PERF_CTL);

  ac_printf("IA32_MISC_ENABLE       %x = 0x%lx\n",
      IA32_MISC_ENABLE, ia32_misc_enable);
  ac_printf("IA32_PERF_STATUS       %x = 0x%lx\n",
      IA32_PERF_STATUS, ia32_perf_status);
  ac_printf("IA32_PERF_CTL          %x = 0x%lx\n",
      IA32_PERF_CTL, ia32_perf_ctl);
}

/**
 * Initalize perf_power module
 */
ac_bool msr_perf_power_x86_init(void) {
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
    ac_printf("IA32_ENERGY_PERF_BIAS is not supported\n");
  }
  ac_u64 ia32_misc_enable;
  ac_u64 ia32_perf_status;
  ac_u64 ia32_perf_ctl;

  display_perf_power_msrs();

  ia32_misc_enable = get_msr(IA32_MISC_ENABLE);
  set_msr(IA32_MISC_ENABLE, ia32_misc_enable | 0x10000ll);

  ia32_perf_status = get_msr(IA32_PERF_STATUS);
  ia32_perf_ctl = ia32_perf_status & 0xff00ll;
  set_msr(IA32_PERF_CTL, ia32_perf_ctl);

  display_perf_power_msrs();

  return error;
}
