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

#ifndef ARCH_X86_CPUID_X86_INCS_CPUID_X86_H
#define ARCH_X86_CPUID_X86_INCS_CPUID_X86_H

#include <ac_inttypes.h>

/**
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 2 chapter 3.2 "Instructions (A-M)" CPUID
 * Table 3-17. "Information Returned by CPUID Instruction"
 */

/** CPUID on a sublead with leaf in eax and subleaf in ecx */
static __inline__ void  get_cpuid_subleaf(ac_u32 leaf_eax, ac_u32 subleaf_ecx,
    ac_u32 *out_eax, ac_u32 *out_ebx, ac_u32* out_ecx, ac_u32* out_edx) {
  __asm__ volatile("cpuid"
      : "=a"(*out_eax), "=b"(*out_ebx), "=c"(*out_ecx), "=d"(*out_edx)
      : "0"(leaf_eax), "2"(subleaf_ecx));
}

/** CPUID with the leaf in eax */
static __inline__ void  get_cpuid(ac_u32 leaf_eax,
    ac_u32 *out_eax, ac_u32 *out_ebx, ac_u32* out_ecx, ac_u32* out_edx) {
  __asm__ volatile("cpuid"
      : "=a"(*out_eax), "=b"(*out_ebx), "=c"(*out_ecx), "=d"(*out_edx)
      : "0"(leaf_eax));
}


/**
 * Get the vendor_id, the size should be at least 13.
 */
void cpuid_get_vendor_id(char* vendor_id, ac_uint size_vendor_id);

/**
 * @return the maximum leaf
 */
ac_u32 cpuid_max_leaf(void);

/**
 * @return the maximum extended leaf
 */
ac_u32 cpuid_max_extd_leaf(void);

/**
 * @return the maximum number of physical address bits
 */
ac_u32 cpuid_max_physical_address_bits();

/**
 * @return the maximum number of linear address bits
 */
ac_u32 cpuid_max_linear_address_bits();

#endif
