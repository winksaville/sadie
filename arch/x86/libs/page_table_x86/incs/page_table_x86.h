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

#include <multiboot2.h>

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

/**
 * Page chaching methods
 *
 * Caching styles available on X86
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 11 "Methods of Caching Available"
 * Table 11-2. "Memory Types and Their Properites"
 */
enum page_caching {
  PAGE_CACHING_UNKNOWN = 0,
  PAGE_CACHING_WRITE_BACK = 1,
  PAGE_CACHING_WRITE_THROUGH = 2,
  PAGE_CACHING_UNCACHEABLE = 3,
  PAGE_CACHING_STRONG_UNCACHEABLE = 4,
};

/**
 * PAGE SIZES
 */
#define ONE_GIG_PAGE_SIZE (1 * (ac_uptr)1024 * (ac_uptr)1024 * (ac_uptr)1024)
#define TWO_MEG_PAGE_SIZE (2 * (ac_uptr)1024 * (ac_uptr)1024)
#define FOUR_K_PAGE_SIZE  (4 * (ac_uptr)1024)

/**
 * Size of the Page Directory Entries
 */
#define PDE_FIELDS_SIZE 8

/**
 * Size of the Page Table Entries
 */
#define PTE_FIELDS_SIZE 8

/**
 * Size of linear addresses
 */
#define LINEAR_ADDRESS_SIZE 8

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
  ac_bool pwt:1;
  ac_bool pcd:1;
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
 * PDE that references Page Directory Entry that maps a 2-MByte page
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 4.5 "IA-32e Paging"
 * Table 4-18. "Format of an IA-32 Page-Directory Entry (PDE)
 * that References a Page Table"
 */
struct pde_fields {
  ac_bool p:1;                  // Present
  ac_bool rw:1;                 // Read write
  ac_bool us:1;                 // User/Supervisor
  ac_bool pwt:1;                // Page-level write-through
  ac_bool pcd:1;                // Page-level cache disable
  ac_bool a:1;                  // Accessed
  ac_bool reserved_0:1;
  ac_bool ps_pte:1;             // 0 == pde, 1 = pte
  ac_u64 reserved_1:4;
  ac_u64 phy_addr:51;           // Physical address of Page Directory
  ac_bool xd:1;                 // Execute disable if IA32_EFER.NXE = 1
} __attribute__((__packed__));

_Static_assert(sizeof(struct pde_fields) == PDE_FIELDS_SIZE,
    L"pde_fields is not " AC_XSTR(PDE_FIELDS_SIZE) " bytes");

union pde_fields_u {
  ac_u64 raw;
  struct pde_fields fields;
};

_Static_assert(sizeof(union pde_fields_u) == PDE_FIELDS_SIZE,
    L"pde_fields_u is not " AC_XSTR(PDE_FIELDS_SIZE) " bytes");

/**
 * Page Table Entry that maps for huge pages depending on the PS field.
 * ps == 1 for 1GByte pages and ps == 0 for 2MByte pages.
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 4.5 "IA-32e Paging"
 * Table 4-15. "Format of an IA-32 Page-Direcotry-Pointer-Table
 * (PDPTE) that Maps a 1-GByte page"
 * Table 4-17. "Format of an IA-32 Page-Direcotry-Pointer-Table
 * (PDPTE) that Maps a 2-MByte page"
 */
struct pte_huge_fields {
  ac_bool p:1;                  // Present
  ac_bool rw:1;                 // Read write
  ac_bool us:1;                 // User/Supervisor
  ac_bool pwt:1;                // Page-level write-through
  ac_bool pcd:1;                // Page-level cache disable
  ac_bool a:1;                  // Accessed
  ac_bool d:1;                  // Dirty
  ac_bool ps_pte:1;             // Page size/PTE, must be 1 to signify pte_huge
  ac_bool g:1;                  // Global; if CR4.PGE = 1
  ac_u64 reserved_0:3;
  ac_bool pat:1;                // Memory type (see 11.12.3)
  ac_u64 phy_addr:46;           // Physical address of huge page
  ac_u64 pke:4;                 // Protection key if CR4.PKE == 1
  ac_bool xd:1;                 // Execute disable if IA32_EFER.NXE = 1
} __attribute__((__packed__));

_Static_assert(sizeof(struct pte_huge_fields) == PTE_FIELDS_SIZE,
    L"pdpte_huge_fields is not " AC_XSTR(PTE_FIELDS_SIZE) " bytes");

/**
 * Page Table Entry that maps a 4-KByte page
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 4.5 "IA-32e Paging"
 * Table 4-19. "Format of an IA-32 Page-Table Entry (PTE)
 * that References a 4-KByte Page"
 */
struct pte_small_fields {
  ac_bool p:1;                  // Present
  ac_bool rw:1;                 // Read write
  ac_bool us:1;                 // User/Supervisor
  ac_bool pwt:1;                // Page-level write-through
  ac_bool pcd:1;                // Page-level cache disable
  ac_bool a:1;                  // Accessed
  ac_bool d:1;                  // Dirty
  ac_bool pat:1;                // Memory type (see 11.12.3)
  ac_bool g:1;                  // Global; if CR4.PGE = 1
  ac_u64 reserved_0:3;
  ac_u64 phy_addr:47;           // Physical address of 4-KByte page
  ac_u64 pke:4;                 // Protection key if CR4.PKE == 1
  ac_bool xd:1;                 // Execute disable if IA32_EFER.NXE = 1
} __attribute__((__packed__));

_Static_assert(sizeof(struct pte_small_fields) == PTE_FIELDS_SIZE,
    L"pte_small_fields is not " AC_XSTR(PTE_FIELDS_SIZE) " bytes");


union pte_fields_u {
  ac_u64 raw;
  struct pte_small_fields small;
  struct pte_huge_fields huge;
};

_Static_assert(sizeof(union pte_fields_u) == PTE_FIELDS_SIZE,
    L"pte_fields_u is not " AC_XSTR(PTE_FIELDS_SIZE) " bytes");

/**
 * Decode linear address
 */
struct linear_address_pml_indexes {
  ac_u64 offset:12;
  ac_u64 pml1:9;
  ac_u64 pml2:9;
  ac_u64 pml3:9;
  ac_u64 pml4:9;
};

_Static_assert(sizeof(struct linear_address_pml_indexes) == LINEAR_ADDRESS_SIZE,
    L"linear_address_pml_indexes is not " AC_XSTR(LINEAR_ADDRESS_SIZE) " bytes");

union linear_address_pml_indexes_u {
 ac_u64 raw;
 struct linear_address_pml_indexes indexes;
};


_Static_assert(sizeof(union linear_address_pml_indexes_u) == LINEAR_ADDRESS_SIZE,
    L"linear_address_pml_indexes_u is not " AC_XSTR(LINEAR_ADDRESS_SIZE) " bytes");

/**
 * Return the current cpu page_mode
 */
enum page_mode get_page_mode(void);

/**
 * For the moment assume linear == physical
 */
static inline void* physical_to_linear_addr(ac_uptr addr) {
  return (void*)addr;
}

/**
 * For the moment assume linear == physical
 */
static inline ac_uptr linear_to_physical_addr(void* addr) {
  return (ac_uptr)addr;
}

/**
 * Return the physical field of the current cpu page table
 */
static inline ac_u64 get_pde_phy_addr_field(
    union cr3_paging_fields_u cr3u, enum page_mode mode) {
  ac_u64 addr;

  switch (mode) {
    case PAGE_MODE_NRML_32BIT:{
      addr = (ac_uptr)cr3u.nrml_paging_fields.page_directory_base;
      break;
    }
    case PAGE_MODE_PAE_32BIT:{
      addr = (ac_uptr)cr3u.pae_paging_fields.page_directory_base;
      break;
    }
    case PAGE_MODE_NRML_64BIT:{
      addr = (ac_uptr)cr3u.nrml_paging_fields.page_directory_base;
      break;
    }
    case PAGE_MODE_PCIDE_64BIT:{
      addr = (ac_uptr)cr3u.pcide_paging_fields.page_directory_base;
      break;
    }
    case PAGE_MODE_UNKNOWN:
    default: {
      addr = 0;
      break;
    }
  };

  return addr;
}

/**
 * Return the linear address of the current cpu page table
 */
static inline void* get_cr3_pde_linear_addr(
    union cr3_paging_fields_u cr3u, enum page_mode mode) {

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

  return physical_to_linear_addr(addr);
}

/**
 * Return the current cpu page table
 */
static inline union cr3_paging_fields_u get_page_table(void) {
  union cr3_paging_fields_u val = { .raw=get_cr3() };
  return val;
}

/**
 * Return the current cpu page table as a linear address
 */
static inline void* get_page_table_linear_addr(void) {
  return get_cr3_pde_linear_addr(get_page_table(), get_page_mode());
}

/**
 * Create page table entries for the linear to physical address range
 *
 * param: page_table_base is a phylin_addr of the base of the page table
 *        if its AC_NULL it will be allocated.
 * param: lin_addr is the linear address the phy_addr is mapped to
 * param: phy_addr is the physical address to map to lin_addr
 * param: size is the number of bytes in the phy_addr must be multiple of 4K
 * param: caching is the caching strategy to use.
 *
 * returns AC_NULL on failure else page_table_base
 */
struct pde_fields* page_table_map_lin_to_phy(
    struct pde_fields* page_table_base, void* linear_addr, ac_u64 phyiscal_addr,
    ac_u64 size, enum page_caching caching);

/**
 * Initialize page tables from multiboot2 memory map information.
 */
void init_page_tables(struct multiboot2_memory_map_tag* mm, ac_uint count);

/**
 * Initialize early page table which are linear == physical
 *
 * @param phylinbase is starting address of linear/physical address
 * @param lingth is number of bytes to map
 */
void init_early_page_tables(ac_u64 phylin_base, ac_u64 length);

#endif
