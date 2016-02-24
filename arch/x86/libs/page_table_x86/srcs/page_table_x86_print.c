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

#include <page_table_x86_print.h>

#include <ac_inttypes.h>
#include <ac_printf.h>

static void print_leader(char *str) {
  char* space;

  if ((str == AC_NULL) || (str[0] == 0)) {
    str = "";
    space = "";
  } else {
    space = " ";
  }

  ac_printf("%s%s", str, space);
}

void print_cr3_nrml_paging_fields(char* str, ac_uint cr3) {
  union cr3_paging_fields_u reg = { .raw = cr3 };

  print_leader(str);
  ac_printf("cr3_nrml_fields: 0x%x\n", reg.raw);
  ac_printf(" pwt=%d\n", reg.nrml_paging_fields.pwt);
  ac_printf(" pcd=%d\n", reg.nrml_paging_fields.pcd);
  ac_printf(" page_directory_base=0x%x\n", reg.nrml_paging_fields.page_directory_base);
}

void print_cr3_pae_paging_fields(char* str, ac_uint cr3) {
  union cr3_paging_fields_u reg = { .raw = cr3 };

  print_leader(str);
  ac_printf("cr3_pae_fields: 0x%p\n", reg.raw);
  ac_printf(" page_directory_base=0x%x\n", reg.pae_paging_fields.page_directory_base);
}

void print_cr3_pcide_paging_fields(char* str, ac_uint cr3) {
  union cr3_paging_fields_u reg = { .raw = cr3 };

  print_leader(str);
  ac_printf("cr3_pcide_fields: 0x%p\n", reg.raw);
  ac_printf(" page_directory_base=0x%x\n", reg.pae_paging_fields.page_directory_base);
}

/**
 * print pde_fields
 */
void print_pde_fields(char* str, ac_u64 val) {
  union pde_fields_u reg = { .raw = val };

  print_leader(str);
  ac_printf("pde_fields: 0x%llx\n", reg.raw);
  ac_printf(" p=%d\n", reg.fields.p);
  ac_printf(" rw=%d\n", reg.fields.rw);
  ac_printf(" us=%d\n", reg.fields.us);
  ac_printf(" pwt=%d\n", reg.fields.pwt);
  ac_printf(" pcd=%d\n", reg.fields.pcd);
  ac_printf(" a=%d\n", reg.fields.a);
  ac_printf(" reserved_0=0x%llx\n", reg.fields.reserved_0);
  ac_printf(" pte=%d\n", reg.fields.pte);
  ac_printf(" reserved_1=0x%llx\n", reg.fields.reserved_1);
  ac_printf(" phy_addr=0x%llx 0x%llx\n", reg.fields.phy_addr,
      (ac_uptr)reg.fields.phy_addr << 12);
  ac_printf(" xd=%d\n", reg.fields.xd);
}

/**
 * print pte_small_fields
 */
void print_pte_small_fields(char* str, ac_u64 val) {
  union pte_fields_u reg = { .raw = val };

  print_leader(str);
  ac_printf("pte_small_fields: 0x%llx\n", reg.raw);
  ac_printf(" p=%d\n", reg.small.p);
  ac_printf(" rw=%d\n", reg.small.rw);
  ac_printf(" us=%d\n", reg.small.us);
  ac_printf(" pwt=%d\n", reg.small.pwt);
  ac_printf(" pcd=%d\n", reg.small.pcd);
  ac_printf(" a=%d\n", reg.small.a);
  ac_printf(" d=%d\n", reg.small.d);
  ac_printf(" pat=%d\n", reg.small.pat);
  ac_printf(" g=%d\n", reg.small.g);
  ac_printf(" reserved_0=0x%llx\n", reg.small.reserved_0);
  ac_printf(" phy_addr=0x%llx 0x%llx\n", reg.small.phy_addr,
      (ac_uptr)reg.small.phy_addr << 12);
  ac_printf(" pke=%d\n", reg.small.pke);
  ac_printf(" xd=%d\n", reg.small.xd);
}

/**
 * print pte_huge_fields, 1g, 2m
 */
void print_pte_huge_fields(char* str, ac_u64 val) {
  union pte_fields_u reg = { .raw = val };

  ac_printf("pte_huge_fields: 0x%llx\n", reg.raw);
  ac_printf(" p=%d\n", reg.huge.p);
  ac_printf(" rw=%d\n", reg.huge.rw);
  ac_printf(" us=%d\n", reg.huge.us);
  ac_printf(" pwt=%d\n", reg.huge.pwt);
  ac_printf(" pcd=%d\n", reg.huge.pcd);
  ac_printf(" a=%d\n", reg.huge.a);
  ac_printf(" d=%d\n", reg.huge.d);
  ac_printf(" ps=%d\n", reg.huge.ps); // should be 1
  ac_printf(" g=%d\n", reg.huge.g);
  ac_printf(" reserved_0=0x%llx\n", reg.huge.reserved_0);
  ac_printf(" pat=%d\n", reg.huge.pat);
  ac_printf(" phy_addr=0x%llx 0x%llx\n", reg.huge.phy_addr,
      (ac_uptr)reg.huge.phy_addr << 13);
  ac_printf(" pke=%d\n", reg.huge.pke);
  ac_printf(" xd=%d\n", reg.huge.xd);
}

/**
 * Recursive algorithm to print either
 * page directory entires (pde) or page table entries (pte)
 */
static void print_pde_pte(ac_uint level, ac_u64* p_base) {
  union pde_fields_u pdeu = { .raw = *p_base };
  ac_u64* p_entry;

  ac_printf("first pass level=%d base=0x%p\n", level, p_base);
  if ((level > 1) && (pdeu.fields.pte == 0)) {
    // This is a pde entry, print all entries that are present
    p_entry = p_base;
    for (ac_uint i = 0; i < 512; i++) {
      pdeu.raw = *p_entry;
      if (pdeu.fields.p == 1) {
        char* str;

        ac_printf("level=%d i=%d\n", level, i);
        if (((ac_u64)pdeu.fields.phy_addr << 12) == (ac_u64)p_base) {
          ac_printf("pdeu.fields.phy_addr=%llx p_base=0x%llx\n",
              pdeu.fields.phy_addr, p_base);
          str = "recursive";
        } else {
          str = "";
        }
        print_pde_fields(str, *p_entry);
      }
      p_entry += 1;
    }
  }

  ac_printf("\nsecond pass level=%d base=0x%p\n", level, p_base);
  p_entry = p_base;
  for (ac_uint i = 0; i < 512; i++) {
    pdeu.raw = *p_entry;

    if (pdeu.fields.p == 1) {
      if (level == 1) {
        // Is a small pte, print its fields
        print_pte_small_fields("Small page", *p_entry);
      } else if ((level > 1) && (pdeu.fields.pte == 1)) {
        // Is a huge pte, print its fields
        print_pte_huge_fields("Huge page", *p_entry);
      } else {
        // Is a pde
        if (((ac_u64)pdeu.fields.phy_addr << 12) != (ac_u64)p_base) {
          // A non-recrusive entry so recurse into it
          print_pde_pte(level - 1, (ac_u64*)((ac_u64)pdeu.fields.phy_addr << 12));
        } else {
          // Is a recursive entry so skip
          ac_printf("Skipping recursive entry level=%d i=%d\n", level, i);
        }
      }
    }
    p_entry += 1;
  }
  ac_printf("leaving level=%d base=0x%p\n", level, p_base);
}

/**
 * Print the page table pointed to by cr3u and one of the modes.
 */
void print_page_table_linear(void* linear_base_addr, enum page_mode mode) {
  ac_u64* p_pde_base = (ac_u64*)linear_base_addr;

  ac_printf("page directory addr=0x%p mode=%d\n", p_pde_base, mode);

  switch(mode) {
    case PAGE_MODE_NRML_32BIT: {
      ac_printf("PAGE_MODE_NRML_32BIT Unsupported  addr=0x%p mode=%d\n",
          p_pde_base, mode);
      break;
    }
    case PAGE_MODE_PAE_32BIT: {
      ac_printf("PAGE_MODE_PAE_32BIT Unsupported  addr=0x%p mode=%d\n",
          p_pde_base, mode);
      break;
    }
    case PAGE_MODE_NRML_64BIT:
    case PAGE_MODE_PCIDE_64BIT: {
      print_pde_pte(4, p_pde_base);
      break;
    }
    default: {
      ac_printf("Unknown page directory addr=0x%p mode=%d\n", p_pde_base, mode);
    }
  }
}

/**
 * Print the page table pointed to by cr3u and one of the modes.
 */
void print_page_table(union cr3_paging_fields_u cr3u, enum page_mode mode) {
  print_page_table_linear(get_cr3_pde_linear_addr(cr3u, mode), mode);
}
