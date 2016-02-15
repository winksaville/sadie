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

#include "ac_inttypes.h"
#include "ac_printf.h"
#include "interrupts_x86.h"

void print_intr_frame(char* str, intr_frame* frame) {
  ac_printf("%s:\n", str);
  ac_printf(" frame->ip: %p\n", frame->ip);
  ac_printf(" frame->cs: %p\n", frame->cs);
  ac_printf(" frame->flags: %p\n", frame->flags);
  ac_printf(" frame->sp: %p\n", frame->sp);
  ac_printf(" frame->ss: %x\n", frame->ss);
}

void print_idt_intr_gate(char *str, idt_intr_gate* gate) {
  ac_printf("%s:\n", str);
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
  ac_printf(" gate->offset: %p\n", GET_IDT_INTR_GATE_OFFSET(*gate));
}
