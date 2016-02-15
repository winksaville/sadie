/*
 * Copyright 2015 Wink Saville
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
#include <native_x86.h>

#include <ac_inttypes.h>
#include <ac_architecture.h>
#include <ac_test.h>
#include <ac_printf.h>

volatile ac_u32 intr_79_counter;

__attribute__ ((__interrupt__))
static void intr_79(struct intr_frame *frame) {
  (void)frame;
  intr_79_counter += 1;
  print_intr_frame("intr 79:", frame);
  ac_printf(" ss: %x\n", get_ss());
  ac_printf(" sp: %p\n", get_sp());
  ac_printf(" intr_79_counter=%d\n", intr_79_counter);
}

ac_bool test_interrupts(void) {
  ac_bool error = AC_FALSE;
  idt_ptr idtp1;
  idt_ptr idtp2;

  // Verify we can read the interrupt descriptor table
  get_idt(&idtp1);
  error |= AC_TEST(idtp1.limit != 0);
  error |= AC_TEST(idtp1.iig != 0);
  ac_printf("idtp1.limit=%d iig=%p\n", idtp1.limit, idtp1.iig);

  // Verify we can set it and re-reading has had the same result.
  set_idt(&idtp1);
  get_idt(&idtp2);

  error |= AC_TEST(idtp1.limit == idtp2.limit);
  error |= AC_TEST(idtp1.iig == idtp2.iig);
  ac_printf("idtp2.limit=%d iig=%p\n", idtp2.limit, idtp2.iig);


  // Test we can set an interrupt handler and invoke it
  set_intr_handler(79, intr_79);
  print_idt_intr_gate("idt[79]", get_idt_intr_gate(79));

  idt_intr_gate *g = get_idt_intr_gate(79);
  error |= AC_TEST(GET_IDT_INTR_GATE_OFFSET(*g) == (ac_uptr)intr_79);

  intr_79_counter = 0;
  ac_printf("invoke intr(79)\n");
  intr(79);
  ac_printf("done   intr(79)\n");

  error |= AC_TEST(intr_79_counter == 1);

  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  initialize_intr_descriptor_table();

  error |= test_interrupts();

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
