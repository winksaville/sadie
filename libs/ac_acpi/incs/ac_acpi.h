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

#ifndef SADIE_LIBS_AC_ACPI_INCS_AC_APCI_H
#define SADIE_LIBS_AC_ACPI_INCS_AC_APCI_H

#include <ac_assert.h>
#include <ac_inttypes.h>

#define RSDP_SIGNATURE "RSDP PTR "

/**
 * Root System Descriptrion Pointer
 * See: [Advanced Configuration and Power Interface Specification v6.1]
 * (https://goo.gl/G1ov65)
 * section 5.2.5.3 "RSDP Structure".
 */
struct ac_acpi_rsdp {
  ac_u8 signature[8];
  ac_u8 check_sum;
  ac_u8 oem_id[6];
  ac_u8 revision;
  ac_u32 rsdt_address;
  ac_u32 length;
  ac_u64 xsdt_address;
  ac_u8 extended_check_sum;
  ac_u8 resv[3];  
} __attribute__((__packed__));

typedef struct ac_acpi_rsdp ac_acpi_rsdp;

ac_static_assert(sizeof(ac_acpi_rsdp) == 36, L"ac_acpi_rsdp is not 36 bytes");

/*********************************************************
 * The following are declared in ac_acpi_impl.h
 */
#include <ac_acpi_impl.h>

/**
 * Get the acpi rsdp
 *
 * @return AC_NULL if rsdp does not exist
 */
//ac_acpi_rsdp* ac_acpi_rsdp_get(void);

/*********************************************************/

#endif
