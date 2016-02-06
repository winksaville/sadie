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

#ifndef ARCH_X86_INCS_CPUID_X86_H
#define ARCH_X86_INCS_CPUID_X86_H

#include <ac_inttypes.h>

/** CPUID on a sublead with leaf in eax and subleaf in ecx */
static __inline__ void  get_cpuid_subleaf(ac_u32 leaf_eax, ac_u32 subleaf_ecx,
    ac_u32 *out_eax, ac_u32 *out_ebx, ac_u32* out_ecx, ac_u32* out_edx) {
  ac_u32 eax, ebx, ecx, edx;
  __asm__ volatile("cpuid"
      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
      : "0"(leaf_eax), "2"(subleaf_ecx));
  *out_eax = eax;
  *out_ebx = ebx;
  *out_ecx = ecx;
  *out_edx = edx;
}

/** CPUID with the leaf in eax */
static __inline__ void  get_cpuid(ac_u32 leaf_eax,
    ac_u32 *out_eax, ac_u32 *out_ebx, ac_u32* out_ecx, ac_u32* out_edx) {
  ac_u32 eax, ebx, ecx, edx;
  __asm__ volatile("cpuid"
      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
      : "0"(leaf_eax));
  *out_eax = eax;
  *out_ebx = ebx;
  *out_ecx = ecx;
  *out_edx = edx;
}

#endif
