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

#include <ac_keyboard_impl.h>

#include <apic_x86.h>
#include <ioapic_x86.h>
#include <ioapic_x86_print.h>
#include <io_x86.h>

#include <ac_inttypes.h>
#include <ac_printf.h>
#include <ac_test.h>

extern ac_bool channel1_enabled;
extern ac_sint channel1_device_type;
extern ac_bool channel2_enabled;
extern ac_sint channel2_device_type;

/**
 * Test the keyboard
 */
ac_bool test_keyboard(void) {
  ac_printf("test_keyboard:+\n");
  ac_bool error = AC_FALSE;

#if defined(pc_x86_64)
  for (ac_uint i = 0; ; i++) {
    ac_u8 ch;
    ac_printf("%d Press a key:\n", i);
    ch = ac_keyboard_rd();
    ac_printf("%d You pressed: %c (0x%x)\n", i, ch, ch);
  }
#endif

  ac_printf("test_keyboard:-error=%b\n", error);
  return error;
}

ac_bool test_keyboard_interrupts(void) {
  ac_bool error = AC_FALSE;
  ac_printf("test_keyboard_interrupts:+\n");

  // Step 1: Get iopic that the keyboard is connected to
  ac_uint ioapic = 0; // Assume zero for now

  // Step 2: Get the ioppic registers
  ioapic_regs* regs = ioapic_get_addr(ioapic);

  // Step 3: Determine which IRQ the keyboard is connected to
  // for the moment we'll assume only one keyboard and give
  // prioritiy to the first one.
  ac_uint keyboard_irq = ioapic_get_redir_max_entry(regs) + 1;
  if (channel2_enabled) {
    keyboard_irq = 12;
  }
  if (channel1_enabled) {
    keyboard_irq = 1;
  }

  // Step 4: Program the redir register
  ioapic_redir redir = ioapic_get_redir(regs, keyboard_irq);
  redir.intr_vec = 32;
  redir.delivery_mode = 0; // fixed
  redir.dest_mode = 0;     // Physical (dest_field == Local APIC ID)
  redir.intr_polarity = 0; // Active high??
  redir.trigger = 1;       // Level sensitive
  redir.intr_mask = 0;     // NOT masked (interrupt is allowed)
  redir.dest_field = get_apic_id(); // For now this CPU's local apic
  ioapic_redir_print("keyboard_irq set ioapic_redir=", redir, "\n");
  ioapic_set_redir(regs, keyboard_irq, redir);

  error |= AC_TEST(redir.raw == ioapic_get_redir(regs, keyboard_irq).raw);

  ac_printf("test_keyboard_interrupts:-error=%b\n", error);
  return error;
}

int main(void) {
  ac_uint error = AC_FALSE;

  if (!error) {
    error |= test_keyboard_interrupts();
    error |= test_keyboard();
  }

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
