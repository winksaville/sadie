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
#include <keyboard_impl.h>

#include <apic_x86.h>
#include <apic_x86_print.h>
#include <native_x86.h>
#include <interrupts_x86.h>
#include <interrupts_x86_print.h>
#include <ioapic_x86.h>
#include <ioapic_x86_print.h>
#include <io_x86.h>
#include <thread_x86.h>

#include <ac_inttypes.h>
#include <ac_printf.h>
#include <ac_putchar.h>
#include <ac_test.h>

extern ac_bool channel1_enabled;
extern ac_sint channel1_device_type;
extern ac_bool channel2_enabled;
extern ac_sint channel2_device_type;

volatile ac_u64 keyboard_isr_counter;
volatile ac_u8 ch;

__attribute__ ((__interrupt__))
void keyboard_isr(struct intr_frame *frame) {
  //print_intr_frame("keyboard_isr:+", frame);
  __atomic_add_fetch(&keyboard_isr_counter, 1, __ATOMIC_RELEASE);

  ac_u8 data = 0xFF;
  if ((keyboard_rd_status() & 0x1) == 0x1) {
    data = keyboard_rd_data_now();
    __atomic_store_n(&ch, data, __ATOMIC_RELEASE);
  }
  ac_printf("keyboard_isr:- keyboard_isr_counter=%ld ch=0x%x\n",
      __atomic_load_n(&keyboard_isr_counter, __ATOMIC_ACQUIRE), data);
  send_apic_eoi();
}

#define DELAY_REDIR 1000000000ll

static void print_info(void) {
  ac_printf("apic: tpr=0x%x ppr=0x%x esr=0x%x\n",
      get_apic_tpr(), get_apic_ppr(), get_apic_esr());
  apic_irr_print("irr: ");
  apic_isr_print("isr: ");
  apic_tmr_print("tmr: ");
  ioapic_print();
}

/**
 * Test the keyboard
 */
ac_bool test_keyboard(void) {
  ac_printf("test_keyboard:+\n");
  ac_bool error = AC_FALSE;

  ac_printf("redirs before looping\n");
  print_info();
#if defined(pc_x86_64)
#if 1
  ac_u64 isr_counter;
  do {
    isr_counter =  __atomic_load_n(&keyboard_isr_counter, __ATOMIC_ACQUIRE);
    ac_printf("test_keyboard: keyboard_isr_counter=%lu ch=0x%x\n",
        isr_counter, __atomic_load_n(&ch, __ATOMIC_ACQUIRE));
    ac_thread_wait_ns(100000000);
  } while (isr_counter < 100);
#else
  for (ac_uint i = 0; ; i++) {
    ac_u8 ch;
    ac_printf("%d Press a key:\n", i);

    while((keyboard_rd_status() & 0x1) == 0); // Wait until ready
    //ac_thread_wait_ns(DELAY_REDIR); // Wait before printing redir to see if DS=1
    ac_printf("redirs before reading keypressed\n");
    print_info();
    ch = ac_keyboard_rd();
    ac_printf("%d You pressed: %c (0x%x)\n", i, ch, ch);
    //ac_thread_wait_ns(DELAY_REDIR); // one second printing redir to see that DS=0
    print_info();
  }
#endif

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
  ac_uint keyboard_irq;
  if (channel2_enabled) {
    keyboard_irq = 12;
  }
  if (channel1_enabled) {
    keyboard_irq = 1;
  }

  ac_printf("test_keyboard_interrupts: info before programming\n");
  print_info();

  ac_u8 keyboard_intr_vector;
  keyboard_intr_vector = 0x20 + keyboard_irq;
  ioapic_redir redir = ioapic_get_redir(regs, keyboard_irq);
  redir.intr_vec = keyboard_intr_vector;
  redir.delivery_mode = 0; // fixed
  redir.dest_mode = 0;     // Physical (dest_field == Local APIC ID)
  redir.intr_polarity = 0; // Active 0=High 1=Low
  redir.trigger = 1;       // 0=Edge 1=Level sensitive
  redir.intr_mask = 0;     // NOT masked (interrupt is allowed)
  redir.dest_field = get_apic_id(); // For now this CPU's local apic
  ioapic_set_redir(regs, keyboard_irq, redir);

  ac_printf("test_keyboard_interrupts: info after  programming\n");
  print_info();

  // Set interrupt handler and enable interrupts
  set_intr_handler(keyboard_intr_vector, keyboard_isr);
  keyboard_intr_enable();

  ac_printf("test_keyboard_interrupts:-error=%b\n", error);
  return error;
}

int main(void) {
  ac_uint error = AC_FALSE;

  keyboard_isr_counter = 0;

  if (!error) {
    error |= test_keyboard_interrupts();
    error |= test_keyboard();
  }

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
