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
#include <page_table_x86_print.h>
#include <cr_x86.h>
#include <cr_x86_print.h>

#include <interrupts_x86.h>

#include <ac_architecture.h>
#include <ac_inttypes.h>
#include <ac_printf.h>
#include <ac_test.h>

/**
 * Test case array filled with the val followed
 * by the expected field values. The array initialization
 * fields that are NOT explicitly initialized are zeros.
 *
 * This is a shortened walking 1 bit test.
 */
static struct test_case_cr3_nrml_paging_fields {
  union cr3_paging_fields_u val;
  ac_uint reserved_0;
  ac_uint pwt;
  ac_uint pcd;
  ac_uint reserved_1;
  ac_uint page_directory_base;
} test_case_cr3_nrml_paging_fields_array[] = {
  { .val.raw=0x1, .reserved_0=0x1, },
  { .val.raw=0x4, .reserved_0=0x4, },
  { .val.raw=0x8, .pwt=0x1, },
  { .val.raw=0x10, .pcd=0x1, },
  { .val.raw=0x20, .reserved_1=0x1, },
  { .val.raw=0x800, .reserved_1=0x40, },
#ifdef CPU_X86_64
  { .val.raw=0x1000, .page_directory_base=0x1, },
  { .val.raw=0x8000000000000000, .page_directory_base=0x8000000000000, },
  { .val.raw=0xC000000000000000, .page_directory_base=0xC000000000000, },
#else
  { .val.raw=0x1000, .page_directory_base=0x1, },
  { .val.raw=0x80000000, .page_directory_base=0x80000, },
#endif
};

static ac_bool test_cr3_nrml_paging_fields(
    struct test_case_cr3_nrml_paging_fields* test) {
  ac_bool error = AC_FALSE;

  print_cr3_nrml_paging_fields("", test->val.raw);

  error |= AC_TEST(test->val.nrml_paging_fields.reserved_0 == test->reserved_0);
  error |= AC_TEST(test->val.nrml_paging_fields.pwt == test->pwt);
  error |= AC_TEST(test->val.nrml_paging_fields.pcd == test->pcd);
  error |= AC_TEST(test->val.nrml_paging_fields.reserved_1 == test->reserved_1);
  error |= AC_TEST(test->val.nrml_paging_fields.page_directory_base
      == test->page_directory_base);

  return error;
}

static ac_bool test_cr3_nrml_paging_fields_array(void) {
  ac_bool error = AC_FALSE;

  for (ac_uint i = 0;
      i < AC_ARRAY_COUNT(test_case_cr3_nrml_paging_fields_array); i++) {
    error |= test_cr3_nrml_paging_fields(
        &test_case_cr3_nrml_paging_fields_array[i]);
  }

  return error;
}


/**
 * Test case array filled with the val followed
 * by the expected field values. The array initialization
 * fields that are NOT explicitly initialized are zeros.
 *
 * This is a shortened walking 1 bit test.
 */
static struct test_case_cr3_pae_paging_fields {
  union cr3_paging_fields_u val;
  ac_uint reserved_0;
  ac_uint page_directory_base;
} test_case_cr3_pae_paging_fields_array[] = {
  { .val.raw=0x1, .reserved_0=0x1, },
  { .val.raw=0x10, .reserved_0=0x10, },
#ifdef CPU_X86_64
  { .val.raw=0x20, .page_directory_base=0x1, },
  { .val.raw=0x8000000000000000, .page_directory_base=0x400000000000000, },
#else
  { .val.raw=0x20, .page_directory_base=0x1, },
  { .val.raw=0x80000000, .page_directory_base=0x4000000, },
#endif
};

static ac_bool test_cr3_pae_paging_fields(
    struct test_case_cr3_pae_paging_fields* test) {
  ac_bool error = AC_FALSE;

  print_cr3_pae_paging_fields("", test->val.raw);

  error |= AC_TEST(test->val.pae_paging_fields.reserved_0 == test->reserved_0);
  error |= AC_TEST(test->val.pae_paging_fields.page_directory_base
      == test->page_directory_base);

  return error;
}

static ac_bool test_cr3_pae_paging_fields_array(void) {
  ac_bool error = AC_FALSE;

  for (ac_uint i = 0;
      i < AC_ARRAY_COUNT(test_case_cr3_pae_paging_fields_array); i++) {
    error |= test_cr3_pae_paging_fields(
        &test_case_cr3_pae_paging_fields_array[i]);
  }

  return error;
}

/**
 * Test case array filled with the val followed
 * by the expected field values. The array initialization
 * fields that are NOT explicitly initialized are zeros.
 *
 * This is a shortened walking 1 bit test.
 */
static struct test_case_cr3_pcide_paging_fields {
  union cr3_paging_fields_u val;
  ac_uint pcid;
  ac_uint page_directory_base;
} test_case_cr3_pcide_paging_fields_array[] = {
  { .val.raw=0x1, .pcid=0x1, },
  { .val.raw=0x800, .pcid=0x800, },
#ifdef CPU_X86_64
  { .val.raw=0x1000, .page_directory_base=0x1, },
  { .val.raw=0x8000000000000000, .page_directory_base=0x8000000000000, },
  { .val.raw=0xC000000000000000, .page_directory_base=0xC000000000000, },
#else
  { .val.raw=0x1000, .page_directory_base=0x1, },
  { .val.raw=0x80000000, .page_directory_base=0x80000, },
#endif
};

static ac_bool test_cr3_pcide_paging_fields(
    struct test_case_cr3_pcide_paging_fields* test) {
  ac_bool error = AC_FALSE;

  print_cr3_pcide_paging_fields("", test->val.raw);

  error |= AC_TEST(test->val.pcide_paging_fields.pcid == test->pcid);
  error |= AC_TEST(test->val.pcide_paging_fields.page_directory_base
      == test->page_directory_base);

  return error;
}

static ac_bool test_cr3_pcide_paging_fields_array(void) {
  ac_bool error = AC_FALSE;

  for (ac_uint i = 0;
      i < AC_ARRAY_COUNT(test_case_cr3_pcide_paging_fields_array); i++) {
    error |= test_cr3_pcide_paging_fields(
        &test_case_cr3_pcide_paging_fields_array[i]);
  }

  return error;
}

/**
 * Test case array filled with the val followed
 * by the expected field values. The array initialization
 * fields that are NOT explicitly initialized are zeros.
 *
 * This is a shortened walking 1 bit test.
 */
static struct test_case_pde_fields {
  union pde_fields_u val;
  ac_u64 p;
  ac_u64 rw;
  ac_u64 us;
  ac_u64 pwt;
  ac_u64 pcd;
  ac_u64 a;
  ac_u64 reserved_0;
  ac_u64 ps_pte;
  ac_u64 reserved_1;
  ac_u64 phy_addr;
  ac_u64 xd;
} test_case_pde_fields_array[] = {
  { .val.raw=0x1, .p=0x1, },
  { .val.raw=0x2, .rw=0x1, },
  { .val.raw=0x4, .us=0x1, },
  { .val.raw=0x8, .pwt=0x1, },
  { .val.raw=0x10, .pcd=0x1, },
  { .val.raw=0x20, .a=0x1, },
  { .val.raw=0x40, .reserved_0=0x1, },
  { .val.raw=0x80, .ps_pte=0x1, },
  { .val.raw=0x100, .reserved_1=0x1, },
  { .val.raw=0x800, .reserved_1=0x8, },
  { .val.raw=0x1000, .phy_addr=0x1, },
  { .val.raw=0x4000000000000000, .phy_addr=0x4000000000000, },
  { .val.raw=0x8000000000000000, .xd=0x1, },
};

static ac_bool test_pde_fields(struct test_case_pde_fields* test) {
  ac_bool error = AC_FALSE;

  print_pde_fields("", test->val.raw);

  error |= AC_TEST(test->val.fields.p == test->p);
  error |= AC_TEST(test->val.fields.rw == test->rw);
  error |= AC_TEST(test->val.fields.us == test->us);
  error |= AC_TEST(test->val.fields.pwt == test->pwt);
  error |= AC_TEST(test->val.fields.pcd == test->pcd);
  error |= AC_TEST(test->val.fields.a == test->a);
  error |= AC_TEST(test->val.fields.reserved_0 == test->reserved_0);
  error |= AC_TEST(test->val.fields.ps_pte == test->ps_pte);
  error |= AC_TEST(test->val.fields.reserved_1 == test->reserved_1);
  error |= AC_TEST(test->val.fields.phy_addr == test->phy_addr);
  error |= AC_TEST(test->val.fields.xd == test->xd);

  return error;
}

static ac_bool test_pde_fields_array(void) {
  ac_bool error = AC_FALSE;

  for (ac_uint i = 0; i < AC_ARRAY_COUNT(test_case_pde_fields_array); i++) {
    error |= test_pde_fields(&test_case_pde_fields_array[i]);
  }

  return error;
}

/**
 * Test case array filled with the val followed
 * by the expected field values. The array initialization
 * fields that are NOT explicitly initialized are zeros.
 *
 * This is a shortened walking 1 bit test.
 */
static struct test_case_pte_small_fields {
  union pte_fields_u val;
  ac_u64 p;
  ac_u64 rw;
  ac_u64 us;
  ac_u64 pwt;
  ac_u64 pcd;
  ac_u64 a;
  ac_u64 d;
  ac_u64 pat;
  ac_u64 g;
  ac_u64 reserved_0;
  ac_u64 phy_addr;
  ac_u64 pke;
  ac_u64 xd;
} test_case_pte_fields_array[] = {
  { .val.raw=0x1, .p=0x1, },
  { .val.raw=0x2, .rw=0x1, },
  { .val.raw=0x4, .us=0x1, },
  { .val.raw=0x8, .pwt=0x1, },
  { .val.raw=0x10, .pcd=0x1, },
  { .val.raw=0x20, .a=0x1, },
  { .val.raw=0x40, .d=0x1, },
  { .val.raw=0x80, .pat=0x1, },
  { .val.raw=0x100, .g=0x1, },
  { .val.raw=0x200, .reserved_0=0x1, },
  { .val.raw=0x800, .reserved_0=0x4, },
  { .val.raw=0x1000, .phy_addr=0x1, },
  { .val.raw=0x0400000000000000, .phy_addr=0x400000000000, },
  { .val.raw=0x0800000000000000, .pke=0x1, },
  { .val.raw=0x4000000000000000, .pke=0x8, },
  { .val.raw=0x8000000000000000, .xd=0x1, },
};

static ac_bool test_pte_small_fields(
    struct test_case_pte_small_fields* test) {
  ac_bool error = AC_FALSE;

  print_pte_small_fields("", test->val.raw);

  error |= AC_TEST(test->val.small.p == test->p);
  error |= AC_TEST(test->val.small.rw == test->rw);
  error |= AC_TEST(test->val.small.us == test->us);
  error |= AC_TEST(test->val.small.pwt == test->pwt);
  error |= AC_TEST(test->val.small.pcd == test->pcd);
  error |= AC_TEST(test->val.small.a == test->a);
  error |= AC_TEST(test->val.small.d == test->d);
  error |= AC_TEST(test->val.small.pat == test->pat);
  error |= AC_TEST(test->val.small.g == test->g);
  error |= AC_TEST(test->val.small.reserved_0 == test->reserved_0);
  error |= AC_TEST(test->val.small.phy_addr == test->phy_addr);
  error |= AC_TEST(test->val.small.xd == test->xd);

  return error;
}

static ac_bool test_pte_small_fields_array(void) {
  ac_bool error = AC_FALSE;

  for (ac_uint i = 0;
      i < AC_ARRAY_COUNT(test_case_pte_fields_array); i++) {
    error |= test_pte_small_fields(&test_case_pte_fields_array[i]);
  }

  return error;
}

/**
 * Test case array filled with the val followed
 * by the expected field values. The array initialization
 * fields that are NOT explicitly initialized are zeros.
 *
 * This is a shortened walking 1 bit test.
 */
static struct test_case_pte_huge_fields {
  union pte_fields_u val;
  ac_u64 p;
  ac_u64 rw;
  ac_u64 us;
  ac_u64 pwt;
  ac_u64 pcd;
  ac_u64 a;
  ac_u64 d;
  ac_u64 ps_pte;
  ac_u64 g;
  ac_u64 reserved_0;
  ac_u64 pat;
  ac_u64 phy_addr;
  ac_u64 pke;
  ac_u64 xd;
} test_case_pte_huge_fields_array[] = {
  { .val.raw=0x1, .p=0x1, },
  { .val.raw=0x2, .rw=0x1, },
  { .val.raw=0x4, .us=0x1, },
  { .val.raw=0x8, .pwt=0x1, },
  { .val.raw=0x10, .pcd=0x1, },
  { .val.raw=0x20, .a=0x1, },
  { .val.raw=0x40, .d=0x1, },
  { .val.raw=0x80, .ps_pte=0x1, },
  { .val.raw=0x100, .g=0x1, },
  { .val.raw=0x200, .reserved_0=0x1, },
  { .val.raw=0x800, .reserved_0=0x4, },
  { .val.raw=0x1000, .pat=0x1, },
  { .val.raw=0x2000, .phy_addr=0x1, },
  { .val.raw=0x0400000000000000, .phy_addr=0x200000000000, },
  { .val.raw=0x0800000000000000, .pke=0x1, },
  { .val.raw=0x4000000000000000, .pke=0x8, },
  { .val.raw=0x8000000000000000, .xd=0x1, },
};

static ac_bool test_pte_huge_fields(struct test_case_pte_huge_fields* test) {
  ac_bool error = AC_FALSE;

  print_pte_huge_fields("", test->val.raw);

  error |= AC_TEST(test->val.huge.p == test->p);
  error |= AC_TEST(test->val.huge.rw == test->rw);
  error |= AC_TEST(test->val.huge.us == test->us);
  error |= AC_TEST(test->val.huge.pwt == test->pwt);
  error |= AC_TEST(test->val.huge.pcd == test->pcd);
  error |= AC_TEST(test->val.huge.a == test->a);
  error |= AC_TEST(test->val.huge.d == test->d);
  error |= AC_TEST(test->val.huge.ps_pte == test->ps_pte);
  error |= AC_TEST(test->val.huge.g == test->g);
  error |= AC_TEST(test->val.huge.reserved_0 == test->reserved_0);
  error |= AC_TEST(test->val.huge.pat == test->pat);
  error |= AC_TEST(test->val.huge.phy_addr == test->phy_addr);
  error |= AC_TEST(test->val.huge.pke == test->pke);
  error |= AC_TEST(test->val.huge.xd == test->xd);

  return error;
}

static ac_bool test_pte_huge_fields_array(void) {
  ac_bool error = AC_FALSE;

  for (ac_uint i = 0;
      i < AC_ARRAY_COUNT(test_case_pte_huge_fields_array); i++) {
    error |= test_pte_huge_fields(&test_case_pte_huge_fields_array[i]);
  }

  return error;
}



int main(void) {
  struct pde_fields* pml4;
  struct pde_fields* pml3_pde;
  struct pte_huge_fields* pml3_pte;
  struct pde_fields* pml2_pde;
  struct pte_huge_fields* pml2_pte;
  struct pte_small_fields* pml1_pte;
  ac_bool error = AC_FALSE;

  initialize_intr_descriptor_table();

  error |= test_cr3_nrml_paging_fields_array();
  error |= test_cr3_pae_paging_fields_array();
  error |= test_cr3_pcide_paging_fields_array();

  error |= test_pde_fields_array();
  error |= test_pte_small_fields_array();
  error |= test_pte_huge_fields_array();

  //ac_printf("\n****** Current Page Table:\n");
  //print_page_table_linear_addr(get_page_table_linear_addr(), get_page_mode());

  // Test we can create an empty page table with only the
  // recursive level 4 entry. This isn't useful but the
  // algoritm must work this way.
  pml4 = page_table_map_lin_to_phy(
      AC_NULL, AC_NULL, 0, 0, PAGE_CACHING_UNKNOWN);
  for (ac_uint i = 0; i < 511; i++) {
    error |= AC_TEST(pml4[i].p == 0);
  }
  error |= AC_TEST(pml4[511].p == 1);
  error |= AC_TEST(pml4[511].rw == 1);
  error |= AC_TEST(pml4[511].ps_pte == 0);
  error |= AC_TEST(pml4[511].phy_addr == ((ac_uptr)&pml4[0] >> 12));
  //print_page_table_linear_addr(pml4, PAGE_MODE_NRML_64BIT);

  // Test we can create a 1G page starting from nothing
  ac_printf("****** Create a new Test Page Table with ONE_GIG_PAGE_SIZE:\n");
  pml4 = page_table_map_lin_to_phy(
      AC_NULL, (void*)0, 0x0, ONE_GIG_PAGE_SIZE,
      PAGE_CACHING_STRONG_UNCACHEABLE);
  error |= AC_TEST(pml4[0].p == 1);
  error |= AC_TEST(pml4[0].rw == 1);
  error |= AC_TEST(pml4[0].ps_pte == 0);
  error |= AC_TEST(pml4[511].p == 1);
  error |= AC_TEST(pml4[511].rw == 1);
  error |= AC_TEST(pml4[511].ps_pte == 0);
  error |= AC_TEST(pml4[511].phy_addr == ((ac_uptr)&pml4[0] >> 12));
  pml3_pte = physical_to_linear_addr(pml4[0].phy_addr << 12);
  error |= AC_TEST(pml3_pte[0].p == 1);
  error |= AC_TEST(pml3_pte[0].rw == 1);
  error |= AC_TEST(pml3_pte[0].ps_pte == 1);
  error |= AC_TEST(pml3_pte[0].phy_addr == 0 >> 12);
  //print_page_table_linear_addr(pml4, PAGE_MODE_NRML_64BIT);

  // Test we can add 1G pages starting with an existing page table
  ac_printf("****** Add 3 pages Test Page Table with ONE_GIG_PAGE_SIZE:\n");
  page_table_map_lin_to_phy(
      pml4, (void*)ONE_GIG_PAGE_SIZE, ONE_GIG_PAGE_SIZE, 3 * ONE_GIG_PAGE_SIZE,
      PAGE_CACHING_STRONG_UNCACHEABLE);
  for (ac_uptr i = 0; i < 4; i++) {
    error |= AC_TEST(pml3_pte[i].p == 1);
    error |= AC_TEST(pml3_pte[i].rw == 1);
    error |= AC_TEST(pml3_pte[i].ps_pte == 1);
    error |= AC_TEST(pml3_pte[i].phy_addr == ((i * ONE_GIG_PAGE_SIZE) >> 13));
  }
  //print_page_table_linear_addr(pml4, PAGE_MODE_NRML_64BIT);

  // Test we can create a 2M page starting from nothing
  ac_printf("****** Create a new Test Page Table with TWO_MEG_PAGE_SIZE:\n");
  pml4 = page_table_map_lin_to_phy(
      AC_NULL, (void*)0x0, 0x0, TWO_MEG_PAGE_SIZE,
      PAGE_CACHING_STRONG_UNCACHEABLE);
  error |= AC_TEST(pml4[0].p == 1);
  error |= AC_TEST(pml4[0].rw == 1);
  error |= AC_TEST(pml4[0].ps_pte == 0);
  error |= AC_TEST(pml4[511].p == 1);
  error |= AC_TEST(pml4[511].rw == 1);
  error |= AC_TEST(pml4[511].ps_pte == 0);
  error |= AC_TEST(pml4[511].phy_addr == ((ac_uptr)&pml4[0] >> 12));
  pml3_pde = physical_to_linear_addr(pml4[0].phy_addr << 12);
  error |= AC_TEST(pml3_pde[0].p == 1);
  error |= AC_TEST(pml3_pde[0].rw == 1);
  error |= AC_TEST(pml3_pde[0].ps_pte == 0);
  pml2_pte = physical_to_linear_addr(pml3_pde[0].phy_addr << 12);
  error |= AC_TEST(pml2_pte[0].p == 1);
  error |= AC_TEST(pml2_pte[0].rw == 1);
  error |= AC_TEST(pml2_pte[0].ps_pte == 1);
  error |= AC_TEST(pml2_pte[0].phy_addr == 0 >> 13);
  //print_page_table_linear_addr(pml4, PAGE_MODE_NRML_64BIT);

  // Test we can add 2M pages starting with an existing page table
  ac_printf("****** Add 5 pages to Test Page Table with TWO_MEG_PAGE_SIZE:\n");
  page_table_map_lin_to_phy(
      pml4, (void*)TWO_MEG_PAGE_SIZE, TWO_MEG_PAGE_SIZE, 5 * TWO_MEG_PAGE_SIZE,
      PAGE_CACHING_STRONG_UNCACHEABLE);
  for (ac_uptr i = 0; i < 5; i++) {
    error |= AC_TEST(pml2_pte[i].p == 1);
    error |= AC_TEST(pml2_pte[i].rw == 1);
    error |= AC_TEST(pml2_pte[i].ps_pte == 1);
    error |= AC_TEST(pml2_pte[i].phy_addr == ((i * TWO_MEG_PAGE_SIZE) >> 13));
  }
  //print_page_table_linear_addr(pml4, PAGE_MODE_NRML_64BIT);

  // Test we can mix 2M and 4K pages, 0xfee00000 happens
  // to be the default address of the APIC. Which is what
  // I need to do to use the APIC
  page_table_map_lin_to_phy(pml4,
      (void*)0xfee00000, 0xfee00000, FOUR_K_PAGE_SIZE,
      PAGE_CACHING_STRONG_UNCACHEABLE);
  pml3_pde = physical_to_linear_addr(pml4[0].phy_addr << 12);
  error |= AC_TEST(pml3_pde[3].p == 1);
  error |= AC_TEST(pml3_pde[3].rw == 1);
  error |= AC_TEST(pml3_pde[3].ps_pte == 0);
  pml2_pde = physical_to_linear_addr(pml3_pde[3].phy_addr << 12);
  error |= AC_TEST(pml2_pde[503].p == 1);
  error |= AC_TEST(pml2_pde[503].rw == 1);
  error |= AC_TEST(pml2_pde[503].ps_pte == 0);
  pml1_pte = physical_to_linear_addr(pml2_pde[503].phy_addr << 12);
  error |= AC_TEST(pml1_pte[0].p == 1);
  error |= AC_TEST(pml1_pte[0].rw == 1);
  error |= AC_TEST(pml1_pte[0].phy_addr == (0xfee00000 >> 12));
  //print_page_table_linear_addr(pml4, PAGE_MODE_NRML_64BIT);

  // Test we can create a 4K page starting from nothing
  ac_printf("****** Create new Test Page Table with FOUR_K_PAGE_SIZE:\n");
  pml4 = page_table_map_lin_to_phy(
      AC_NULL, (void*)0x0, 0x0, FOUR_K_PAGE_SIZE,
      PAGE_CACHING_STRONG_UNCACHEABLE);
  error |= AC_TEST(pml4[0].p == 1);
  error |= AC_TEST(pml4[0].rw == 1);
  error |= AC_TEST(pml4[0].ps_pte == 0);
  error |= AC_TEST(pml4[511].p == 1);
  error |= AC_TEST(pml4[511].rw == 1);
  error |= AC_TEST(pml4[511].ps_pte == 0);
  error |= AC_TEST(pml4[511].phy_addr == ((ac_uptr)&pml4[0] >> 12));
  pml3_pde = physical_to_linear_addr(pml4[0].phy_addr << 12);
  error |= AC_TEST(pml3_pde[0].p == 1);
  error |= AC_TEST(pml3_pde[0].rw == 1);
  error |= AC_TEST(pml3_pde[0].ps_pte == 0);
  pml2_pde = physical_to_linear_addr(pml3_pde[0].phy_addr << 12);
  error |= AC_TEST(pml2_pde[0].p == 1);
  error |= AC_TEST(pml2_pde[0].rw == 1);
  error |= AC_TEST(pml2_pde[0].ps_pte == 0);
  pml1_pte = physical_to_linear_addr(pml2_pde[0].phy_addr << 12);
  error |= AC_TEST(pml1_pte[0].p == 1);
  error |= AC_TEST(pml1_pte[0].rw == 1);
  error |= AC_TEST(pml1_pte[0].phy_addr == 0 >> 12);
  //print_page_table_linear_addr(pml4, PAGE_MODE_NRML_64BIT);

  // Test we can add 2M pages starting with an existing page table
  // And that will be split across two pml2_pde's
  ac_printf("****** Add 2 pages to Test Page Table with FOUR_K_PAGE_SIZE:\n");
  page_table_map_lin_to_phy(
      pml4, (void*)(511 * FOUR_K_PAGE_SIZE), FOUR_K_PAGE_SIZE, 2 * FOUR_K_PAGE_SIZE,
      PAGE_CACHING_STRONG_UNCACHEABLE);
  error |= AC_TEST(pml2_pde[1].p == 1);
  error |= AC_TEST(pml2_pde[1].rw == 1);
  error |= AC_TEST(pml2_pde[1].ps_pte == 0);

  error |= AC_TEST(pml1_pte[511].p == 1);
  error |= AC_TEST(pml1_pte[511].rw == 1);
  error |= AC_TEST(pml1_pte[511].phy_addr == (511 * FOUR_K_PAGE_SIZE) >> 12);
  pml1_pte = physical_to_linear_addr(pml2_pde[1].phy_addr << 12);
  error |= AC_TEST(pml1_pte[0].p == 1);
  error |= AC_TEST(pml1_pte[0].rw == 1);
  error |= AC_TEST(pml1_pte[0].phy_addr == (512 * FOUR_K_PAGE_SIZE) >> 12);
  //print_page_table_linear_addr(pml4, PAGE_MODE_NRML_64BIT);
  
  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
