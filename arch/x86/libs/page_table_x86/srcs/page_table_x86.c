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

#include <page_table_x86.h>

#include <multiboot2.h>

#include <cr_x86.h>
#include <msr_x86.h>
#include <reset_x86.h>

#include <ac_inttypes.h>
#include <ac_memset.h>
#include <ac_printf.h>

#undef NDEBUG
#include <ac_debug_printf.h>

/**
 * Return the current cpu page_mode
 */
enum page_mode get_page_mode(void) {
  enum page_mode pm;

  union cr0_u cr0u = { .raw = get_cr0() };
  union cr4_u cr4u = { .raw = get_cr4() };
  union msr_efer_u eferu = { .raw = get_msr(MSR_EFER) };

  if (cr0u.fields.pg == 1) {
    if ((cr4u.fields.pae == 0) && (eferu.fields.lme == 0)) {
      pm = PAGE_MODE_NRML_32BIT;
    } else if ((cr4u.fields.pae == 1) && (eferu.fields.lme == 0)) {
      pm = PAGE_MODE_PAE_32BIT;
    } else if ((cr4u.fields.pae == 1) && (eferu.fields.lme == 1)) {
      if (cr4u.fields.pcide == 0) {
        pm = PAGE_MODE_NRML_64BIT;
      } else {
        pm = PAGE_MODE_PCIDE_64BIT;
      }
    } else {
      pm = PAGE_MODE_UNKNOWN;
    }
  } else {
    pm = PAGE_MODE_UNKNOWN;
  }

  return pm;
}

/**
 * Stack of 4k pages for use as pde/pte's that are
 * located in an area where physical_addr == linear_address.
 * I call these phylin.
 */
ac_uint top;
void* phylin_4k_stack[256];
ac_u8 phylin_4k_pages[256][4096] __attribute__((__aligned__(4096)));

_Static_assert(sizeof(phylin_4k_pages) == (256*4096), "Wrong size");

/**
 * Initalize phylin 4k pages
 */
void init_phylin_4k_pages(void) {
  for(ac_uint i = 0; i < AC_ARRAY_COUNT(phylin_4k_stack); i++) {
    phylin_4k_stack[i] = (void*)&phylin_4k_pages[i];
  }
  top = 0;
  ac_debug_printf("init_phy_4k_pages:+- top=%d\n", top);
}

/**
 * return a phylin address of a cleared 4k page.
 */
void* calloc_phylin_4k_page() {
  ac_debug_printf("calloc_phylin_4k_page:+ top=%d\n", top);
  if (top >= AC_ARRAY_COUNT(phylin_4k_stack)) {
    return 0;
  }
  ac_u8 *p = phylin_4k_stack[top++];
  ac_memset(p, 0, FOUR_K_PAGE_SIZE);
  ac_debug_printf("calloc_phylin_4k_page:- p=0x%p top=%d\n", p, top);
  return p;
}

void free_phylin_4k_page(void* page) {
  ac_debug_printf("free_phylin_4k_page:+ page=0x%p top=%d\n", page, top);
  if ((top > 0) && (top >= AC_ARRAY_COUNT(phylin_4k_stack))) {
    phylin_4k_stack[--top] = page;
    ac_debug_printf("free_phylin_4k_page:+ page=0x%p top=%d\n", page, top);
  } else {
    ac_debug_printf("free_phylin_4k_page:- ignore page=0x%p stack is full\n",
        page);
  }
}

/**
 * Get a pde or allocate it if it doesn't exist
 */
struct pde_fields* alloc_pde() {
  struct pde_fields* entry = calloc_phylin_4k_page();

  if (entry == AC_NULL) {
    ac_printf("No phy_4k_pages\n");
    reset_x86();
  }

  ac_debug_printf("alloc_pde:+- entry=0x%p\n", entry);
  return entry;
}

/**
 * Get a pde or allocate it if it doesn't exist
 */
struct pde_fields* get_or_alloc_pde(struct pde_fields *base, ac_uint idx) {
  union pde_fields_u* entry = (union pde_fields_u*)&base[idx];
  ac_debug_printf("get_or_alloc_pde:+ base=0x%p idx=0x%x entry=%p raw=0x%lx entry.p=%d\n",
      base, idx, entry, entry->raw, entry->fields.p);
  if (!entry->fields.p) {
    entry->fields.phy_addr = (ac_u64)alloc_pde() >> 12;
    entry->fields.p = 1;
    entry->fields.rw = 1;
  } else {
    ac_debug_printf("get_or_alloc_pde: present entry=%p entry.p=%d\n",
      entry, entry->fields.p);
  }

  ac_debug_printf("get_or_alloc_pde:- entry=0x%p\n", entry);
  return (struct pde_fields*)entry;
}

/**
 * get the pat, pcd and pwt associated with the caching mode.
 */
void get_caching_bits(enum page_caching caching, ac_bool* pat, ac_bool* pcd,
    ac_bool* pwt) {

  struct caching_bits {
    ac_bool pat;
    ac_bool pcd;
    ac_bool pwt;
  } caching_to_bits_array[8] = {
    { 0, 0, 0 },
    { 0, 0, 1 },
    { 0, 1, 0 },
    { 0, 1, 1 },
    { 1, 0, 0 },
    { 1, 0, 1 },
    { 1, 1, 0 },
    { 1, 1, 1 },
  };

  if (caching == PAGE_CACHING_UNKNOWN) {
    ac_printf("ABORTING: caching=0x%x (PAGE_CACHING_UNKNOWN)\n", caching);
    reset_x86();
  }

  if (caching > 8) {
    ac_printf("ABORTING: caching=0x%x is to large\n", caching);
    reset_x86();
  }

  ac_uint idx = ((ac_uint)caching - 1) & 0x7;
  struct caching_bits* entry = &caching_to_bits_array[idx];
  *pat = entry->pat;
  *pcd = entry->pcd;
  *pwt = entry->pwt;
  ac_debug_printf("get_caching_bits:+- caching=%x idx=0x%x pat=%d pcd=%d pwt=%d\n",
      caching, idx, *pat, *pcd, *pwt);
}

/**
 * Create a page table entries for the physical/linear address
 *
 * param: page_table_base is a phylin_addr of the base of the page table
 *        if its AC_NULL it will be allocated.
 * param: phy_addr is the physical address to map to lin_addr
 * param: lin_addr is the linear address the phy_addr is mapped to
 * param: size is the number of bytes in the phy_addr must be multiple of 4K
 * param: caching is the caching strategy to use.
 *
 * returns AC_NULL on failure else page_table_base
 */
struct pde_fields* page_table_map_physical_to_linear(
    struct pde_fields* page_table_base, ac_uptr phy_addr, void* lin_addr,
    ac_uptr size, enum page_caching caching) {

  ac_bool pat, pcd, pwt;
  ac_debug_printf("page_table_map_physical_to_linear:+ page_table_base=0x%p\n",
      page_table_base);
  ac_debug_printf("  phy_addr=0x%p lin_addr=0x%p\n", phy_addr, lin_addr);
  ac_debug_printf("  size=0x%p caching=0x%x\n", size, caching);

  if (page_table_base == AC_NULL) {
    page_table_base = alloc_pde();

    // Add recursive entry
    page_table_base[511].p = 1;
    page_table_base[511].rw = 1;
    page_table_base[511].phy_addr = linear_to_physical_addr(&page_table_base[0]) >> 12;

    ac_debug_printf("page_table_map_physical_to_linear: allocate page_table base=0x%p\n", page_table_base);
  }

#ifdef CPU_X86_64
  union linear_address_pml_indexes_u laddr = { .raw = (ac_uptr)lin_addr };

  while (size >= ONE_GIG_PAGE_SIZE) {
    ac_printf("ABORTING: page_table_map_physical_to_linear One gig pages not yet supported\n");
    reset_x86();
  }

  while (size >= TWO_MEG_PAGE_SIZE) {
    ac_printf("ABORTING: page_table_map_physical_to_linear Two meg pages not yet supported\n");
    reset_x86();
  }

  struct pde_fields* pml4 = page_table_base;
  while (size > 0) {
    ac_debug_printf("\npage_table_map_physical_to_linear: 4K loop phy_addr=0x%p lin_addr=0x%p size=%x\n",
        phy_addr, laddr.raw, size);

    ac_debug_printf("page_table_map_physical_to_linear: pml4=0x%p idx=%x\n", pml4, laddr.indexes.pml4);
    struct pde_fields* pml4_entry = get_or_alloc_pde(pml4, laddr.indexes.pml4);
    ac_debug_printf("page_table_map_physical_to_linear: pml4_entry=0x%p\n", pml4_entry);

    struct pde_fields* pml3 =
      physical_to_linear_addr(pml4_entry->phy_addr << 12);
    ac_debug_printf("page_table_map_physical_to_linear: pml3=0x%p idx=%x\n", pml3, laddr.indexes.pml3);
    struct pde_fields* pml3_entry = get_or_alloc_pde(pml3, laddr.indexes.pml4);
    ac_debug_printf("page_table_map_physical_to_linear: pml3_entry=0x%p\n", pml3_entry);

    struct pde_fields* pml2 =
      physical_to_linear_addr(pml3_entry->phy_addr << 12);
    ac_debug_printf("page_table_map_physical_to_linear: pml2=0x%p idx=%x\n", pml2, laddr.indexes.pml2);
    struct pde_fields* pml2_entry = get_or_alloc_pde(pml2, laddr.indexes.pml4);
    ac_debug_printf("page_table_map_physical_to_linear: pml2_entry=0x%p\n", pml2_entry);

    union pte_fields_u* pml1 =
      physical_to_linear_addr(pml2_entry->phy_addr << 12);
    ac_debug_printf("page_table_map_physical_to_linear: pml1=0x%p idx=%x\n", pml1, laddr.indexes.pml1);
    union pte_fields_u* pml1_entry = &pml1[laddr.indexes.pml1];
    ac_debug_printf("page_table_map_physical_to_linear: pml1_entry=0x%p\n", pml1_entry);

    if (pml1_entry->small.p) {
      ac_printf("ABORTING: page_table_map_physical_to_linear 4K page at 0x%lx is already present\n");
      reset_x86();
    }

    // Get the caching bits and initialize the entry
    get_caching_bits(caching, &pat, &pcd, &pwt);
    pml1_entry->small.phy_addr = linear_to_physical_addr((void*)laddr.raw) >> 12;
    pml1_entry->small.p = 1;
    pml1_entry->small.rw = 1;
    pml1_entry->small.pat = pat;
    pml1_entry->small.pcd = pcd;
    pml1_entry->small.pwt = pwt;

    laddr.raw += FOUR_K_PAGE_SIZE;
    phy_addr += FOUR_K_PAGE_SIZE;
    size -= FOUR_K_PAGE_SIZE;
  }

#else /* CPU_X86_32 */
  ac_printf("ABORTING: page_table_map_physical_to_linear CPU_X86_32 not supported\n");
  reset_x86();
  page_table_base = ac_null;
#endif
  ac_debug_printf("page_table_map_physical_to_linear:0 page_table_base=0x%lx\n",
      page_table_base);
  return page_table_base;
}

struct pde_fields pml4[512] __attribute__((__aligned__(0x2000)));
struct pde_fields pml3[512] __attribute__((__aligned__(0x2000)));
struct pte_huge_fields pte2m[512] __attribute__((__aligned__(0x2000)));

/**
 * Initialize page tables from multiboot2 memory map information.
 */
void init_page_tables(struct multiboot2_memory_map_tag* mm, ac_uint count) {
  AC_UNUSED(mm);
  AC_UNUSED(count);

  ac_printf("init_page_tables+:\n");

  init_phylin_4k_pages();

  ac_memset(pml4, 0, sizeof(pml4));
  pml4[0].p = 1;
  pml4[0].rw = 1;
  pml4[0].phy_addr = linear_to_physical_addr(&pml3[0]) >> 12;

  // Recursive entry
  pml4[511].p = 1;
  pml4[511].rw = 1;
  pml4[511].phy_addr = linear_to_physical_addr(&pml4[0]) >> 12;

  ac_memset(pml3, 0, sizeof(pml3));
  pml3[0].p = 1;
  pml3[0].rw = 1;
  pml3[0].phy_addr = linear_to_physical_addr(&pte2m[0]) >> 12;

  ac_memset(pte2m, 0, sizeof(pte2m));
  ac_u8* phy_addr = 0;
  for (ac_uint i = 0; i < AC_ARRAY_COUNT(pte2m); i++) {
    pte2m[i].p = 1;
    pte2m[i].rw = 1;
    pte2m[i].ps = 1;
    pte2m[i].phy_addr = linear_to_physical_addr(phy_addr) >> 13;
    phy_addr += 1024 * 1024 * 2;
  }
  //print_page_table_linear(pml4, PAGE_MODE_NRML_64BIT);

  ac_printf("CR3 is now 0x%x\n", get_cr3());
  set_cr3((ac_uint)pml4);
  ac_printf("init_page_tables:- Changed to our page tables, CR3=0x%x\n",
      get_cr3());
}
