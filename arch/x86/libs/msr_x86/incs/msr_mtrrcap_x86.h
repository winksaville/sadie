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

#ifndef ARCH_X86_MSR_X86_INCS_MSR_MTRRCAP_X86_H
#define ARCH_X86_MSR_X86_INCS_MSR_MTRRCAP_X86_H

#include <ac_attributes.h>
#include <ac_inttypes.h>


/**
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 35.1 "Architectural MSRS"
 * Table 35-2. "IA-32 Architectural MSRs"
 */
#define MSR_MTRRCAP     0xFE

/**
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 10.12.1 "Detecting and Enabling x2APIC Mode"
 * Figure 10-26 "IA32_APIC_BASE MSR Supporting x2APIC"
 */
struct AC_ATTR_PACKED msr_mtrrcap_fields {
  ac_u64 vcnt:8;
  ac_u64 fix:1;
  ac_u64 reserved_0:1;
  ac_u64 wc:1;
  ac_u64 smrr:1;
  ac_u64 reserved_1:52;
};

_Static_assert(sizeof(struct msr_mtrrcap_fields) == sizeof(ac_u64),
    L"struct msr_mtrrcap_fields is not 8 bytes");

union msr_mtrrcap_u {
  ac_u64 raw;
  struct msr_mtrrcap_fields fields;
};

_Static_assert(sizeof(union msr_mtrrcap_u) == sizeof(ac_u64),
    L"union msr_mtrrcap_u is not 8 bytes");

#endif

