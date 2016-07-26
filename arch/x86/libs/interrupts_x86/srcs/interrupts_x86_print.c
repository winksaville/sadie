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

#include <interrupts_x86.h>
#include <interrupts_x86_print.h>

#include <ac_assert.h>
#include <ac_attributes.h>
#include <ac_inttypes.h>
#include <ac_printf.h>

#define NUM_SAVED_REGS 15
struct saved_regs {
  ac_u64 rax;
  ac_u64 rdx;
  ac_u64 rcx;
  ac_u64 rbx;
  ac_u64 rsi;
  ac_u64 rdi;
  ac_u64 r8;
  ac_u64 r9;
  ac_u64 r10;
  ac_u64 r11;
  ac_u64 r12;
  ac_u64 r13;
  ac_u64 r14;
  ac_u64 r15;
  ac_u64 rbp;
};

struct AC_ATTR_PACKED full_expt_stack_frame {
  union {
    struct saved_regs regs;
    ac_u64 regs_array[NUM_SAVED_REGS];
  };
  ac_u64 error_code;
  IntrFrame iret_frame;
};

#define FULL_EXPT_STACK_FRAME_SIZE (NUM_SAVED_REGS + 5 + 1) * sizeof(ac_u64)
ac_static_assert(sizeof(struct full_expt_stack_frame) == FULL_EXPT_STACK_FRAME_SIZE,
    "full_expt_stack_frame is not " AC_XSTR(EXPT_FULL_STACK_FRAME_SIZE) " bytes in size");

struct AC_ATTR_PACKED full_intr_stack_frame {
  union {
    struct saved_regs regs;
    ac_u64 regs_array[NUM_SAVED_REGS];
  };
  IntrFrame iret_frame;
};

#define FULL_INTR_STACK_FRAME_SIZE (NUM_SAVED_REGS + 5) * sizeof(ac_u64)
ac_static_assert(sizeof(struct full_intr_stack_frame) == FULL_INTR_STACK_FRAME_SIZE,
    "full_intr_stack_frame is not " AC_XSTR(FULL_INTR_STACK_FRAME_SIZE) " bytes in size");

/**
 * Print full stack frame
 */
void print_saved_regs(struct saved_regs* sr) {
  ac_printf(" rax: 0x%lx 0x%p\n", sr->rax, &sr->rax);
  ac_printf(" rdx: 0x%lx 0x%p\n", sr->rdx, &sr->rdx);
  ac_printf(" rcx: 0x%lx 0x%p\n", sr->rcx, &sr->rcx);
  ac_printf(" rbx: 0x%lx 0x%p\n", sr->rbx, &sr->rbx);
  ac_printf(" rsi: 0x%lx 0x%p\n", sr->rsi, &sr->rsi);
  ac_printf(" rdi: 0x%lx 0x%p\n", sr->rdi, &sr->rdi);
  ac_printf("  r8: 0x%lx 0x%p\n", sr->r8,  &sr->r8);
  ac_printf("  r9: 0x%lx 0x%p\n", sr->r9,  &sr->r9);
  ac_printf(" r10: 0x%lx 0x%p\n", sr->r10, &sr->r10);
  ac_printf(" r11: 0x%lx 0x%p\n", sr->r11, &sr->r11);
  ac_printf(" r12: 0x%lx 0x%p\n", sr->r12, &sr->r12);
  ac_printf(" r13: 0x%lx 0x%p\n", sr->r13, &sr->r13);
  ac_printf(" r14: 0x%lx 0x%p\n", sr->r14, &sr->r14);
  ac_printf(" r15: 0x%lx 0x%p\n", sr->r15, &sr->r15);
  ac_printf(" rbp: 0x%lx 0x%p\n", sr->rbp, &sr->rbp);
}

/**
 * Print full stack frame
 */
void print_full_expt_stack_frame(char* str, IntrFrame* f) {
  if (str != AC_NULL) {
    ac_printf("%s:", str);
  }
  // Calculate the fsf subract saved_regs and error_code
  struct full_expt_stack_frame* fsf = (void*)f - sizeof(struct saved_regs) - sizeof(ac_u64);
  ac_printf("fsf=0x%p\n", fsf);
  print_saved_regs(&fsf->regs);
  ac_printf(" err: 0x%lx 0x%p\n", fsf->error_code, &fsf->error_code);
  print_intr_frame(AC_NULL, &fsf->iret_frame);
}

/**
 * Print full iintr stack frame
 */
void print_full_intr_stack_frame(char* str, IntrFrame* f) {
  if (str != AC_NULL) {
    ac_printf("%s:", str);
  }
  // Calculate the fsf subract saved_regs
  struct full_intr_stack_frame* fsf = (void*)f - sizeof(struct saved_regs);
  ac_printf("fsf=0x%p\n", fsf);
  print_saved_regs(&fsf->regs);
  print_intr_frame(AC_NULL, &fsf->iret_frame);
}

void print_intr_frame(char* str, IntrFrame* frame) {
  if (str != AC_NULL) {
    ac_printf("%s:\n", str);
  }
  ac_printf(" frame->ip:    0x%p 0x%p\n", frame->ip, &frame->ip);
  ac_printf(" frame->cs:    0x%p 0x%p\n", frame->cs, &frame->cs);
  ac_printf(" frame->flags: 0x%p 0x%p\n", frame->flags, &frame->flags);
  ac_printf(" frame->sp:    0x%p 0x%p\n", frame->sp, &frame->sp);
  ac_printf(" frame->ss:    0x%x 0x%p\n", frame->ss, &frame->ss);
}

void print_intr_gate(char *str, IntrGate* gate) {
  if (str != AC_NULL) {
    ac_printf("%s:\n", str);
  }
  ac_printf(" gate->offset_lo: %p\n", gate->offset_lo);
  ac_printf(" gate->segment: %p\n", gate->segment);
#ifdef CPU_X86_64
  ac_printf(" gate->ist: %d\n", gate->ist);
#endif
  ac_printf(" gate->unused_1: %d\n", gate->unused_1);
  ac_printf(" gate->type: %d\n", gate->type);
  ac_printf(" gate->unused_2: %d\n", gate->unused_2);
  ac_printf(" gate->dpl: %d\n", gate->dpl);
  ac_printf(" gate->p: %d\n", gate->p);
  ac_printf(" gate->offset_hi: %p\n", gate->offset_hi);
#ifdef CPU_X86_64
  ac_printf(" gate->unused_3: %d\n", gate->unused_3);
#endif
  ac_printf(" gate->offset: %p\n", GET_INTR_GATE_OFFSET(*gate));
}
