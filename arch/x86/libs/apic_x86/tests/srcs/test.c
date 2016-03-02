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

#include <apic_x86.h>

#include <cpuid_x86.h>
#include <interrupts_x86.h>
#include <interrupts_x86_print.h>
#include <msr_x86.h>
#include <reset_x86.h>
#include <print_msr.h>

#include <ac_bits.h>
#include <ac_inttypes.h>
#include <ac_memset.h>
#include <ac_printf.h>
#include <ac_putchar.h>
#include <ac_test.h>

ac_bool test_apic() {
  ac_bool error = AC_FALSE;

  ac_u64 msr_apic_base = get_msr(MSR_APIC_BASE);
  error |= AC_TEST(msr_apic_base != 0);

  print_msr(MSR_APIC_BASE, msr_apic_base);

  ac_u64 phy_addr = apic_get_physical_addr();
  ac_printf(" phy_addr=0x%llx\n", phy_addr);
  error |= AC_TEST(phy_addr != 0);

  void* lin_addr = apic_get_linear_addr();
  ac_printf(" lin_addr=0x%llx\n", lin_addr);
  error |= AC_TEST(lin_addr != 0);

  ac_u32 local_id = apic_get_id();
  ac_printf(" local_id=0x%x\n", local_id);

  return error;
}

ac_bool test_apic_version() {
  ac_bool error = AC_FALSE;

  ac_printf("test_apic_version\n");
  ac_u64 msr_apic_base = get_msr(MSR_APIC_BASE);
  print_msr(MSR_APIC_BASE, msr_apic_base);
  ac_u32* ver_reg = (ac_u32*)((ac_uptr)apic_get_linear_addr() + 0x30);
  ac_printf("ver_reg=0x%p\n", ver_reg);
  ac_printf("*ver_reg=0x%x\n", *ver_reg);

#ifdef CPU_X86_64
  // Not sure if this is always true will probably
  // need to be altered in the future.
  ac_u32 ver_reg_values[32];
  ac_memset(ver_reg_values, 0, sizeof(ver_reg_values));
  ver_reg_values[0x14] = 0x1050014;
  ver_reg_values[0x15] = 0x1060015;

  error |= AC_TEST(*ver_reg == ver_reg_values[*ver_reg & 0xFF]);

#else
  // Not sure what X86_32 should be
#endif

  return error;
}

volatile ac_u64 apic_timer_isr_counter;
volatile ac_u64 apic_timer_df_counter;
volatile ac_u64 apic_timer_loops;
volatile ac_u32 apic_timer_initial_count;

__attribute__ ((__interrupt__))
//static void apic_timer_df(struct intr_frame *frame, ac_uint error_code) {
static void apic_timer_df(struct intr_frame *frame) {
  (void)frame;
  union apic_timer_lvt_fields_u lvtu = { .fields = apic_timer_get_lvt() };
  apic_timer_df_counter += 1;

  // print the frame
  print_intr_frame("apic_timer_df", frame);
  ac_printf(" apic_timer_df_counter =%ld\n", apic_timer_df_counter);
  ac_printf(" apic_timer_isr_counter=%ld\n", apic_timer_isr_counter);
  ac_printf(" apic_timer_loops=%ld\n", apic_timer_loops);
  ac_printf(" lvtu.raw=0x%x\n", lvtu.raw);
  ac_printf(" lvtu.fields.vector=%d\n", lvtu.fields.vector);
  ac_printf(" lvtu.fields.status=%d\n", lvtu.fields.status);
  ac_printf(" lvtu.fields.disable=%d\n", lvtu.fields.disable);
  ac_printf(" lvtu.fields.mode=%d\n", lvtu.fields.mode);
  reset_x86();
}

__attribute__ ((__interrupt__))
static void apic_timer_isr(struct intr_frame *frame) {
  (void)frame;
  // Reset the count so it might go again
  union apic_timer_lvt_fields_u lvtu = { .fields = apic_timer_get_lvt() };
  apic_timer_isr_counter += 1;
  apic_timer_set_initial_count(apic_timer_initial_count);

  // print the frame
  print_intr_frame("apic_timer_isr", frame);
  ac_printf(" apic_timer_df_counter =%ld\n", apic_timer_df_counter);
  ac_printf(" apic_timer_isr_counter=%ld\n", apic_timer_isr_counter);
  ac_printf(" apic_timer_loops=%ld\n", apic_timer_loops);
  ac_printf(" lvtu.raw=0x%x\n", lvtu.raw);
  ac_printf(" lvtu.fields.vector=%d\n", lvtu.fields.vector);
  ac_printf(" lvtu.fields.status=%d\n", lvtu.fields.status);
  ac_printf(" lvtu.fields.disable=%d\n", lvtu.fields.disable);
  ac_printf(" lvtu.fields.mode=%d\n", lvtu.fields.mode);
}

ac_bool test_apic_timer() {
  ac_bool error = AC_FALSE;

  ac_printf("test_apic_timer\n");
  union apic_timer_lvt_fields_u lvtu = { .fields = apic_timer_get_lvt() };
  union apic_timer_lvt_fields_u lvtu2;
  union apic_timer_lvt_fields_u lvtu3;

  ac_printf("test_apic_timer: lvtu.raw=0x%x\n", lvtu.raw);
  ac_printf("test_apic_timer: lvtu.fields.vector=%d\n", lvtu.fields.vector);
  ac_printf("test_apic_timer: lvtu.fields.status=%d\n", lvtu.fields.status);
  ac_printf("test_apic_timer: lvtu.fields.disable=%d\n", lvtu.fields.disable);
  ac_printf("test_apic_timer: lvtu.fields.mode=%d\n", lvtu.fields.mode);
  error |= AC_TEST(lvtu.raw == 0x10000);
  error |= AC_TEST(lvtu.fields.vector == 0);
  error |= AC_TEST(lvtu.fields.disable == AC_TRUE);
  error |= AC_TEST(lvtu.fields.mode == 0);

  apic_timer_isr_counter = 0;
  set_intr_handler(80, apic_timer_isr);

  apic_timer_df_counter = 0;
  //set_expt_handler(8, apic_timer_df);
  set_intr_handler(8, apic_timer_df);

  lvtu.fields.vector = 80; // interrupt vector
  lvtu.fields.disable = AC_FALSE; // interrupt enabled
  lvtu.fields.mode = 0;     // one shot
  apic_timer_set_lvt(lvtu.fields);

  lvtu2.fields = apic_timer_get_lvt();
  ac_printf("test_apic_timer: lvtu2.raw=0x%x\n", lvtu2.raw);
  ac_printf("test_apic_timer: lvtu2.fields.vector=%d\n", lvtu2.fields.vector);
  ac_printf("test_apic_timer: lvtu2fields.status=%d\n", lvtu2.fields.status);
  ac_printf("test_apic_timer: lvtu2.fields.disable=%d\n", lvtu2.fields.disable);
  ac_printf("test_apic_timer: lvtu2.fields.mode=%d\n", lvtu2.fields.mode);
  error |= AC_TEST(lvtu2.fields.vector == 80);
  error |= AC_TEST(lvtu2.fields.disable == AC_FALSE);
  error |= AC_TEST(lvtu2.fields.mode == 0);

  // counter starts at 0
  error |= AC_TEST(apic_timer_isr_counter == 0);

  // Enable interrupts start the timer and wait until we get a timer interrupt
  // or we have waited long enough without one.
  apic_timer_loops = 0;
  apic_timer_initial_count = 1;
  ac_printf("test_apic_timer: sti apic_timer_initial_count=%d\n", apic_timer_initial_count);
  apic_timer_set_divide_config(7); // DIVIDE by 1
  apic_timer_set_initial_count(apic_timer_initial_count);
  sti();
  for (ac_u64 i = 0; (i < 1000000000) && (apic_timer_isr_counter < 1); i++) {
    apic_timer_loops += 1;
  }
  cli();
  ac_printf("test_apic_timer: cli apic_timer_initial_count=%d\n", apic_timer_initial_count);
  ac_printf("  apic_timer_df_counter =%ld\n", apic_timer_df_counter);
  ac_printf("  apic_timer_isr_counter=%ld\n", apic_timer_isr_counter);
  ac_printf("  apic_timer_loops=%ld\n", apic_timer_loops);
  lvtu3.fields = apic_timer_get_lvt();
  ac_printf("  lvtu3.raw=0x%x\n", lvtu3.raw);
  ac_printf("  lvtu3.fields.vector=%d\n", lvtu3.fields.vector);
  ac_printf("  lvtu3fields.status=%d\n", lvtu3.fields.status);
  ac_printf("  lvtu3.fields.disable=%d\n", lvtu3.fields.disable);
  ac_printf("  lvtu3.fields.mode=%d\n", lvtu3.fields.mode);

  // Expect that the counter fired one or more times.
  // FIXME: When we get things working this should be a precise number.
  error |= AC_TEST(apic_timer_isr_counter >= 1);

  return error;
}


int main(void) {
  ac_bool error = AC_FALSE;

  // Initialize interrupt descriptor table since its not done by default, yet.
  initialize_intr_descriptor_table();

  initialize_apic();

  error |= test_apic();
  error |= test_apic_version();
  error |= test_apic_timer();

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
