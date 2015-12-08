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

#include <ac_timer.h>

#include <ac_test.h>
#if defined(VersatilePB)
#include <ac_interrupts.h>
#include <ac_pl190_vic.h>
#endif

ac_bool test_ac_timer() {
  ac_bool error = AC_FALSE;
#if defined(Posix)

  ac_printf("test_ac_timer: no tests for Posix\n");

#elif defined(VersatilePB)

  //ac_printf("test_ac_timer: initial routes=0x%x enable=0x%x\n",
  //    ac_interrupts_rd_int_routes(), ac_interrupts_rd_int_enable());

  ac_u32 int_routes;
  ac_u32 int_enabled;

  int_routes = ac_interrupts_int_route_to_irq(PIC_ALL);
  ac_interrupts_int_disable(PIC_ALL);
  int_enabled = ac_interrupts_rd_int_enable();
  error |= AC_TEST(int_routes == ac_interrupts_rd_int_routes());
  error |= AC_TEST(int_enabled == ac_interrupts_rd_int_enable());
  ac_printf("test_ac_timer: after route to IRQ/disable all 1 routes=0x%x enable=0x%x\n",
      ac_interrupts_rd_int_routes(), ac_interrupts_rd_int_enable());
  ac_printf("test_ac_timer: after route to IRQ/disable all 2 routes=0x%x enable=0x%x\n",
      int_routes, int_enabled);

  int_routes = ac_interrupts_int_route_to_fiq(PIC_ALL);
  ac_interrupts_int_enable(PIC_ALL);
  int_enabled = ac_interrupts_rd_int_enable();
  error |= AC_TEST(int_routes == ac_interrupts_rd_int_routes());
  error |= AC_TEST(int_enabled == ac_interrupts_rd_int_enable());
  ac_printf("test_ac_timer: after route to FIQ/enable all 1 routes=0x%x enable=0x%x\n",
      ac_interrupts_rd_int_routes(), ac_interrupts_rd_int_enable());
  ac_printf("test_ac_timer: after route to FiQ/enable all 2 routes=0x%x enable=0x%x\n",
      int_routes, int_enabled);

  int_routes = ac_interrupts_int_route_to_irq(PIC_ALL);
  ac_interrupts_int_enable(PIC_ALL);
  int_enabled = ac_interrupts_rd_int_enable();
  error |= AC_TEST(int_routes == ac_interrupts_rd_int_routes());
  error |= AC_TEST(int_enabled == ac_interrupts_rd_int_enable());
  ac_printf("test_ac_timer: after route to IRQ/enable 1 routes=0x%x enable=0x%x\n",
      ac_interrupts_rd_int_routes(), ac_interrupts_rd_int_enable());
  ac_printf("test_ac_timer: after route to IRQ/enable 2 routes=0x%x enable=0x%x\n",
      int_routes, int_enabled);

  int_routes = ac_interrupts_int_route_to_irq(PIC_ALL);
  ac_interrupts_int_disable(PIC_ALL);
  int_enabled = ac_interrupts_rd_int_enable();
  error |= AC_TEST(int_routes == ac_interrupts_rd_int_routes());
  error |= AC_TEST(int_enabled == ac_interrupts_rd_int_enable());
  ac_printf("test_ac_timer: after route to IRQ/disable 1 routes=0x%x enable=0x%x\n",
      ac_interrupts_rd_int_routes(), ac_interrupts_rd_int_enable());
  ac_printf("test_ac_timer: after route to IRQ/disable 2 routes=0x%x enable=0x%x\n",
      int_routes, int_enabled);

  int_routes = ac_interrupts_int_route_to_irq(PIC_TIMERS0_1);
  ac_interrupts_int_enable(PIC_TIMERS0_1);
  int_enabled = ac_interrupts_rd_int_enable();
  error |= AC_TEST(int_routes == ac_interrupts_rd_int_routes());
  error |= AC_TEST(int_enabled == ac_interrupts_rd_int_enable());
  ac_printf("test_ac_timer: after routing/enabling PIC_TIMERS0_1 routes=0x%x enable=0x%x\n",
      ac_interrupts_rd_int_routes(), ac_interrupts_rd_int_enable());
  ac_printf("test_ac_timer: after routing/enabling PIC_TIMERS0_1 routes=0x%x enable=0x%x\n",
     int_routes, int_enabled);

  ac_u32 count = ac_timer_get_count();
  ac_printf("test_ac_timer: count=%u\n", count);

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
  ac_timer_free_running(1);
  for (ac_u32 i = 0; i < 20; i++) {
    cur_value = ac_timer_rd_free_running(1);
    ac_printf("test_ac_timer: new free_running_value=%u\n",
          cur_value);
  }

  cur_value = 0;
  ac_timer_one_shot(2, 1000);
  for (ac_u32 i = 0; i < 20; i++) {
    cur_value = ac_timer_rd_value(2);
    ac_u32 timer_ris = ac_timer_rd_ris(2);
    ac_u32 timer_mis = ac_timer_rd_mis(2);
    ac_u32 irq_status = ac_interrupts_rd_irq_status();
    ac_u32 fiq_status = ac_interrupts_rd_fiq_status();
    ac_u32 ris_status = ac_interrupts_rd_ris_status();
    ac_u32 int_routes = ac_interrupts_rd_int_routes();
    ac_u32 int_enable = ac_interrupts_rd_int_enable();
    ac_printf("test_ac_timer: new cur_value=%u timer_ris=0x%x timer_mis=0x%x\n",
        cur_value, timer_ris, timer_mis);
    ac_printf("test_ac_timer: irq_status=0x%x fiq_status=0x%x ris_status=0x%x\n",
        irq_status, fiq_status, ris_status);
    ac_printf("test_ac_timer: int_routes=0x%x int_enable=0x%x\n",
        int_routes, int_enable);
  }

  //cur_value = 0;
  //ac_timer_periodic(0, 1000);
  //for (ac_u32 i = 0; i < 10000; i++) {
  //  cur_value = ac_timer_rd_value(0);
  //  //ac_printf("test_ac_timer: new cur_value=%u\n",
  //  //    cur_value);
  //}

#else
  ac_printf("test_ac_timer: Unknown Platform\n");
#endif

  return error;
}

int main(void) {
  if (test_ac_timer()) {
      // Failed
      ac_printf("ERR\n");
      return 1;
  } else {
      // Succeeded
      ac_printf("OK\n");
      return 0;
  }
}

