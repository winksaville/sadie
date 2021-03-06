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

/**
 * Timer 0 and 1 are reserved by the system.
 */

#define NDEBUG

#include <ac_timer.h>

#include <ac_bits.h>
#include <ac_test.h>
#include <ac_debug_printf.h>

#if AC_PLATFORM == VersatilePB
#include <ac_exceptions.h>
#include <ac_interrupts.h>
#include <ac_pl190_vic.h>
#endif

ac_u32 one_shot_counter;

typedef struct {
  ac_u32  timer;
  ac_bool  source;
} irq_param;

/**
 * Indentify and clear source of interrupt.
 * After returning interrupts will be enabled
 * so we use __atomic operations on source.
 */
void one_shot_iacs(ac_uptr param) {
  irq_param* pirq_param = (irq_param*)param;
  ac_u32 timer_ris = ac_timer_rd_ris(pirq_param->timer);
  if ((timer_ris & 0x1) != 0) {
    ac_bool* psource = &pirq_param->source;
    __atomic_store_n(psource, AC_TRUE, __ATOMIC_RELEASE);
    ac_timer_wr_int_clr(pirq_param->timer);
    ac_debug_printf("\n\none_shot: %d cleared\n", pirq_param->timer);
  }
}

/**
 * Handle the one_shot interrupt.
 *
 * NOTE: Interrupts are enabled so __atomic operations are used.
 */
void one_shot_handler(ac_uptr param) {
  irq_param* pirq_param = (irq_param*)param;
  ac_bool ac_true = AC_TRUE;
  ac_bool* psource = &pirq_param->source;

  ac_bool ok = __atomic_compare_exchange_n(psource, &ac_true, AC_FALSE,
      AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
  if (ok) {
    __atomic_add_fetch(&one_shot_counter, 1, __ATOMIC_RELEASE);
    ac_debug_printf("one_shot: %d inc counter\n\n", pirq_param->timer);
  }
}

ac_u32 periodic_counter;

/**
 * Indentify and clear source of interrupt.
 * After returning interrupts will be enabled
 * so we use __atomic operations on source.
 */
void periodic_iacs(ac_uptr param) {
  irq_param* pirq_param = (irq_param*)param;
  ac_u32 timer_ris = ac_timer_rd_ris(pirq_param->timer);
  if ((timer_ris & 0x1) != 0) {
    ac_bool* psource = &pirq_param->source;
    __atomic_store_n(psource, AC_TRUE, __ATOMIC_RELEASE);
    ac_timer_wr_int_clr(pirq_param->timer);
    ac_debug_printf("\n\nperiodic: %d cleared\n", pirq_param->timer);
  }
}

/**
 * Handle the one_shot interrupt.
 *
 * NOTE: Interrupts are enabled so __atomic operations are used.
 */
void periodic_handler(ac_uptr param) {
  irq_param* pirq_param = (irq_param*)param;
  ac_bool ac_true = AC_TRUE;
  ac_bool* psource = &pirq_param->source;

  ac_bool ok = __atomic_compare_exchange_n(psource, &ac_true, AC_FALSE,
      AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
  if (ok) {
    __atomic_add_fetch(&periodic_counter, 1, __ATOMIC_RELEASE);
    ac_debug_printf("periodic: %d inc counter\n\n", pirq_param->timer);
  }
}


int main(void) {
  ac_bool error = AC_FALSE;
#if AC_PLATFORM == Posix

  ac_printf("test_ac_timer: no tests for Posix\n");

#elif AC_PLATFORM == VersatilePB
  ac_u32 result;

  //ac_printf("test_ac_timer: initial routes=0x%x enable=0x%x\n",
  //    ac_interrupts_rd_int_routes(), ac_interrupts_rd_int_enable());

  // Set to route to irq then fiq
  ac_interrupts_int_route_to_irq(PIC_ALL);
  error |= AC_TEST(ac_interrupts_rd_int_routes() == 0);
  ac_interrupts_int_route_to_fiq(PIC_ALL);
  error |= AC_TEST(ac_interrupts_rd_int_routes() == 0xFFFFFFFF);

  // Set interrupts disable then enabled
  ac_interrupts_int_disable(PIC_ALL);
  error |= AC_TEST(ac_interrupts_rd_int_enable() == 0);
  ac_interrupts_int_enable(PIC_ALL);
  error |= AC_TEST(ac_interrupts_rd_int_enable() == 0xFFFFFFFF);

  // Back to route to irq and no interrupts
  ac_interrupts_int_route_to_irq(PIC_ALL);
  error |= AC_TEST(ac_interrupts_rd_int_routes() == 0);
  ac_interrupts_int_disable(PIC_ALL);
  error |= AC_TEST(ac_interrupts_rd_int_enable() == 0);

  // Expect 4 timers
  ac_u32 count = ac_timer_get_count();
  error |= AC_TEST(count == 4);
  ac_printf("test_ac_timer: count=%u\n", count);

  /* Loop through all of the timers printing the registers */
  for (ac_u32 i = 0; i < count; i++) {
    ac_u32 value = ac_timer_rd_value(i);
    ac_u32 load = ac_timer_rd_load(i);
    ac_u32 control = ac_timer_rd_control(i);
    ac_u32 ris = ac_timer_rd_ris(i);
    ac_u32 mis = ac_timer_rd_mis(i);
    ac_u32 bgload = ac_timer_rd_bgload(i);
    ac_printf("test_ac_timer: value=%u\n", value);
    ac_printf("test_ac_timer: load=0x%x\n", load);
    ac_printf("test_ac_timer: control=0x%x\n", control);
    ac_printf("test_ac_timer: ris=0x%x\n", ris);
    ac_printf("test_ac_timer: mis=0x%x\n", mis);
    ac_printf("test_ac_timer: bgload=0x%x\n", bgload);
  }

  ac_u32 cur_value = 0;
  ac_timer_free_running(3);
  for (ac_u32 i = 0; i < 10; i++) {
    cur_value = ac_timer_rd_free_running(3);
    ac_printf("test_ac_timer: new free_running_value=%u\n", cur_value);
  }

  irq_param one_shot_param = {
    .timer = 2,
    .source = AC_FALSE,
  };

  cur_value = 1;
  one_shot_counter = cur_value;
  ac_u32 prev_value = cur_value;
  ac_u32 terminal_value = cur_value + 1;
  ac_printf("test_ac_timer: one_shot_counter is %u\n", cur_value);

  result = ac_exception_irq_register(&one_shot_handler, &one_shot_iacs,
      (ac_uptr)&one_shot_param);
  error |= AC_TEST(result == 0);

  ac_timer_one_shot(one_shot_param.timer, 1000);
  for (ac_u32 i = 0; i < 10000 && cur_value != terminal_value; i++) {
    ac_u32 timer_value = ac_timer_rd_value(0);
    ac_u32 timer_ris = ac_timer_rd_ris(0);
    ac_u32 timer_mis = ac_timer_rd_mis(0);
    ac_u32 timer_control = ac_timer_rd_control(0);
    ac_printf("test_ac_timer: new value=%u timer_ris=0x%x"
        " timer_mis=0x%x timer_control=0x%x\n",
        timer_value, timer_ris, timer_mis, timer_control);

    ac_u32 irq_status = ac_interrupts_rd_irq_status();
    ac_u32 fiq_status = ac_interrupts_rd_fiq_status();
    ac_u32 ris_status = ac_interrupts_rd_ris_status();
    ac_u32 int_routes = ac_interrupts_rd_int_routes();
    ac_u32 int_enable = ac_interrupts_rd_int_enable();
    ac_printf("test_ac_timer: irq_status=0x%x fiq_status=0x%x"
        " ris_status=0x%x\n",
        irq_status, fiq_status, ris_status);
    ac_printf("test_ac_timer: int_routes=0x%x int_enable=0x%x\n",
        int_routes, int_enable);

    cur_value = __atomic_load_n(&one_shot_counter, __ATOMIC_ACQUIRE);
    if (cur_value != prev_value) {
      prev_value = cur_value;
      ac_printf("test_ac_timer: one_shot_counter changed to %u\n",
          cur_value);
    }
  }
  error |= AC_TEST(one_shot_counter == terminal_value);

  irq_param periodic_param = {
    .timer = 3,
    .source = AC_FALSE,
  };

  result = ac_exception_irq_register(&periodic_handler, &periodic_iacs,
      (ac_uptr)&periodic_param);
  error |= AC_TEST(result == 0);

  ac_timer_periodic(periodic_param.timer, 1000000);
  cur_value = 1;
  periodic_counter = cur_value;
  prev_value = cur_value;
  terminal_value = cur_value + 3;
  ac_printf("test_ac_timer: periodic_counter is %u\n", cur_value);
  for (ac_u64 i = 0; i < 1000000000 && cur_value != terminal_value; i++) {
    cur_value = __atomic_load_n(&periodic_counter, __ATOMIC_ACQUIRE);
    if (cur_value != prev_value) {
      prev_value = cur_value;
      ac_printf("test_ac_timer: periodic_counter changed to %u\n", cur_value);
    }
  }
  error |= AC_TEST(cur_value == terminal_value);

#else

ac_printf("test_ac_timer: Unknown Platform\n");

#endif

  if (!error) {
    // Succeeded
    ac_printf("OK\n");
  }

  return error;
}

