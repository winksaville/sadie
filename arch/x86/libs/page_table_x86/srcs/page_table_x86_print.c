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

void print_pml4e_fields(char* str, ac_u64 val) {
  union pml4e_fields_u reg = { .raw = val };

  print_leader(str);
  ac_printf("pml4e_fields: 0x%llx\n", reg.raw);
  ac_printf(" p=%d\n", reg.fields.p);
  ac_printf(" rw=%d\n", reg.fields.rw);
  ac_printf(" us=%d\n", reg.fields.us);
  ac_printf(" pwt=%d\n", reg.fields.pwt);
  ac_printf(" pcd=%d\n", reg.fields.pcd);
  ac_printf(" a=%d\n", reg.fields.a);
  ac_printf(" reserved_0=0x%llx\n", reg.fields.reserved_0);
  ac_printf(" phy_addr=0x%llx\n", reg.fields.phy_addr);
  ac_printf(" xd=%d\n", reg.fields.xd);
}

/**
 * print pdpte_1g_fields
 */
void print_pdpte_1g_fields(char* str, ac_u64 val) {
  union pdpte_fields_u reg = { .raw = val };

  ac_printf("pdpte_1g_fields: 0x%llx\n", reg.raw);
  ac_printf(" p=%d\n", reg.g_fields.p);
  ac_printf(" rw=%d\n", reg.g_fields.rw);
  ac_printf(" us=%d\n", reg.g_fields.us);
  ac_printf(" pwt=%d\n", reg.g_fields.pwt);
  ac_printf(" pcd=%d\n", reg.g_fields.pcd);
  ac_printf(" a=%d\n", reg.g_fields.a);
  ac_printf(" d=%d\n", reg.g_fields.d);
  ac_printf(" ps=%d\n", reg.g_fields.ps); // should be 1
  ac_printf(" g=%d\n", reg.g_fields.g);
  ac_printf(" reserved_0=0x%llx\n", reg.g_fields.reserved_0);
  ac_printf(" pat=%d\n", reg.g_fields.pat);
  ac_printf(" reserved_1=0x%llx\n", reg.g_fields.reserved_1);
  ac_printf(" phy_addr=0x%llx\n", reg.g_fields.phy_addr);
  ac_printf(" pke=%d\n", reg.g_fields.pke);
  ac_printf(" xd=%d\n", reg.g_fields.xd);
}

/**
 * print pdpte_fields
 */
void print_pdpte_fields(char* str, ac_u64 val) {
  union pdpte_fields_u reg = { .raw = val };

  print_leader(str);
  ac_printf("pdpte_fields: 0x%llx\n", reg.raw);
  ac_printf(" p=%d\n", reg.fields.p);
  ac_printf(" rw=%d\n", reg.fields.rw);
  ac_printf(" us=%d\n", reg.fields.us);
  ac_printf(" pwt=%d\n", reg.fields.pwt);
  ac_printf(" pcd=%d\n", reg.fields.pcd);
  ac_printf(" a=%d\n", reg.fields.a);
  ac_printf(" reserved_0=0x%llx\n", reg.fields.reserved_0);
  ac_printf(" phy_addr=0x%llx\n", reg.fields.phy_addr);
  ac_printf(" xd=%d\n", reg.fields.xd);
}

/**
 * print pde_2m_fields
 */
void print_pde_2m_fields(char* str, ac_u64 val) {
  union pde_fields_u reg = { .raw = val };

  print_leader(str);
  ac_printf("pde_2m_fields: 0x%llx\n", reg.raw);
  ac_printf(" p=%d\n", reg.m_fields.p);
  ac_printf(" rw=%d\n", reg.m_fields.rw);
  ac_printf(" us=%d\n", reg.m_fields.us);
  ac_printf(" pwt=%d\n", reg.m_fields.pwt);
  ac_printf(" pcd=%d\n", reg.m_fields.pcd);
  ac_printf(" a=%d\n", reg.m_fields.a);
  ac_printf(" d=%d\n", reg.m_fields.d);
  ac_printf(" ps=%d\n", reg.m_fields.ps); // should be 1
  ac_printf(" g=%d\n", reg.m_fields.g);
  ac_printf(" reserved_0=0x%llx\n", reg.m_fields.reserved_0);
  ac_printf(" pat=%d\n", reg.m_fields.pat);
  ac_printf(" reserved_1=0x%llx\n", reg.m_fields.reserved_1);
  ac_printf(" phy_addr=0x%llx\n", reg.m_fields.phy_addr);
  ac_printf(" pke=%d\n", reg.m_fields.pke);
  ac_printf(" xd=%d\n", reg.m_fields.xd);
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
  ac_printf(" phy_addr=0x%llx\n", reg.fields.phy_addr);
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
  ac_printf(" phy_addr=0x%llx\n", reg.small.phy_addr);
  ac_printf(" pke=%d\n", reg.small.pke);
  ac_printf(" xd=%d\n", reg.small.xd);
}

/**
 * print pdpte_1g_fields
 */
void print_pte_huge_fields(char* str, ac_u64 val) {
  union pte_fields_u reg = { .raw = val };

  ac_printf("pdpte_huge_fields: 0x%llx\n", reg.raw);
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
  ac_printf(" reserved_1=0x%llx\n", reg.huge.reserved_1);
  ac_printf(" phy_addr=0x%llx\n", reg.huge.phy_addr);
  ac_printf(" pke=%d\n", reg.huge.pke);
  ac_printf(" xd=%d\n", reg.huge.xd);
}


/**
 * Print the page table whose top most directory is
 * table and the mode is one of the three modes supported
 * by x86 cpus.
 */
void print_page_table(union cr3_paging_fields_u cr3u, enum page_mode mode) {
  ac_u64 base_phy_addr_field = get_pde_phy_addr_field(cr3u, mode);
  ac_u64* p_pde = get_pde_linear_addr(cr3u, mode);

  ac_printf("page directory addr=0x%p mode=%d\n", p_pde, mode);

  for (ac_uint i = 0; i < 512; i++) {
    if (*p_pde != 0) {
      union pde_fields_u pdeu = { .raw = *p_pde };
      char* str;

      if (pdeu.fields.phy_addr == base_phy_addr_field) {
        str = "recursive";
      } else {
        str = "";
      }
      print_pde_fields(str, *p_pde);
    }
    p_pde += 1;
  }
}
