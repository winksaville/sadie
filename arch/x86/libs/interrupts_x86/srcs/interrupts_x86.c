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

#include <ac_inttypes.h>
#include <ac_printf.h>

#include <descriptors_x86.h>
#include <native_x86.h>
#include <reset_x86.h>

/* Interrupt Descriptor Table */
static intr_gate idt[256];

static ac_u32 intr_undefined_counter;
static ac_u32 expt_undefined_counter;
static ac_u32 expt_double_fault_counter;
static ac_u32 expt_invalid_tss_counter;
static ac_u32 expt_segment_not_present_counter;
static ac_u32 expt_stack_fault_counter;
static ac_u32 expt_general_protection_counter;
static ac_u32 expt_page_fault_counter;
static ac_u32 expt_alignment_check_counter;
static ac_u32 intr_invalid_opcode_counter;

INTERRUPT_HANDLER
static void intr_undefined(struct intr_frame *frame) {
  intr_undefined_counter += 1;
  print_intr_frame("intr_undefined", frame);
  ac_printf(" sp: %p\n", get_sp());
  ac_printf(" intr_undefined_counter: %d\n", intr_undefined_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void expt_double_fault(struct intr_frame *frame, ac_uint error_code) {
  expt_double_fault_counter += 1;
  print_intr_frame("expt_double_fault - #DF 8", frame);
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: 0x%x\n", error_code);
  ac_printf(" expt_double_fault_counter: %d\n", expt_double_fault_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void expt_invalid_tss(struct intr_frame *frame, ac_uint error_code) {
  expt_invalid_tss_counter += 1;
  print_intr_frame("expt_invalid_tss - #TS 10", frame);
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: 0x%x\n", error_code);
  ac_printf(" expt_invalid_tss_counter: %d\n", expt_invalid_tss_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void expt_segment_not_present(struct intr_frame *frame, ac_uint error_code) {
  expt_segment_not_present_counter += 1;
  print_intr_frame("expt_segment_not_present - #NP 11", frame);
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: 0x%x\n", error_code);
  ac_printf(" expt_segment_not_present_counter: %d\n", expt_segment_not_present_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void expt_stack_fault(struct intr_frame *frame, ac_uint error_code) {
  expt_stack_fault_counter += 1;
  print_intr_frame("expt_stack_fault - #SS 12", frame);
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: 0x%x\n", error_code);
  ac_printf(" expt_stack_fault_counter: %d\n", expt_stack_fault_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void expt_general_protection(struct intr_frame *frame, ac_uint error_code) {
  expt_general_protection_counter += 1;
  print_intr_frame("expt_general_protection - #GP 13", frame);
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: 0x%x\n", error_code);
  ac_printf(" expt_general_protection_counter: %d\n", expt_general_protection_counter);

  idt_ptr idtp;
  get_idt(&idtp);
  ac_printf(" idt.limit=%d\n", idtp.limit);
  ac_printf(" idt.iig=%p\n", idtp.iig);
  reset_x86();
}

INTERRUPT_HANDLER
static void expt_page_fault(struct intr_frame *frame, ac_uint error_code) {
  expt_page_fault_counter += 1;
  print_intr_frame("expt_page_fault - #PF 14", frame);
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: 0x%x\n", error_code);
  ac_printf(" expt_page_fault_counter: %d\n", expt_page_fault_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void expt_alignment_check(struct intr_frame *frame, ac_uint error_code) {
  expt_alignment_check_counter += 1;
  print_intr_frame("expt_alignment_check - #AC 17", frame);
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: 0x%x\n", error_code);
  ac_printf(" expt_alignment_check_counter: %d\n", expt_alignment_check_counter);
  reset_x86();
}

INTERRUPT_HANDLER
static void intr_invalid_opcode(struct intr_frame *frame) {
  intr_invalid_opcode_counter += 1;
  print_intr_frame("intr_invalid_opcode - #UD 6", frame);
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_invalid_opcode_counter: %d\n", intr_invalid_opcode_counter);
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
  expt_double_fault_counter = 0;
  expt_invalid_tss_counter = 0;
  expt_segment_not_present_counter = 0;
  expt_stack_fault_counter = 0;
  expt_general_protection_counter = 0;
  expt_page_fault_counter = 0;
  expt_alignment_check_counter = 0;
  intr_invalid_opcode_counter = 0;

  // Initialize all of the entires to intr_undefined
  for (ac_u64 intr_num = 0; intr_num < AC_ARRAY_COUNT(idt); intr_num++) {
    set_intr_gate(&idt[intr_num], intr_undefined);
  }
  set_intr_gate(&idt[6], intr_invalid_opcode);

  // Initialize some exceptions
  set_expt_gate(&idt[8],  expt_double_fault);
  set_expt_gate(&idt[10], expt_invalid_tss);
  set_expt_gate(&idt[11], expt_segment_not_present);
  set_expt_gate(&idt[12], expt_stack_fault);
  set_expt_gate(&idt[13], expt_general_protection);
  set_expt_gate(&idt[14], expt_page_fault);
  set_expt_gate(&idt[17], expt_alignment_check);

  // set the idt register to point to idt array
  set_idtr(idt, AC_ARRAY_COUNT(idt));
}
