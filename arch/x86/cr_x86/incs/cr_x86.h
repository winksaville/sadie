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

#ifndef ARCH_X86_CR_X86_INCS_CR_X86_H
#define ARCH_X86_CR_X86_INCS_CR_X86_H

#include <ac_inttypes.h>

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
