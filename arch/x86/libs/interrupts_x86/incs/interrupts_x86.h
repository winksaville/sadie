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

#include <ac_inttypes.h>
#include <ac_architecture.h>
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
struct intr_gate {
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
} __attribute__((__packed__));

_Static_assert(sizeof(struct intr_gate) == INTR_GATE_SIZE,
    L"struct intr_gate is not " AC_XSTR(INTR_GATE_SIZE_STR) " bytes");

typedef struct intr_gate intr_gate;

/**
 * Task gate
 */
struct task_gate {
  ac_uptr unused_1:16;
  ac_uptr tss_seg_selector:16;
  ac_uptr unused_2:8;
  ac_uptr type:4;
  ac_uptr unused_3:1;
  ac_uptr dpl:2;
  ac_uptr p:1;
  ac_uptr unused_4:16;
} __attribute__((__packed__));

_Static_assert(sizeof(struct task_gate) == TASK_GATE_SIZE,
    L"struct intr_trap_gate is not 16 bytes");

typedef struct task_gate task_gate;


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

/** Return the bits for intr_gate.offset_lo as a ac_uptr */
#define INTR_GATE_OFFSET_LO(addr) ({ \
  ac_uptr r = ((ac_uptr)(addr) >> 0) & 0xFFFF; \
  r; \
})

/** Return the bits for intr_gate.offset_hi as a ac_uptr */
#define INTR_GATE_OFFSET_HI(addr) ({ \
  ac_uptr r = ((ac_uptr)(addr) >> 16) & INTR_GATE_OFFSET_HI_MASK; \
  r; \
})

/** Return the intr_gate.offset as an ac_uptr */
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
 * the __attribute__((__interrupt__))
 */
typedef struct intr_frame {
  ac_uptr ip;
  ac_uptr cs;
  ac_uptr flags;
  ac_uptr sp;
  ac_uptr ss;
} intr_frame;

/**
 * Pointer to an interrupt descriptor table
 */
struct idt_ptr {
    ac_u16 unused[3];   // Align descriptor_ptr.limit to an odd ac_u16 boundary
                        // so descriptor_ptr.address is on a ac_uptr boundary.
                        // This is for better performance and for user mode
                        // it avoids an alignment check fault. See the last
                        // paragraph of "Intel 64 and IA-32 Architectures
                        // Software Developer's Manual" Volume 3 chapter 3.5.1
                        // "Segment Descriptor Tables".
    volatile ac_u16 limit;    // Volatile so limit is stored when in set_gdt/ldt
    intr_gate * volatile iig; // Volatile so sd is stored when calling set_gdt/ldt
} __attribute__((__packed__));

_Static_assert(sizeof(struct idt_ptr) == IDT_PTR_SIZE,
    L"struct descriptor_ptr != " AC_XSTR(IDT_PTR_SIZE) " bytes");

typedef struct idt_ptr idt_ptr;

/** Set the IDT register from idt_ptr */
static __inline__ void set_idt(volatile idt_ptr* ptr) {
  ac_u16* p = (ac_u16*)&ptr->limit;
  __asm__ volatile("lidt %0" :: "m" (*p));
}

/** Get the IDT register to idt_ptr */
static __inline__ void get_idt(idt_ptr* ptr) {
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


typedef void (intr_handler)(struct intr_frame* frame);

typedef void (expt_handler) (struct intr_frame* frame, ac_uint error_code);

void set_intr_handler(ac_u32 intr_num, intr_handler ih);

void set_expt_handler(ac_u32 intr_num, expt_handler eh);

intr_gate* get_intr_gate(ac_u32 intr_num);

void initialize_intr_descriptor_table(void);

#endif
