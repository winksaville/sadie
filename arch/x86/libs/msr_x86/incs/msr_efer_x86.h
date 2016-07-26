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

/**
 * MSR Extended Feature Enables
 */

#ifndef ARCH_X86_MSR_X86_INCS_MSR_EFER_X86_H
#define ARCH_X86_MSR_X86_INCS_MSR_EFER_X86_H

#include <ac_attributes.h>
#include <ac_inttypes.h>


/**
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 35.1 "Architectural MSRS"
 * Table 35-2. "IA-32 Architectural MSRs"
 */
#define MSR_EFER   0xC0000080

/**
 * EFER ::= Extended Feature Enables Register
 */
struct AC_ATTR_PACKED msr_efer_fields {
  ac_u64 sce:1;            // syscall enable
  ac_u64 reserved_0:7;
  ac_u64 lme:1;            // long mode enable (R/W)
  ac_u64 reserved_1:1;
  ac_u64 lma:1;            // long mode active (R)
  ac_u64 nxe:1;            // Execute Disable Bit (R/W)
  ac_u64 reserved_2:52;
};

_Static_assert(sizeof(struct msr_efer_fields) == sizeof(ac_u64),
    L"struct msr_efer_fields is not 8 bytes");

union msr_efer_u {
  ac_u64 raw;
  struct msr_efer_fields fields;
};

_Static_assert(sizeof(union msr_apic_base_u) == sizeof(ac_u64),
    L"union msr_efer_u is not 8 bytes");


#endif
