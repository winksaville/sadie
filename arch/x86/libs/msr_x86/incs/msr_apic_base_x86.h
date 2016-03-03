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

#ifndef ARCH_X86_MSR_X86_INCS_MSR_APIC_BASE_X86_H
#define ARCH_X86_MSR_X86_INCS_MSR_APIC_BASE_X86_H

#include <cpuid_x86.h>
#include <msr_x86.h>

#include <ac_inttypes.h>
#include <ac_bits.h>


/**
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 35.1 "Architectural MSRS"
 * Table 35-2. "IA-32 Architectural MSRs"
 */
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


static __inline__ struct msr_apic_base_fields msr_get_apic_base(void) {
    union msr_apic_base_u abu = { .raw = get_msr(MSR_APIC_BASE) };
    return abu.fields;
}

static __inline__ void msr_set_apic_base(struct msr_apic_base_fields abf) {
    union msr_apic_base_u abu = { .fields = abf };
    return set_msr(MSR_APIC_BASE, abu.raw);
}

/** APIC phyiscal address */
static __inline__ ac_u64 msr_get_apic_base_physical_addr(struct msr_apic_base_fields abf) {
  return abf.base_addr << 12 & AC_BIT_MASK(ac_u64, cpuid_max_physical_address_bits());
}

#endif
