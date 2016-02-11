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

#ifndef ARCH_X86_MSR_X86_INCS_MSR_X86_H
#define ARCH_X86_MSR_X86_INCS_MSR_X86_H

#include <cpuid_x86.h>

#include <ac_inttypes.h>
#include <ac_bits.h>


#define MSR_APIC_BASE   0x1B

/**
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 10.12.1 "Detecting and Enabling x2APIC Mode"
 * Figure 10-26 "IA32_APIC_BASE MSR Supporting x2APIC"
 */
struct msr_apic_base_fields {
  ac_u64 reserved_0:8;
  ac_u64 bsp:1;
  ac_u64 reserved_1:1;
  ac_u64 extd:1;
  ac_u64 e:1;
  ac_u64 base_addr:52;
} __attribute__((__packed__));

_Static_assert(sizeof(struct msr_apic_base_fields) == sizeof(ac_u64),
    L"struct msr_apic_base_fields is not 8 bytes");

union msr_apic_base_u {
  ac_u64 raw;
  struct msr_apic_base_fields fields;
};

_Static_assert(sizeof(union msr_apic_base_u) == sizeof(ac_u64),
    L"union msr_apic_base_u is not 8 bytes");

/** APIC phyiscal address */
static __inline__ ac_u64 msr_apic_base_physical_addr(ac_u64 msr_apic_base) {
  return AC_GET_BITS(ac_u64, msr_apic_base, 12,
            cpuid_max_physical_address_bits() - 12) << 12;
}

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
