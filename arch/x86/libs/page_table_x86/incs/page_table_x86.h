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
  PAGE_MODE_32BIT,
  PAGE_MODE_PAE,
  PAGE_MODE_64BIT,
};

/**
 * Return the current cpu page_mode
 */
enum page_mode get_page_mode(void);

/**
 * Return the current cpu page table
 */
static inline union cr3_u get_page_table(void) {
  union cr3_u val = { .raw=get_cr3() };
  return val;
}

/**
 * Return the current cpu page table
 */
static inline void* get_page_directory_addr(union cr3_u cr3u) {
  return (void*)((ac_uptr)cr3u.fields.page_directory_base << 12);
}

#endif
