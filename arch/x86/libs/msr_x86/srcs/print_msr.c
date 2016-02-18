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

#include <print_msr.h>

#include <msr_x86.h>

#include <ac_inttypes.h>
#include <ac_printf.h>

void print_msr(ac_u32 msr, ac_u64 value) {
  switch (msr) {
    case MSR_APIC_BASE: print_msr_apic_base(value); break;
    case MSR_EFER: print_msr_efer(value); break;
    case MSR_MTRRCAP: print_msr_mtrrcap(value); break;
    default: ac_printf("unknown msr 0x%x: 0x%llx\n", msr, value);
  }
}
