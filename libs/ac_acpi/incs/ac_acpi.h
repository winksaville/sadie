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

#define AC_ACPI_RSDP_SIGNATURE "RSD PTR "

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

ac_static_assert(sizeof(ac_acpi_rsdp) == 36,
    L"ac_acpi_rsdp is not 36 bytes");

/**
 * System Description header
 *
 * See: [Advanced Configuration and Power Interface Specification v6.1]
 * (https://goo.gl/G1ov65)
 * section 5.2.7 "Root System Description Table" and
 * section 5.2.8 "Extended System Description Table".
 */
struct ac_acpi_desc_hdr {
  ac_u8 signature[4];
  ac_u32 length;
  ac_u8 revision;
  ac_u8 check_sum;
  ac_u8 oem_id[6];       // String (Null terminated if < 6 bytes)
  ac_u8 oem_table_id[8]; // String (NULL terminated if < 8 bytes)
  ac_u32 oem_revision;
  ac_u8 creator_id[4];
  ac_u32 creator_revision;
  ac_u8 table[];
} __attribute__((__packed__));

typedef struct ac_acpi_desc_hdr ac_acpi_desc_hdr;

ac_static_assert(sizeof(ac_acpi_desc_hdr) == 36,
    L"ac_acpi_desc_hdr is not 36 bytes");

/**
 * MADT type_len
 *
 * See: [Advanced Configuration and Power Interface Specification v6.1]
 * (https://goo.gl/G1ov65)
 * Table 5-46. "Interrupt Controller Structure Types"
 * Table 5-47. "Processor Local APIC Structure"
 *
 */
struct ac_acpi_madt_tl {
  ac_u8 type;   // Table 5-46
  ac_u8 len;    // Length of the table incluing madt_tl
} __attribute__((__packed__));

typedef struct ac_acpi_madt_tl ac_acpi_madt_tl;

ac_static_assert(sizeof(ac_acpi_madt_tl) == 2,
    L"ac_acpi_madt_tl is not 2 bytes");

// Values for ac_acpi_madt_tl.type field
#define MADT_TYPE_LOCAL_APIC                0
#define MADT_TYPE_IO_APIC                   1
#define MADT_TYPE_INTR_SOURCE_OVERRIDE      2
#define MADT_TYPE_NMI_SOURCE                3
#define MADT_TYPE_LOCAL_APIC_NMI            4
#define MADT_TYPE_LOCAL_APIC_ADDR_OVERRIDE  5
#define MADT_TYPE_IO_SAPIC                  6
#define MADT_TYPE_LOCAL_SAPIC               7
#define MADT_TYPE_PLATFOMR_INTR_SOURCES     8
#define MADT_TYPE_LOCAL_X2APIC              9
#define MADT_TYPE_LOCAL_X2APIC_NMI          10

/**
 * Processor Local APIC
 *
 * See: [Advanced Configuration and Power Interface Specification v6.1]
 * (https://goo.gl/G1ov65)
 * section 5.2.12.2 "Processor Local APIC Structure"
 */
struct ac_acpi_madt_local_apic {
  ac_acpi_madt_tl tl;          // .type = 0 MADT_TYPE_LOCAL_APIC
  ac_u8 proc_uid;
  ac_u8 id;
  ac_bool enabled:1;            // See Table 5-48
  ac_u32 resv:31;
} __attribute__((__packed__));

typedef struct ac_acpi_madt_local_apic ac_acpi_madt_local_apic;

ac_static_assert(sizeof(ac_acpi_madt_local_apic) == 8,
    L"ac_acpi_madt_local_apic is not 8 bytes");

/**
 * Processor Local SAPIC
 *
 * See: [Advanced Configuration and Power Interface Specification v6.1]
 * (https://goo.gl/G1ov65)
 * section 5.2.12.10 "Processor Local SAPIC Structure"
 */
struct ac_acpi_madt_local_sapic {
  ac_acpi_madt_tl tl;           // .type = 7 MADT_TYPE_LOCAL_SAPIC
  ac_u8 proc_id;
  ac_u8 id;
  ac_u8 eid;
  ac_u8 resv0[3];
  ac_bool enabled:1;            // See Table 5-48
  ac_u32 resv1:31;
  ac_u32 proc_uid;
  ac_u8 proc_uid_str[1];        // Null termineated string at least one byte
} __attribute__((__packed__));

typedef struct ac_acpi_madt_local_sapic ac_acpi_madt_local_sapic;

ac_static_assert(sizeof(ac_acpi_madt_local_sapic) == 17,
    L"ac_acpi_madt_local_sapic is not 17 bytes");

/**
 * Processor Local x2APIC
 *
 * See: [Advanced Configuration and Power Interface Specification v6.1]
 * (https://goo.gl/G1ov65)
 * section 5.2.12.12 "Processor Local x2APIC Structure"
 */
struct ac_acpi_madt_local_x2apic {
  ac_acpi_madt_tl tl;           // .type = 9 MADT_TYPE_LOCAL_X2APIC
  ac_u8 resv0[2];
  ac_u32 id;
  ac_bool enabled:1;            // See Table 5-48
  ac_u32 resv1:31;
  ac_u32 proc_uid;
} __attribute__((__packed__));

typedef struct ac_acpi_madt_local_x2apic ac_acpi_madt_local_x2apic;

ac_static_assert(sizeof(ac_acpi_madt_local_x2apic) == 16,
    L"ac_acpi_madt_local_x2apic is not 16 bytes");

/**
 * IO APIC
 *
 * See: [Advanced Configuration and Power Interface Specification v6.1]
 * (https://goo.gl/G1ov65)
 * section 5.2.12.3. "Processor Local APIC Structure"
 */
struct ac_acpi_madt_io_apic {
  ac_acpi_madt_tl tl;          // .type = 1 MADT_TYPE_IO_APIC
  ac_u8 id;
  ac_u8 resv;
  ac_u32 addr;
  ac_u32 global_system_intr_base;
} __attribute__((__packed__));

typedef struct ac_acpi_madt_io_apic ac_acpi_madt_io_apic;

ac_static_assert(sizeof(ac_acpi_madt_io_apic) == 12,
    L"ac_acpi_madt_io_apic is not 12 bytes");

/**
 * IO SAPIC
 *
 * See: [Advanced Configuration and Power Interface Specification v6.1]
 * (https://goo.gl/G1ov65)
 * section 5.2.12.9. "Processor IO SAPIC Structure"
 */
struct ac_acpi_madt_io_sapic {
  ac_acpi_madt_tl tl;          // .type = 6 MADT_TYPE_IO_SAPIC
  ac_u8 id;
  ac_u8 resv;
  ac_u32 global_system_intr_base;
  ac_u64 addr;
} __attribute__((__packed__));

typedef struct ac_acpi_madt_io_sapic ac_acpi_madt_io_sapic;

ac_static_assert(sizeof(ac_acpi_madt_io_sapic) == 16,
    L"ac_acpi_madt_io_sapic is not 16 bytes");

/**
 * Local APIC Address Override
 *
 * See: [Advanced Configuration and Power Interface Specification v6.1]
 * (https://goo.gl/G1ov65)
 * section 5.2.12.8. "Local APIC Address Override Structure"
 */
struct ac_acpi_madt_local_apic_addr_override {
  ac_acpi_madt_tl tl;          // .type = 5 MADT_TYPE_LOCAL_APIC_ADDR_OVERRIDE
  ac_u8 resv[2];
  ac_u64 addr;
} __attribute__((__packed__));

typedef struct ac_acpi_madt_local_apic_addr_override
        ac_acpi_madt_local_apic_addr_override;

ac_static_assert(sizeof(ac_acpi_madt_local_apic_addr_override) == 12,
    L"ac_acpi_madt_local_apic_addr_override is not 12 bytes");

/**
 * Interrupt Source Override
 *
 * See: [Advanced Configuration and Power Interface Specification v6.1]
 * (https://goo.gl/G1ov65)
 * section 5.2.12.5 "Interrupt Source Override Structure"
 */
struct ac_acpi_madt_intr_source_override {
  ac_acpi_madt_tl tl;          // .type = 2 MADT_TYPE_INTR_SOURCE_OVERRIDE
  ac_u8 bus;
  ac_u8 source;
  ac_u32 global_system_intr;
  ac_u8 polarity:2;
  ac_u8 trigger_mode:2;
  ac_u16 resv:12;
} __attribute__((__packed__));

typedef struct ac_acpi_madt_intr_source_override
        ac_acpi_madt_intr_source_override;

ac_static_assert(sizeof(ac_acpi_madt_intr_source_override) == 10,
    L"ac_acpi_madt_intr_source_override is not 10 bytes");

/**
 * Local APIC NMI
 *
 * See: [Advanced Configuration and Power Interface Specification v6.1]
 * (https://goo.gl/G1ov65)
 * section 5.2.12.7. "Local APIC NMI Structure"
 */
struct ac_acpi_madt_local_apic_nmi {
  ac_acpi_madt_tl tl;          // .type = 4 MADT_TYPE_LOCAL_APIC_NMI
  ac_u8 proc_uid;
  ac_u8 polarity:2;
  ac_u8 trigger_mode:2;
  ac_u16 resv:12;
  ac_u8 lint;
} __attribute__((__packed__));

typedef struct ac_acpi_madt_local_apic_nmi
        ac_acpi_madt_local_apic_nmi;

ac_static_assert(sizeof(ac_acpi_madt_local_apic_nmi) == 6,
    L"ac_acpi_madt_local_apic_nmi is not 6 bytes");

/**
 * Multiple APIC Descripton Table
 *
 * See: [Advanced Configuration and Power Interface Specification v6.1]
 * (https://goo.gl/G1ov65)
 * section 5.2.12 "Multiple APIC Description Table (MADT)"
 */
struct ac_acpi_madt {
  ac_acpi_desc_hdr hdr;
  ac_u32 addr;
  ac_bool pcat_comp:1;
  ac_u32  resv:31;
  ac_u8   table[];
} __attribute__((__packed__));

typedef struct ac_acpi_madt ac_acpi_madt;



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
