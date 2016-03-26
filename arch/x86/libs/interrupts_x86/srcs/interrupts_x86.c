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
#include <ac_inttypes.h>
#include <ac_printf.h>

#include <descriptors_x86.h>
#include <native_x86.h>
#include <reset_x86.h>

/* Interrupt Descriptor Table */
static intr_gate idt[256];

static ac_u32 intr_divide_err_counter;
static ac_u32 intr_debug_counter;
static ac_u32 intr_nmi_counter;
static ac_u32 intr_breakpoint_counter;
static ac_u32 intr_overflow_counter;
static ac_u32 intr_bound_range_counter;
static ac_u32 intr_invalid_opcode_counter;
static ac_u32 intr_device_not_available_counter;
static ac_u32 expt_double_fault_counter;
static ac_u32 intr_coprocessor_segment_overrun_counter;
static ac_u32 expt_invalid_tss_counter;
static ac_u32 expt_segment_not_present_counter;
static ac_u32 expt_stack_fault_counter;
static ac_u32 expt_general_protection_counter;
static ac_u32 expt_page_fault_counter;
static ac_u32 intr_x87_floating_point_counter;
static ac_u32 expt_alignment_check_counter;
static ac_u32 intr_machine_check_counter;
static ac_u32 intr_simd_floating_point_counter;
static ac_u32 intr_virtualization_counter;

static ac_u32 intr_undefined_counter;
static ac_u32 expt_undefined_counter;

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

struct full_expt_stack_frame {
  union {
    struct saved_regs regs;
    ac_u64 regs_array[NUM_SAVED_REGS];
  };
  ac_u64 error_code;
  struct intr_frame iret_frame;
} __attribute__ ((__packed__));

#define FULL_EXPT_STACK_FRAME_SIZE (NUM_SAVED_REGS + 5 + 1) * sizeof(ac_u64)
ac_static_assert(sizeof(struct full_expt_stack_frame) == FULL_EXPT_STACK_FRAME_SIZE,
    "full_expt_stack_frame is not " AC_XSTR(EXPT_FULL_STACK_FRAME_SIZE) " bytes in size");

/**
 * Print full stack frame
 */
void print_full_expt_stack_frame(char* str, struct intr_frame* f) {
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

struct full_intr_stack_frame {
  union {
    struct saved_regs regs;
    ac_u64 regs_array[NUM_SAVED_REGS];
  };
  struct intr_frame iret_frame;
} __attribute__ ((__packed__));

#define FULL_INTR_STACK_FRAME_SIZE (NUM_SAVED_REGS + 5) * sizeof(ac_u64)
ac_static_assert(sizeof(struct full_intr_stack_frame) == FULL_INTR_STACK_FRAME_SIZE,
    "full_intr_stack_frame is not " AC_XSTR(FULL_INTR_STACK_FRAME_SIZE) " bytes in size");

/**
 * Print full iintr stack frame
 */
void print_full_intr_stack_frame(char* str, struct intr_frame* f) {
  if (str != AC_NULL) {
    ac_printf("%s:", str);
  }
  // Calculate the fsf subract saved_regs
  struct full_intr_stack_frame* fsf = (void*)f - sizeof(struct saved_regs);
  ac_printf("fsf=0x%p\n", fsf);
  print_saved_regs(&fsf->regs);
  print_intr_frame(AC_NULL, &fsf->iret_frame);
}

INTERRUPT_HANDLER
static void intr_undefined(struct intr_frame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#UD xx", frame);

  intr_undefined_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_undefined_counter: %d\n", intr_undefined_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void intr_divide_err(struct intr_frame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#DE 0", frame);

  intr_divide_err_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_divide_err_counter: %d\n", intr_divide_err_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void intr_debug(struct intr_frame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#DB 1", frame);

  intr_debug_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_debug_counter: %d\n", intr_debug_counter);
  reset_x86();
}


INTERRUPT_HANDLER
static void intr_nmi(struct intr_frame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#NMI 2", frame);

  intr_nmi_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_nmi_counter: %d\n", intr_nmi_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void intr_breakpoint(struct intr_frame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#BP 3", frame);

  intr_breakpoint_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_breakpoint_counter: %d\n", intr_breakpoint_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void intr_overflow(struct intr_frame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#OF 4", frame);

  intr_overflow_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_overflow_counter: %d\n", intr_overflow_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void intr_bound_range(struct intr_frame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#BR 5", frame);

  intr_bound_range_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_bound_range_counter: %d\n", intr_bound_range_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void intr_invalid_opcode(struct intr_frame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#UD 6", frame);

  intr_invalid_opcode_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_invlid_opcode_counter: %d\n", intr_invalid_opcode_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void intr_device_not_available(struct intr_frame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#UD 7", frame);

  intr_device_not_available_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_device_not_available_counter: %d\n", intr_device_not_available_counter);
  reset_x86();
}


INTERRUPT_HANDLER
static void expt_double_fault(struct intr_frame *frame, ac_uint error_code) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_expt_stack_frame("#DF 8", frame);

  expt_double_fault_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: 0x%x\n", error_code);
  ac_printf(" expt_double_fault_counter: %d\n", expt_double_fault_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void intr_coprocessor_segment_overrun(struct intr_frame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#CPSO 9", frame);

  intr_coprocessor_segment_overrun_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" coprocessor_segment_overrun_counter: %d\n", intr_coprocessor_segment_overrun_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void expt_invalid_tss(struct intr_frame *frame, ac_uint error_code) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_expt_stack_frame("#TS 10", frame);

  expt_invalid_tss_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: 0x%x\n", error_code);
  ac_printf(" expt_invalid_tss_counter: %d\n", expt_invalid_tss_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void expt_segment_not_present(struct intr_frame *frame, ac_uint error_code) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_expt_stack_frame("#NP 11", frame);

  expt_segment_not_present_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: 0x%x\n", error_code);
  ac_printf(" expt_segment_not_present_counter: %d\n", expt_segment_not_present_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void expt_stack_fault(struct intr_frame *frame, ac_uint error_code) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#SS 12", frame);

  expt_stack_fault_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: 0x%x\n", error_code);
  ac_printf(" expt_stack_fault_counter: %d\n", expt_stack_fault_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void expt_general_protection(struct intr_frame *frame, ac_uint error_code) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_expt_stack_frame("#GP13", frame);

  expt_general_protection_counter += 1;
  ac_printf(" expt_general_protection_counter: %d\n", expt_general_protection_counter);

  idt_ptr idtp;
  get_idt(&idtp);
  ac_printf(" idt.limit=%d\n", idtp.limit);
  ac_printf(" idt.iig=%p\n", idtp.iig);
  reset_x86();
}

INTERRUPT_HANDLER
static void expt_page_fault(struct intr_frame *frame, ac_uint error_code) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_expt_stack_frame("#PF 14", frame);

  expt_page_fault_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: 0x%x\n", error_code);
  ac_printf(" expt_page_fault_counter: %d\n", expt_page_fault_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void intr_x87_floating_point(struct intr_frame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#MF 16", frame);

  intr_x87_floating_point_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_x87_floating_point_counter: %d\n", intr_x87_floating_point_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void expt_alignment_check(struct intr_frame *frame, ac_uint error_code) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_expt_stack_frame("#AC 17", frame);

  expt_alignment_check_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: 0x%x\n", error_code);
  ac_printf(" expt_alignment_check_counter: %d\n", expt_alignment_check_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void intr_machine_check(struct intr_frame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#MC 18", frame);

  intr_machine_check_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_machine_check_counter: %d\n", intr_machine_check_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void intr_simd_floating_point(struct intr_frame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_intr_stack_frame("#XM 19", frame);

  intr_simd_floating_point_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_simd_floating_point_counter: %d\n", intr_simd_floating_point_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void intr_virtualization(struct intr_frame *frame) {
  __asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", // "rbp", rbp is already saved
                         "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15");
  print_full_expt_stack_frame("#VE 20", frame);

  intr_virtualization_counter += 1;
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_virtualization_counter: %d\n", intr_virtualization_counter);
  reset_x86();
}

static void set_intr_gate(intr_gate* gate, intr_handler* ih) {
  static intr_gate g = INTR_GATE_INITIALIZER;
  *gate = g;

  gate->offset_lo = INTR_GATE_OFFSET_LO(ih);
  gate->offset_hi = INTR_GATE_OFFSET_HI(ih);
  gate->segment = 8; // Code Segment TI=0, RPL=0
                     // FIXME: How to know where the Code Segement is
#ifdef CPU_X86_64
  gate->ist = 0; // Modified legacy stack switching mode
#endif
  gate->type = DT_64_INTR_GATE;
  gate->dpl = 0;
  gate->p = 1;
}

static void set_expt_gate(intr_gate* gate, expt_handler* eh) {
  static intr_gate g = INTR_GATE_INITIALIZER;
  *gate = g;

  gate->offset_lo = INTR_GATE_OFFSET_LO(eh);
  gate->offset_hi = INTR_GATE_OFFSET_HI(eh);
  gate->segment = 8; // Code Segment TI=0, RPL=0
                     // FIXME: How to know where the Code Segement is
#ifdef CPU_X86_64
  gate->ist = 0; // Modified legacy stack switching mode
#endif
  gate->type = DT_64_INTR_GATE;
  gate->dpl = 0;
  gate->p = 1;
}

static void set_idtr(intr_gate idt[], ac_u32 count) {
  idt_ptr dp;
  dp.limit = (ac_u16)(((ac_uptr)&idt[count] - (ac_uptr)&idt[0] - 1)
      & 0xFFFF);
  dp.iig = &idt[0];
  set_idt(&dp);
}

void set_intr_handler(ac_u32 intr_num, intr_handler ih) {
  set_intr_gate(&idt[intr_num], ih);
}

void set_expt_handler(ac_u32 intr_num, expt_handler eh) {
  set_expt_gate(&idt[intr_num], eh);
}

intr_gate* get_intr_gate(ac_u32 intr_num) {
  return &idt[intr_num];
}

void initialize_intr_descriptor_table(void) {
  intr_undefined_counter = 0;
  expt_undefined_counter = 0;

  intr_divide_err_counter = 0;
  intr_debug_counter = 0;
  intr_nmi_counter = 0;
  intr_breakpoint_counter = 0;
  intr_overflow_counter = 0;
  intr_bound_range_counter = 0;
  intr_invalid_opcode_counter = 0;
  expt_double_fault_counter = 0;
  expt_invalid_tss_counter = 0;
  expt_segment_not_present_counter = 0;
  expt_stack_fault_counter = 0;
  expt_general_protection_counter = 0;
  expt_page_fault_counter = 0;
  intr_x87_floating_point_counter = 0;
  expt_alignment_check_counter = 0;
  intr_machine_check_counter = 0;
  intr_simd_floating_point_counter = 0;
  intr_virtualization_counter = 0;

  // Initialize all of the entires to intr_undefined
  for (ac_u64 intr_num = 0; intr_num < AC_ARRAY_COUNT(idt); intr_num++) {
    set_intr_gate(&idt[intr_num], intr_undefined);
  }
  // Initialize interrupts and exceptions
  set_intr_gate(&idt[0], intr_divide_err);
  set_intr_gate(&idt[1], intr_debug);
  set_intr_gate(&idt[2], intr_nmi);
  set_intr_gate(&idt[3], intr_breakpoint);
  set_intr_gate(&idt[4], intr_overflow);
  set_intr_gate(&idt[5], intr_bound_range);
  set_intr_gate(&idt[6], intr_invalid_opcode);
  set_intr_gate(&idt[7], intr_device_not_available);
  set_expt_gate(&idt[8], expt_double_fault);
  set_intr_gate(&idt[9], intr_coprocessor_segment_overrun);
  set_expt_gate(&idt[10], expt_invalid_tss);
  set_expt_gate(&idt[11], expt_segment_not_present);
  set_expt_gate(&idt[12], expt_stack_fault);
  set_expt_gate(&idt[13], expt_general_protection);
  set_expt_gate(&idt[14], expt_page_fault);

  set_intr_gate(&idt[16], intr_x87_floating_point);
  set_expt_gate(&idt[17], expt_alignment_check);
  set_intr_gate(&idt[18], intr_machine_check);
  set_intr_gate(&idt[19], intr_simd_floating_point);
  set_intr_gate(&idt[20], intr_virtualization);

  // set the idt register to point to idt array
  set_idtr(idt, AC_ARRAY_COUNT(idt));
}
