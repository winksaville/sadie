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

#ifndef ARCH_X86_PAGE_TABLE_X86_INCS_PAGE_TABLE_X86_H
#define ARCH_X86_PAGE_TABLE_X86_INCS_PAGE_TABLE_X86_H

#include <cr_x86.h>

#include <ac_inttypes.h>

enum page_mode {
  PAGE_MODE_UNKNOWN,
  PAGE_MODE_NRML_32BIT,
  PAGE_MODE_PAE_32BIT,
  PAGE_MODE_NRML_64BIT,
  PAGE_MODE_PCIDE_64BIT,
};

/**
 * CR3 is used for defining the physical address of the page
 * directory base.  See page_table_x86 directory for more info.
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
