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
 * Control Registers
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 2.5 "Control Registers"
 * Figure 2-7 "Control Registers"
 */

#ifndef ARCH_X86_CR_X86_INCS_CR_X86_H
#define ARCH_X86_CR_X86_INCS_CR_X86_H

#include <ac_architecture.h>
#include <ac_attributes.h>
#include <ac_inttypes.h>
#include <ac_xstr.h>

#ifdef CPU_X86_64
#define CR0_FIELDS_SIZE 8
#define CR3_FIELDS_SIZE 8
#define CR4_FIELDS_SIZE 8
#else
#define CR0_FIELDS_SIZE 4
#define CR3_FIELDS_SIZE 4
#define CR4_FIELDS_SIZE 4
#endif

/**
 * CR0
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 2.5 "Control Registers"
 * Figure 2-7 "Control Registers"
 */
struct AC_ATTR_PACKED cr0_fields {
  ac_uint pe:1;
  ac_uint mp:1;
  ac_uint em:1;
  ac_uint ts:1;
  ac_uint et:1;
  ac_uint ne:1;
  ac_uint reserved_0:10;
  ac_uint wp:1;
  ac_uint reserved_1:1;
  ac_uint am:1;
  ac_uint reserved_2:10;
  ac_uint nw:1;
  ac_uint cd:1;
  ac_uint pg:1;
#ifdef CPU_X86_64
  ac_uint reserved_3:32;
#endif
};

_Static_assert(sizeof(struct cr0_fields) == CR0_FIELDS_SIZE,
    L"cr0_fields is not " AC_XSTR(CR0_FIELDS_SIZE) " bytes");

union cr0_u {
  ac_uint raw;
  struct cr0_fields fields;
};

_Static_assert(sizeof(union cr0_u) == CR0_FIELDS_SIZE,
    L"cr0_u is not " AC_XSTR(CR0_FIELDS_SIZE) " bytes");

/**
 * Do not use these CR3 definitions, use the page_table_x86 module.
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 4 "Paging"
 */
struct cr3_fields {
  ac_uint val;
};

_Static_assert(sizeof(struct cr3_fields) == CR3_FIELDS_SIZE,
    L"cr3_fields is not " AC_XSTR(CR3_FIELDS_SIZE) " bytes");

union cr3_u {
  ac_uint raw;
  struct cr3_fields fields;
};

_Static_assert(sizeof(struct cr3_fields) == CR3_FIELDS_SIZE,
    L"cr3_fields is not " AC_XSTR(CR3_FIELDS_SIZE) " bytes");

/**
 * CR4
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 2.5 "Control Registers"
 * Figure 2-7 "Control Registers"
 */
struct AC_ATTR_PACKED cr4_fields {
  ac_uint vme:1;
  ac_uint pvi:1;
  ac_uint tsd:1;
  ac_uint de:1;
  ac_uint pse:1;
  ac_uint pae:1;
  ac_uint mce:1;
  ac_uint pge:1;
  ac_uint pce:1;
  ac_uint osfxsr:1;
  ac_uint osxmmexcpt:1;
  ac_uint reserved_0:2;
  ac_uint fsgsbase:1;
  ac_uint pcide:1;
  ac_uint oxsave:1;
  ac_uint reserve_1:1;
  ac_uint smep:1;
  ac_uint smap:1;
  ac_uint pke:1;
#ifdef CPU_X86_64
  ac_uint reserved_2:41;
#else /* CPU_X86_32 */
  ac_uint reserved_2:9;
#endif
};

_Static_assert(sizeof(struct cr4_fields) == CR4_FIELDS_SIZE,
    L"cr4_fields is not " AC_XSTR(CR4_FIELDS_SIZE) " bytes");

union AC_ATTR_PACKED cr4_u {
  ac_uint raw;
  struct cr4_fields fields;
};

_Static_assert(sizeof(union cr4_u) == CR4_FIELDS_SIZE,
    L"cr4_u is not " AC_XSTR(CR4_FIELDS_SIZE) " bytes");

/** get control register 0 (CR0) */
static __inline__ ac_uint get_cr0(void) {
  ac_uint ret_val;
  __asm__ __volatile__ ("mov %%cr0, %0" : "=r"(ret_val));
  return ret_val;
}

/** set control register 0 (CR0) */
static __inline__ void set_cr0(ac_uint val) {
  __asm__ volatile("mov %0, %%cr0" :: "r" (val));
}

/** get control register 2 (CR2) */
static __inline__ ac_uint get_cr2(void) {
  ac_uint ret_val;
  __asm__ __volatile__ ("mov %%cr2, %0" : "=r"(ret_val));
  return ret_val;
}

/** get control register 3 (CR3) */
static __inline__ ac_uint get_cr3(void) {
  ac_uint ret_val;
  __asm__ volatile("mov %%cr3, %0;" : "=r" (ret_val));
  return ret_val;
}

/** set control register 3 (CR3) */
static __inline__ void set_cr3(ac_uint val) {
  __asm__ volatile("mov %0, %%cr3;" :: "r" (val));
}

/** get control register 4 (CR4) */
static __inline__ ac_uint get_cr4(void) {
  ac_uint ret_val;
  __asm__ volatile("mov %%cr4, %0;" : "=r" (ret_val));
  return ret_val;
}

/** set control register 4 (CR4) */
static __inline__ void set_cr4(ac_uint val) {
  __asm__ volatile("mov %0, %%cr4;" :: "r" (val));
}

/** get control register 8 (CR8) */
static __inline__ ac_u64 get_cr8(void) {
  ac_u64 ret_val;
#ifdef CPU_X86_64
  __asm__ volatile("movq %%cr8, %0;" : "=r" (ret_val));
#else /* CPU_X86_64 */
  ret_val = 0;
#endif
  return ret_val;
}

/** set control register 8 (CR8) */
static __inline__ void set_cr8(ac_u64 val) {
#ifdef CPU_X86_64
  __asm__ volatile("movq %0, %%cr8;" :: "r" (val));
#else /* CPU_X86_32 */
  // Not implemented
#endif
}

#endif
