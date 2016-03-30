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

#ifndef ARCH_X86_INCS_NATIVE_X86_H
#define ARCH_X86_INCS_NATIVE_X86_H

#include <ac_inttypes.h>
#include <ac_architecture.h>

/** Set Task Register which is a 16 bit selector into the GDT */
static __inline__ void set_tr(ac_u16 selector) {
  __asm__ volatile("ltr %0;" :: "r" (selector));
}

/** Get Task Register which is a 16 bit selector into the GDT */
static __inline__ ac_u16 get_tr(void) {
  ac_u16 tr;
  __asm__ volatile("str %0;" : "=r" (tr));
  return tr;
}

/** Set data segment register */
static __inline__ void set_ds(ac_u16 ds) {
  __asm__ volatile("movw %0, %%ds;" :: "r" (ds));
}

/** Get data segment register */
static __inline__ ac_u16 get_ds(void) {
  ac_u16 ds;
  __asm__ volatile("movw %%ds, %0;" : "=r" (ds));
  return ds;
}

/** Set stack segment register */
static __inline__ void set_ss(ac_u16 ss) {
  __asm__ volatile("movw %0, %%ss;" :: "r" (ss));
}

/** Get stack segment register */
static __inline__ ac_u16 get_ss(void) {
  ac_u16 ss;
  __asm__ volatile("movw %%ss, %0;" : "=r" (ss));
  return ss;
}

/** Set extra segment register */
static __inline__ void set_es(ac_u16 es) {
  __asm__ volatile("movw %0, %%es;" :: "r" (es));
}

/** Get extra segment register */
static __inline__ ac_u16 get_es(void) {
  ac_u16 es;
  __asm__ volatile("movw %%es, %0;" : "=r" (es));
  return es;
}

/** set stack pointer */
static __inline__ void set_sp(void* sp) {
#ifdef CPU_X86_64
  __asm__ volatile("movq %0, %%rsp;" :: "r" (sp));
#else /* CPU_X86_32 */
  __asm__ volatile("movl %0, %%rsp;" :: "r" (sp));
#endif
}

/** get stack pointer */
static __inline__ ac_uptr get_sp(void) {
  ac_uptr sp;
#ifdef CPU_X86_64
  __asm__ volatile("movq %%rsp, %0;" : "=r" (sp));
#else /* CPU_X86_32 */
  __asm__ volatile("movl %%esp, %0;" : "=r" (sp));
#endif
  return sp;
}

/** set base pointer */
static __inline__ void set_bp(void* bp) {
#ifdef CPU_X86_64
  __asm__ volatile("movq %0, %%rbp;" :: "g" (bp));
#else /* CPU_X86_32 */
  __asm__ volatile("movl %0, %%rbp;" :: "g" (bp));
#endif
}

/** get base pointer */
static __inline__ ac_uptr get_bp(void) {
  ac_uptr bp;
#ifdef CPU_X86_64
  __asm__ volatile("movq %%rbp, %0;" : "=r" (bp));
#else /* CPU_X86_32 */
  __asm__ volatile("movl %%ebp, %0;" : "=r" (bp));
#endif
  return bp;
}

/** get flags */
static __inline__ ac_uint get_flags(void) {
  ac_uint flags;
#ifdef CPU_X86_64
  __asm__ volatile(
      "pushfq;"
      "pop %0;" : "=r" (flags)
  );
#else /* CPU_X86_32 */
  __asm__ volatile(
      "pushfd;"
      "pop %0;" : "=r" (flags)
  );
#endif
  return flags;
}

/** set base pointer */
static __inline__ void set_dx(ac_uint val) {
#ifdef CPU_X86_64
  __asm__ volatile("movq %0, %%rdx;" :: "g" (val));
#else /* CPU_X86_32 */
  __asm__ volatile("movl %0, %%edx;" :: "g" (val));
#endif
}

/** get base pointer */
static __inline__ ac_uint get_dx(void) {
  ac_uint dx;
#ifdef CPU_X86_64
  __asm__ volatile("movq %%rdx, %0;" : "=r" (dx));
#else /* CPU_X86_32 */
  __asm__ volatile("movl %%edx, %0;" : "=r" (dx));
#endif
  return dx;
}

/** hlt, halt instruction */
static __inline void hlt(void) {
  __asm__ volatile("hlt");
}

/**
 * Return the time stamp counter plus auxilliary information
 */
static inline ac_u64 rdtsc_aux(ac_u32* aux) {
  // Execute the rdtscp, read Time Stamp Counter, instruction
  // returns the 64 bit TSC value and writes ecx to tscAux value.
  // The tscAux value is the logical cpu number and can be used
  // to determine if the thread migrated to a different cpu and
  // thus the returned value is suspect.
  ac_u32 lo, hi;
  __asm__ volatile (
      "rdtscp\n\t"
      :"=a"(lo), "=d"(hi), "=rm"(*aux));
  // tscAux = aux
  return ((ac_u64)hi << 32) | (ac_u64)lo;
}

/**
 * Return the time stack counter
 */
static inline ac_u64 rdtsc(void) {
  // Execute the rdtscp, read Time Stamp Counter, instruction
  // returns the 64 bit TSC value and writes ecx to tscAux value.
  // The tscAux value is the logical cpu number and can be used
  // to determine if the thread migrated to a different cpu and
  // thus the returned value is suspect.
  ac_u32 lo, hi;
  __asm__ volatile (
      "rdtsc\n\t"
      :"=a"(lo), "=d"(hi));
  // tscAux = aux
  return ((ac_u64)hi << 32) | (ac_u64)lo;
}

#endif
