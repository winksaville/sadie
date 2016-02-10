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
#include <descriptors_x86.h>

#include <io_x86.h>
#include <cpuid_x86.h>
#include <msr_x86.h>

/** Set the GDT register from desc_ptr */
static __inline__ void set_gdt(descriptor_ptr* desc_ptr) {
  ac_u16* p = (ac_u16*)&desc_ptr->limit;
  __asm__ volatile("lgdt %0" :: "m" (*p));
}

/** Get the GDT register to desc_ptr */
static __inline__ void get_gdt(descriptor_ptr* desc_ptr) {
  ac_u16* p = (ac_u16*)&desc_ptr->limit;
  __asm__ volatile("sgdt %0" : "=m" (*p));
}

/** Set the LDT register from desc_ptr */
static __inline__ void set_ldt(descriptor_ptr* desc_ptr) {
  ac_u16* p = (ac_u16*)&desc_ptr->limit;
  __asm__ volatile("lldt %0" :: "m" (*p));
}

/** Get the LDT register to desc_ptr */
static __inline__ void get_ldt(descriptor_ptr* desc_ptr) {
  ac_u16* p = (ac_u16*)&desc_ptr->limit;
  __asm__ volatile("sldt %0" : "=m" (*p));
}

/** Set the IDT register from desc_ptr */
static __inline__ void set_idt(descriptor_ptr* desc_ptr) {
  ac_u16* p = (ac_u16*)&desc_ptr->limit;
  __asm__ volatile("lidt %0" :: "m" (*p));
}

/** Get the IDT register to desc_ptr */
static __inline__ void get_idt(descriptor_ptr* desc_ptr) {
  ac_u16* p = (ac_u16*)&desc_ptr->limit;
  __asm__ volatile("sidt %0" : "=m" (*p));
}

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

/** interrupt instruction */
#if 0
//This inline doesn't work if optimization is -O0
static __inline__ void intr(const ac_u8 num) {
  __asm__ volatile("int %0" :: "i"(num));
}
#else
#define intr(num) ({ \
  __asm__ volatile("int %0" :: "i"(num)); \
  })
#endif

/** interrupt return instruction */
static __inline__ void iret(void) {
#ifdef CPU_X86_64
  __asm__ volatile("iretq");
#else /* CPU_X86_32 */
  __asm__ volatile("iret");
#endif
}

/** hlt, halt instruction */
static __inline void hlt(void) {
  __asm__ volatile("hlt");
}

/** cli, disable interrupts */
static __inline void cli(void) {
  __asm__ volatile("cli");
}

/** sti, enable interrupts */
static __inline void sti(void) {
  __asm__ volatile("sti");
}

#endif
