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

#ifndef ARCH_X86_INCS_MSR_X86_H
#define ARCH_X86_INCS_MSR_X86_H

#include <ac_inttypes.h>

#define MSR_APIC_BASE   0x1B

/** Get msr */
static __inline__ ac_u64 get_msr(ac_u32 msr) {
  ac_u32 lo;
  ac_u32 hi;
  __asm__ volatile("movl  %0, %%ecx\n" :: "g"(msr));
  __asm__ volatile("rdmsr\n" : "=d"(hi), "=a"(lo));
  return (ac_u64)hi << 32 | (ac_u64)lo;
}

/** Set an MSR */
static __inline__ void set_msr(ac_u32 msr, ac_u64 value) {
  ac_u32 eax = (ac_u32)value;
  ac_u32 edx = (ac_u32)(value >> 32);
  __asm__ volatile("movl  %0, %%ecx\n" :: "g"(msr));
  __asm__ volatile("wrmsr"
      :: "a"(eax), "d"(edx));
}

#endif
