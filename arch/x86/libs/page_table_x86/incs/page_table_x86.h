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
 * Page Table x86
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 4 "Paging"
 */

#ifndef ARCH_X86_PAGE_TABLE_X86_INCS_PAGE_TABLE_X86_H
#define ARCH_X86_PAGE_TABLE_X86_INCS_PAGE_TABLE_X86_H

#include <cr_x86.h>

#include <ac_inttypes.h>

/**
 * Page mode
 *
 * Although in 4.1.1 it says there are 3 paging modes when you look
 * at 4.5 IA-32e Paging there it has two modes although from a CR3
 * PAGE_MODE_NRML_32BIT and PAGE_MODE_NRML_64BIT are the same I thought
 * I'd make them different modes.
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 4.1.1 "Three Paging Modes"
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 4.5 "IA-32 Paging"
 */
enum page_mode {
  PAGE_MODE_UNKNOWN,
  PAGE_MODE_NRML_32BIT,
  PAGE_MODE_PAE_32BIT,
  PAGE_MODE_NRML_64BIT,
  PAGE_MODE_PCIDE_64BIT,
};

#define PML4E_FIELDS_SIZE 8
#define PDPTE_FIELDS_SIZE 8
#define PDE_FIELDS_SIZE 8
#define PTE_FIELDS_SIZE 8

/**
 * CR3 normal paging fields
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 4.3 "32-bit Paging"
 * Table 4-3. "Use of CR# with 32-bit Paging"
 *
 * Volume 3 chapter 4.5 "IA-32e Paging"
 * Table 4-12. "Use of CR3 with IA-32e Paging and CR4.PCIDE = 0"
 */
struct cr3_nrml_paging_fields {
  ac_uint reserved_0:3;
  ac_uint pwt:1;
  ac_uint pcd:1;
  ac_uint reserved_1:7;
#ifdef CPU_X86_64
  ac_uint page_directory_base:52;
#else /* CPU_X86_32 */
  ac_uint page_directory_base:20;
#endif
} __attribute__((__packed__));

_Static_assert(sizeof(struct cr3_nrml_paging_fields) == CR3_FIELDS_SIZE,
    L"cr3_nrml_paging_fields is not " AC_XSTR(CR3_FIELDS_SIZE) " bytes");

/**
 * CR3 pae paging fields
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 4.4 "PAE Paging"
 * Table 4-7. "Use of CR3 with PAE Paging"
 */
struct cr3_pae_paging_fields {
  ac_uint reserved_0:5;
#ifdef CPU_X86_64
  ac_uint page_directory_base:59;
#else /* CPU_X86_32 */
  ac_uint page_directory_base:27;
#endif
} __attribute__((__packed__));

_Static_assert(sizeof(struct cr3_pae_paging_fields) == CR3_FIELDS_SIZE,
    L"cr3_pae_paging_fields is not " AC_XSTR(CR3_FIELDS_SIZE) " bytes");

/**
 * CR3 pcide paging fields
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 4.5 "IA-32e Paging"
 * Table 4-13. "Use of CR3 with IA-32e Paging and CR4.PCIDE = 1"
 */
struct cr3_pcide_paging_fields {
  ac_uint pcid:12;
#ifdef CPU_X86_64
  ac_uint page_directory_base:52;
#else /* CPU_X86_32 */
  ac_uint page_directory_base:20;
#endif
} __attribute__((__packed__));

_Static_assert(sizeof(struct cr3_pcide_paging_fields) == CR3_FIELDS_SIZE,
    L"cr3_pcide_paging_fields is not " AC_XSTR(CR3_FIELDS_SIZE) " bytes");

union cr3_paging_fields_u {
  ac_uint raw;
  struct cr3_nrml_paging_fields nrml_paging_fields;
  struct cr3_pae_paging_fields pae_paging_fields;
  struct cr3_pcide_paging_fields pcide_paging_fields;
};

_Static_assert(sizeof(union cr3_paging_fields_u) == CR3_FIELDS_SIZE,
    L"cr3_paging_fields_u is not " AC_XSTR(CR3_FIELDS_SIZE) " bytes");


/**
 * PML4, Page Map Level 4
 *
 * The PML4 table is the top most table for X86_64 (IA-32e) modes.
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 4.5 "IA-32e Paging"
 * Table 4-14. "Format of an IA-32e PML4 Entry that References a
 * Page-Directory-Pointer Table"
 */

struct pml4e_fields {
  ac_u64 p:1;                   // Present
  ac_u64 rw:1;                  // Read write
  ac_u64 us:1;                  // User/Supervisor
  ac_u64 pwt:1;                 // Page-level write-through
  ac_u64 pcd:1;                 // Page-level cache disable
  ac_u64 a:1;                   // Accessed
  ac_u64 reserved_0:6;
  ac_u64 phy_addr:51;           // Physical address of 4K aligned PDPE
  ac_u64 xd:1;                  // Execute disable
} __attribute__((__packed__));

_Static_assert(sizeof(struct pml4e_fields) == PML4E_FIELDS_SIZE,
    L"pml4e_fields is not " AC_XSTR(PML4E_FIELDS_SIZE) " bytes");

union pml4e_fields_u {
  ac_u64 raw;
  struct pml4e_fields fields;
};

_Static_assert(sizeof(union pml4e_fields_u) == PML4E_FIELDS_SIZE,
    L"pml4e_fields_u is not " AC_XSTR(PML4E_FIELDS_SIZE) " bytes");

/**
 * PDPTE for 1Gbyte pages
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 4.5 "IA-32e Paging"
 * Table 4-15. "Format of an IA-32 Page-Direcotry-Pointer-Table
 * (PDPTE) that Maps a 1-GByte page"
 */
struct pdpte_1g_fields {
  ac_u64 p:1;                   // Present
  ac_u64 rw:1;                  // Read write
  ac_u64 us:1;                  // User/Supervisor
  ac_u64 pwt:1;                 // Page-level write-through
  ac_u64 pcd:1;                 // Page-level cache disable
  ac_u64 a:1;                   // Accessed
  ac_u64 d:1;                   // Dirty
  ac_u64 ps:1;                  // Page size, 1 for 1GBE, 0 for other sizes
  ac_u64 g:1;                   // Global; if CR4.PGE = 1
  ac_u64 reserved_0:3;
  ac_u64 pat:1;                 // Memory type (see 11.12.3)
  ac_u64 reserved_1:17;
  ac_u64 phy_addr:29;           // Physical address of 1-GByte page
  ac_u64 pke:4;                 // Protection key if CR4.PKE == 1
  ac_u64 xd:1;                  // Execute disable if IA32_EFER.NXE = 1
} __attribute__((__packed__));

_Static_assert(sizeof(struct pdpte_1g_fields) == PDPTE_FIELDS_SIZE,
    L"pdpte_1g_fields is not " AC_XSTR(PDPTE_FIELDS_SIZE) " bytes");

/**
 * PDPTE that references Page Directory
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 4.5 "IA-32e Paging"
 * Table 4-16. "Format of an IA-32 Page-Direcotry-Pointer-Table
 * (PDPTE) that References a Page Directory"
 */
struct pdpte_fields {
  ac_u64 p:1;                   // Present
  ac_u64 rw:1;                  // Read write
  ac_u64 us:1;                  // User/Supervisor
  ac_u64 pwt:1;                 // Page-level write-through
  ac_u64 pcd:1;                 // Page-level cache disable
  ac_u64 a:1;                   // Accessed
  ac_u64 reserved_0:6;
  ac_u64 phy_addr:51;           // Physical address of Page Directory
  ac_u64 xd:1;                  // Execute disable if IA32_EFER.NXE = 1
} __attribute__((__packed__));

_Static_assert(sizeof(struct pdpte_fields) == PDPTE_FIELDS_SIZE,
    L"pdpte_fields is not " AC_XSTR(PDPTE_FIELDS_SIZE) " bytes");

union pdpte_fields_u {
  ac_uint raw;
  struct pdpte_1g_fields g_fields;
  struct pdpte_fields fields;
};

_Static_assert(sizeof(union pdpte_fields_u) == PDPTE_FIELDS_SIZE,
    L"pdpte_fields_u is not " AC_XSTR(PDPTE_FIELDS_SIZE) " bytes");

/**
 * PDE that references Page Directory Entry that maps a 2-MByte page
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 4.5 "IA-32e Paging"
 * Table 4-17. "Format of an IA-32 Page-Directory Entry (PDE)
 * that References a 2-MByte Page"
 */
struct pde_2m_fields {
  ac_u64 p:1;                   // Present
  ac_u64 rw:1;                  // Read write
  ac_u64 us:1;                  // User/Supervisor
  ac_u64 pwt:1;                 // Page-level write-through
  ac_u64 pcd:1;                 // Page-level cache disable
  ac_u64 a:1;                   // Accessed
  ac_u64 d:1;                   // Dirty
  ac_u64 ps:1;                  // Page size, must be 1 for 2M pages
  ac_u64 g:1;                   // Global; if CR4.PGE = 1
  ac_u64 reserved_0:3;
  ac_u64 pat:1;                 // Memory type (see 11.12.3)
  ac_u64 reserved_1:8;
  ac_u64 phy_addr:38;           // Physical address of 2-MByte page
  ac_u64 pke:4;                 // Protection key if CR4.PKE == 1
  ac_u64 xd:1;                  // Execute disable if IA32_EFER.NXE = 1
} __attribute__((__packed__));

_Static_assert(sizeof(struct pdpte_fields) == PDPTE_FIELDS_SIZE,
    L"pdpte_fields is not " AC_XSTR(PDPTE_FIELDS_SIZE) " bytes");

/**
 * PDE that references Page Directory Entry that maps a 2-MByte page
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 4.5 "IA-32e Paging"
 * Table 4-18. "Format of an IA-32 Page-Directory Entry (PDE)
 * that References a Page Table"
 */
struct pde_fields {
  ac_u64 p:1;                   // Present
  ac_u64 rw:1;                  // Read write
  ac_u64 us:1;                  // User/Supervisor
  ac_u64 pwt:1;                 // Page-level write-through
  ac_u64 pcd:1;                 // Page-level cache disable
  ac_u64 a:1;                   // Accessed
  ac_u64 reserved_0:6;
  ac_u64 phy_addr:51;           // Physical address of Page Directory
  ac_u64 xd:1;                  // Execute disable if IA32_EFER.NXE = 1
} __attribute__((__packed__));

_Static_assert(sizeof(struct pdpte_fields) == PDE_FIELDS_SIZE,
    L"pdpte_fields is not " AC_XSTR(PDE_FIELDS_SIZE) " bytes");

union pde_fields_u {
  ac_uint raw;
  struct pde_2m_fields m_fields;
  struct pde_fields fields;
};

_Static_assert(sizeof(union pde_fields_u) == PDE_FIELDS_SIZE,
    L"pde_fields_u is not " AC_XSTR(PDE_FIELDS_SIZE) " bytes");

/**
 * PTE that references Page Table Entry that maps a 4-KByte page
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 4.5 "IA-32e Paging"
 * Table 4-19. "Format of an IA-32 Page-Table Entry (PTE)
 * that References a 4-KByte Page"
 */
struct pte_fields {
  ac_u64 p:1;                   // Present
  ac_u64 rw:1;                  // Read write
  ac_u64 us:1;                  // User/Supervisor
  ac_u64 pwt:1;                 // Page-level write-through
  ac_u64 pcd:1;                 // Page-level cache disable
  ac_u64 a:1;                   // Accessed
  ac_u64 d:1;                   // Dirty
  ac_u64 pat:1;                 // Memory type (see 11.12.3)
  ac_u64 g:1;                   // Global; if CR4.PGE = 1
  ac_u64 reserved_0:3;
  ac_u64 phy_addr:47;           // Physical address of 4-KByte page
  ac_u64 pke:4;                 // Protection key if CR4.PKE == 1
  ac_u64 xd:1;                  // Execute disable if IA32_EFER.NXE = 1
} __attribute__((__packed__));

_Static_assert(sizeof(struct pte_fields) == PTE_FIELDS_SIZE,
    L"pte_fields is not " AC_XSTR(PTE_FIELDS_SIZE) " bytes");


union pte_fields_u {
  ac_uint raw;
  struct pte_fields fields;
};


/**
 * Return the current cpu page_mode
 */
enum page_mode get_page_mode(void);

/**
 * Return the current cpu page table
 */
static inline union cr3_paging_fields_u get_page_table(void) {
  union cr3_paging_fields_u val = { .raw=get_cr3() };
  return val;
}

/**
 * Return the physical address of the current cpu page table
 */
static inline void* get_page_directory_addr(union cr3_paging_fields_u cr3u, enum page_mode mode) {
  ac_uptr addr;
  switch (mode) {
    case PAGE_MODE_NRML_32BIT:{
      addr = (ac_uptr)cr3u.nrml_paging_fields.page_directory_base << 12;
      break;
    }
    case PAGE_MODE_PAE_32BIT:{
      addr = (ac_uptr)cr3u.pae_paging_fields.page_directory_base << 5;
      break;
    }
    case PAGE_MODE_NRML_64BIT:{
      addr = (ac_uptr)cr3u.nrml_paging_fields.page_directory_base << 12;
      break;
    }
    case PAGE_MODE_PCIDE_64BIT:{
      addr = (ac_uptr)cr3u.pcide_paging_fields.page_directory_base << 12;
      break;
    }
    case PAGE_MODE_UNKNOWN:
    default: {
      addr = 0;
      break;
    }
  };
  return (void*)addr;
}

#endif
