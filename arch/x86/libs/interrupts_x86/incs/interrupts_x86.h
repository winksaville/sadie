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

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <ac_architecture.h>
#include <ac_attributes.h>
#include <ac_inttypes.h>
#include <ac_xstr.h>

#if !defined(ARCH_X86) && (!defined(CPU_X86_32) || !defined(CPU_X86_64))
#error  "Expecting ARCH_X86 && (CPU_X86_32 || CPU_X86_64)"
#endif

#ifdef CPU_X86_64 
#define INTR_GATE_OFFSET_HI_MASK  ((ac_uptr)0xFFFFFFFFFFFFLL)
#define INTR_GATE_SIZE  16
#define IDT_PTR_SIZE        16
#else /* CPU_X86_32 */
#define INTR_GATE_OFFSET_HI_MASK  ((ac_uptr)0xFFFF)
#define INTR_GATE_SIZE  8
#define IDT_PTR_SIZE        12
#endif

#define TASK_GATE_SIZE  8

/**
 * Interrupt Gate, this is the same as a trap gate with a different type.
 *
 * See "Intel 64 and IA-32 Architectures Software Developer's Manual"
 * Volume 3 chapter 6.11 "IDT Descriptors"
 * Figure 6-2. IDT Gate Descriptors
 *
 * and
 *
 * Volume 3 chapter 6.14.1 "64-Bit Mode IDT"
 * Figure 6-7. 64-Bit IDT Gate Descriptors
 */
typedef struct AC_ATTR_PACKED {
  ac_uptr offset_lo:16;
  ac_uptr segment:16;
#ifdef CPU_X86_64
  ac_uptr ist:3;
  ac_uptr unused_1:5;
#else /* CPU_X86_32 */
  ac_uptr unused_1:8;
#endif
  ac_uptr type:4;
  ac_uptr unused_2:1;
  ac_uptr dpl:2;
  ac_uptr p:1;
#ifdef CPU_X86_64
  ac_uptr offset_hi:48;
  ac_uptr unused_3:32;
#else /* CPU_X86_32 */
  ac_uptr offset_hi:16;
#endif
} IntrGate;

_Static_assert(sizeof(IntrGate) == INTR_GATE_SIZE,
    L"IntrGate is not " AC_XSTR(INTR_GATE_SIZE_STR) " bytes");

/**
 * Task gate
 */
typedef struct AC_ATTR_PACKED {
  ac_uptr unused_1:16;
  ac_uptr tss_seg_selector:16;
  ac_uptr unused_2:8;
  ac_uptr type:4;
  ac_uptr unused_3:1;
  ac_uptr dpl:2;
  ac_uptr p:1;
  ac_uptr unused_4:16;
} TaskGate;

_Static_assert(sizeof(TaskGate) == TASK_GATE_SIZE,
    L"TaskGate is not 16 bytes");

#define INTR_GATE_COMMON_INITIALIZER \
   .offset_lo = 0, \
   .segment = 0, \
   .unused_1 = 0, \
   .type = 0, \
   .unused_2 = 0, \
   .dpl = 0, \
   .p = 0, \
   .offset_hi = 0, \

#define INTR_GATE_X86_64_EXTRA_INITIALIZER \
  .ist = 0, \
  .unused_3 = 0,

#ifdef CPU_X86_64
#define INTR_GATE_INITIALIZER { \
   INTR_GATE_COMMON_INITIALIZER \
   INTR_GATE_X86_64_EXTRA_INITIALIZER \
}
#else /* CPU_X86_32 */
#define INTR_GATE_INITIALIZER { \
   INTR_GATE_COMMON_INITIALIZER \
}
#endif

/** Return the bits for IntrGate.offset_lo as a ac_uptr */
#define INTR_GATE_OFFSET_LO(addr) ({ \
  ac_uptr r = ((ac_uptr)(addr) >> 0) & 0xFFFF; \
  r; \
})

/** Return the bits for IntrGate.offset_hi as a ac_uptr */
#define INTR_GATE_OFFSET_HI(addr) ({ \
  ac_uptr r = ((ac_uptr)(addr) >> 16) & INTR_GATE_OFFSET_HI_MASK; \
  r; \
})

/** Return the IntrGate.offset as an ac_uptr */
#define GET_INTR_GATE_OFFSET(gate) ({ \
  ac_uptr r = (ac_uptr)((((ac_uptr)(gate).offset_hi) << 16) \
      | (ac_uptr)((gate).offset_lo)); \
  r; \
})

 
/**
 * Interrupt stack frame
 *
 * This is the signature required for the compiler so
 * intr_handler and expt_handler are can be marked with
 * the AC_ATTR_INTR_HDLR
 */
typedef struct {
  ac_uptr ip;
  ac_uptr cs;
  ac_uptr flags;
  ac_uptr sp;
  ac_uptr ss;
} IntrFrame;

/**
 * Pointer to an interrupt descriptor table
 */
typedef struct AC_ATTR_PACKED {
    ac_u16 unused[3];   // Align descriptor_ptr.limit to an odd ac_u16 boundary
                        // so descriptor_ptr.address is on a ac_uptr boundary.
                        // This is for better performance and for user mode
                        // it avoids an alignment check fault. See the last
                        // paragraph of "Intel 64 and IA-32 Architectures
                        // Software Developer's Manual" Volume 3 chapter 3.5.1
                        // "Segment Descriptor Tables".
    volatile ac_u16 limit;    // Volatile so limit is stored when in set_gdt/ldt
    IntrGate* volatile iig; // Volatile so sd is stored when calling set_gdt/ldt
} IdtPtr;

_Static_assert(sizeof(IdtPtr) == IDT_PTR_SIZE,
    L"IdtPtr != " AC_XSTR(IDT_PTR_SIZE) " bytes");

/** Set the IDT register from IdtPtr */
static __inline__ void set_idt(volatile IdtPtr* ptr) {
  ac_u16* p = (ac_u16*)&ptr->limit;
  __asm__ volatile("lidt %0" :: "m" (*p));
}

/** Get the IDT register to IdtPtr */
static __inline__ void get_idt(IdtPtr* ptr) {
  ac_u16* p = (ac_u16*)&ptr->limit;
  __asm__ volatile("sidt %0" : "=m" (*p));
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

/** cli, disable interrupts */
static __inline void cli(void) {
  __asm__ volatile("cli");
}

/** sti, enable interrupts */
static __inline void sti(void) {
  __asm__ volatile("sti");
}

/** Disable interrupts, and return current flags */
static __inline ac_uint disable_intr(void) {
ac_uint flags;
#ifdef CPU_X86_64

  __asm__ volatile(
      "pushfq;"
      "pop %0;"
      "cli" : "=rm" (flags) ::"memory");
#else /* CPU_X86_32 */
  __asm__ volatile(
      "pushfd;"
      "pop %0;"
      "cli" : "=rm" (flags) ::"memory");
#endif
  return flags;
}

/** Enable interrupts, and return current flags */
static __inline ac_uint enable_intr(void) {
  ac_uint flags;
#ifdef CPU_X86_64

  __asm__ volatile(
      "pushfq;"
      "pop %0;"
      "sti" : "=rm" (flags) ::"memory");
#else /* CPU_X86_32 */
  __asm__ volatile(
      "pushfd;"
      "pop %0;"
      "sti" : "=rm" (flags) ::"memory");
#endif
  return flags;
}

/** Restore interrupts to previous mode as retunred by disable_intr */
static __inline void restore_intr(ac_uint flags) {
#ifdef CPU_X86_64
  __asm__ volatile(
      "push %0;"
      "popfq;" : : "g" (flags) :"memory", "cc");
#else /* CPU_X86_32 */
  __asm__ volatile(
      "push %0;"
      "popfq;" : : "g" (flags) :"memory", "cc");
#endif
}


typedef void (intr_handler)(IntrFrame* frame);

typedef void (expt_handler) (IntrFrame* frame, ac_uint error_code);

void set_intr_handler(ac_u32 intr_num, intr_handler ih);

void set_expt_handler(ac_u32 intr_num, expt_handler eh);

IntrGate* get_intr_gate(ac_u32 intr_num);

void initialize_intr_descriptor_table(void);

#endif
