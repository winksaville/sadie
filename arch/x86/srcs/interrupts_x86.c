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

#include <ac_inttypes.h>
#include <ac_printf.h>
#include <native_x86.h>
#include <interrupts_x86.h>
#include <descriptors_x86.h>
#include <descriptors_x86_print.h>

/* Interrupt Descriptor Table */
static idt_intr_gate idt[256];

static ac_u32 intr_undefined_counter;
static ac_u32 expt_undefined_counter;
static ac_u32 intr_invalid_opcode_counter;

__attribute__ ((__interrupt__))
static void intr_undefined(struct intr_frame *frame) {
  intr_undefined_counter += 1;
  print_intr_frame("intr_undefined", frame);
  ac_printf(" sp: %p\n", get_sp());
  ac_printf(" intr_undefined_counter: %d\n", intr_undefined_counter);
}

__attribute__ ((__interrupt__))
static void expt_08_undefined(struct intr_frame *frame, ac_uint error_code) {
  expt_undefined_counter += 1;
  print_intr_frame("expt_08_undefined", frame);
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: %u\n", error_code);
  ac_printf(" expt_undefined_counter: %d\n", expt_undefined_counter);
}

__attribute__ ((__interrupt__))
static void expt_10_undefined(struct intr_frame *frame, ac_uint error_code) {
  expt_undefined_counter += 1;
  print_intr_frame("expt_10_undefined", frame);
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: %u\n", error_code);
  ac_printf(" expt_undefined_counter: %d\n", expt_undefined_counter);
}

__attribute__ ((__interrupt__))
static void expt_11_undefined(struct intr_frame *frame, ac_uint error_code) {
  expt_undefined_counter += 1;
  print_intr_frame("expt_11_undefined", frame);
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: %u\n", error_code);
  ac_printf(" expt_undefined_counter: %d\n", expt_undefined_counter);
}

__attribute__ ((__interrupt__))
static void expt_12_undefined(struct intr_frame *frame, ac_uint error_code) {
  expt_undefined_counter += 1;
  print_intr_frame("expt_12_undefined", frame);
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: %u\n", error_code);
  ac_printf(" expt_undefined_counter: %d\n", expt_undefined_counter);
}

__attribute__ ((__interrupt__))
static void expt_13_undefined(struct intr_frame *frame, ac_uint error_code) {
  expt_undefined_counter += 1;
  print_intr_frame("expt_13_undefined", frame);
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: %u\n", error_code);
  ac_printf(" expt_undefined_counter: %d\n", expt_undefined_counter);

  descriptor_ptr dp;
  get_idt(&dp);
  ac_printf(" idt.limit=%d\n", dp.limit);
  ac_printf(" idt.address=%p\n", dp.address);
}

__attribute__ ((__interrupt__))
static void expt_14_undefined(struct intr_frame *frame, ac_uint error_code) {
  expt_undefined_counter += 1;
  print_intr_frame("expt_14_undefined", frame);
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: %u\n", error_code);
  ac_printf(" expt_undefined_counter: %d\n", expt_undefined_counter);
}

__attribute__ ((__interrupt__))
static void expt_17_undefined(struct intr_frame *frame, ac_uint error_code) {
  expt_undefined_counter += 1;
  print_intr_frame("expt_17_undefined", frame);
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" error_code: %u\n", error_code);
  ac_printf(" expt_undefined_counter: %d\n", expt_undefined_counter);
}

__attribute__ ((__interrupt__))
static void intr_invalid_opcode(struct intr_frame *frame) {
  intr_invalid_opcode_counter += 1;
  print_intr_frame("intr_invalid_opcode", frame);
  ac_printf(" rsp: %p\n", get_sp());
  ac_printf(" intr_invalid_opcode_counter: %d\n", intr_invalid_opcode_counter);
}

static void set_intr_gate(idt_intr_gate* gate, intr_handler* ih) {
  static idt_intr_gate g = IDT_INTR_GATE_INITIALIZER;
  *gate = g;

  gate->offset_lo = IDT_INTR_GATE_OFFSET_LO(ih);
  gate->offset_hi = IDT_INTR_GATE_OFFSET_HI(ih);
  gate->segment = 8; // Code Segment TI=0, RPL=0
                     // FIXME: How to know where the Code Segement is
#ifdef CPU_X86_64
  gate->ist = 0; // Modified legacy stack switching mode
#endif
  gate->type = DT_64_INTR_GATE;
  gate->dpl = 0;
  gate->p = 1;
}

static void set_expt_gate(idt_intr_gate* gate, expt_handler* eh) {
  static idt_intr_gate g = IDT_INTR_GATE_INITIALIZER;
  *gate = g;

  gate->offset_lo = IDT_INTR_GATE_OFFSET_LO(eh);
  gate->offset_hi = IDT_INTR_GATE_OFFSET_HI(eh);
  gate->segment = 8; // Code Segment TI=0, RPL=0
                     // FIXME: How to know where the Code Segement is
#ifdef CPU_X86_64
  gate->ist = 0; // Modified legacy stack switching mode
#endif
  gate->type = DT_64_INTR_GATE;
  gate->dpl = 0;
  gate->p = 1;
}

static void set_idtr(idt_intr_gate idt[], ac_u32 count) {
  descriptor_ptr dp;
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

idt_intr_gate* get_idt_intr_gate(ac_u32 intr_num) {
  return &idt[intr_num];
}

void initialize_intr_descriptor_table(void) {
  intr_undefined_counter = 0;
  expt_undefined_counter = 0;
  intr_invalid_opcode_counter = 0;

  // Initialize all of the entires to intr_undefined
  for (ac_u64 intr_num = 0; intr_num < AC_ARRAY_COUNT(idt); intr_num++) {
    set_intr_gate(&idt[intr_num], intr_undefined);
  }
  set_intr_gate(&idt[6], intr_invalid_opcode);

  // Initialize some exceptions
  set_expt_gate(&idt[8], expt_08_undefined);
  set_expt_gate(&idt[10], expt_10_undefined);
  set_expt_gate(&idt[11], expt_11_undefined);
  set_expt_gate(&idt[12], expt_12_undefined);
  set_expt_gate(&idt[13], expt_13_undefined);
  set_expt_gate(&idt[14], expt_14_undefined);
  set_expt_gate(&idt[17], expt_17_undefined);

  // set the idt register to point to idt array
  set_idtr(idt, AC_ARRAY_COUNT(idt));
}
