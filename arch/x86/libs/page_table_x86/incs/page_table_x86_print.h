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

#ifndef ARCH_X86_PAGE_TABLE_X86_INCS_PAGE_TABLE_X86_PRINT_H
#define ARCH_X86_PAGE_TABLE_X86_INCS_PAGE_TABLE_X86_PRINT_H


#include <page_table_x86.h>
#include <cr_x86.h>

#include <ac_inttypes.h>

void print_cr3_nrml_paging_fields(char* str, ac_uint cr3);
void print_cr3_pae_paging_fields(char* str, ac_uint cr3);
void print_cr3_pcide_paging_fields(char* str, ac_uint cr3);

/**
 * print Page Map Level 4 Entries.
 */
void print_pml4e_fields(char* str, ac_u64 val);

/**
 * print pdpte_1g_fields
 */
void print_pdpte_1g_fields(char* str, ac_u64 val);

/**
 * print pdpte_fields
 */
void print_pdpte_fields(char* str, ac_u64 val);

/**
 * print pde_2m_fields
 */
void print_pde_2m_fields(char* str, ac_u64 val);

/**
 * print pde_fields
 */
void print_pde_fields(char* str, ac_u64 val);

/**
 * print pte_small_fields
 */
void print_pte_small_fields(char* str, ac_u64 val);

/**
 * print pte_huge_fields
 */
void print_pte_huge_fields(char* str, ac_u64 val);


/**
 * Print a page table.
 */
void print_page_table(union cr3_paging_fields_u cr3u, enum page_mode mode);

#endif
