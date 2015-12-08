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
 * The timers on the VersatilePB are SP804 with the documenation
 * [here](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0271d/index.html)
 */
#include <ac_timer.h>

#include <ac_interrupts.h>
#include <ac_pl190_vic.h>

#include <ac_inttypes.h>

#define TIMER_COUNT 4

// The base memory address for the VersatilePB is
// [here](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0425f/CACDCGDI.html)
#define TIMER0_1_BASE_REG  ((ac_u8*)0x101E2000)
#define TIMER2_3_BASE_REG  ((ac_u8*)0x101E3000)
static ac_u8* timer_base[TIMER_COUNT] = {
  TIMER0_1_BASE_REG + 0x00,
  TIMER0_1_BASE_REG + 0x20,
  TIMER2_3_BASE_REG + 0x00,
  TIMER2_3_BASE_REG + 0x20,
};

// Register offsets from timer_base[0]
// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0271d/index.html
#define TIMER_LOAD    0x00
#define TIMER_VALUE   0x04
#define TIMER_CONTROL 0x08
#define TIMER_INT_CLR 0x0C
#define TIMER_RIS     0x10
#define TIMER_MIS     0x14
#define TIMER_BGLOAD  0x18

// Timer Control Register fields
// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0271d/Babgabfg.html
#define TIMER_ENABLED 0x80
#define TIMER_DISABLED 0x00

#define TIMER_MODE_PERIODIC 0x40
#define TIMER_MODE_FREE_RUNNING 0x00

#define TIMER_INT_ENABLED 0x40
#define TIMER_INT_DISABLED 0x00

#define TIMER_PRE_SCALE_DIV_01 0x00
#define TIMER_PRE_SCALE_DiV_16 0x01
#define TIMER_PRE_SCALE_DiV_256 0x11

#define TIMER_32BIT 0x02
#define TIMER_16BIT 0x00

#define TIMER_ONE_SHOT 0x01
#define TIMER_WRAPPING 0x00



#define READ_REG_U32(timer, reg) \
  (*(volatile ac_u32*)(timer_base[timer] + reg))

#define WRITE_REG_U32(timer, reg, value) \
  (*(volatile ac_u32*)(timer_base[timer] + reg)) = value

#define READ_REG_U8(timer, reg) \
  (*(volatile ac_u8*)(timer_base[timer] + reg))

#define WRITE_REG_U8(timer, reg, value) \
  (*(volatile ac_u8*)(timer_base[timer] + reg)) = value

/**
 * Return the number of timers
 */
ac_u32 ac_timer_get_count() {
  return TIMER_COUNT;
}

/**
 * Return the current timer load value
 */
ac_u32 ac_timer_rd_load(ac_u32 timer) {
  return READ_REG_U32(timer, TIMER_LOAD);
}

/**
 * Write timer load value
 */
void ac_timer_wr_load(ac_u32 timer, ac_u32 value) {
  WRITE_REG_U32(timer, TIMER_LOAD, value);
}

/**
 * Return the current timer value
 */
ac_u32 ac_timer_rd_value(ac_u32 timer) {
  return READ_REG_U32(timer, TIMER_VALUE);
}

/**
 * Return the current timer control value
 */
ac_u32 ac_timer_rd_control(ac_u32 timer) {
  return (ac_u32)(READ_REG_U8(timer, TIMER_CONTROL));
}

/**
 * Write timer control value
 */
void ac_timer_wr_control(ac_u32 timer, ac_u32 value) {
  WRITE_REG_U8(timer, TIMER_CONTROL, (ac_u8)(value));
}

/**
 * Write timer interrupt clear register
 */
void ac_timer_wr_int_clr(ac_u32 timer) {
  WRITE_REG_U32(timer, TIMER_INT_CLR, 0);
}


/**
 * Return the current raw interrupt status
 */
ac_u32 ac_timer_rd_ris(ac_u32 timer) {
  return READ_REG_U32(timer, TIMER_RIS);
}

/**
 * Return the current masked interrupt status
 */
ac_u32 ac_timer_rd_mis(ac_u32 timer) {
  return READ_REG_U32(timer, TIMER_MIS);
}

/**
 * Return the current timer bgload value
 */
ac_u32 ac_timer_rd_bgload(ac_u32 timer) {
  return READ_REG_U32(timer, TIMER_BGLOAD);
}

/**
 * Write timer bgload value which will update the
 * timer load value the next current value is zero
 */
void ac_timer_wr_bgload(ac_u32 timer, ac_u32 value) {
  WRITE_REG_U32(timer, TIMER_BGLOAD, value);
}

/**
 * Start free running counter
 *
 * Return 0 if successful, !0 if error
 */
void ac_timer_free_running(ac_u32 timer) {
#define FREE_RUNNING TIMER_MODE_FREE_RUNNING \
      | TIMER_INT_DISABLED | TIMER_PRE_SCALE_DIV_01 | TIMER_32BIT \
      | TIMER_WRAPPING

  ac_timer_wr_control(timer, TIMER_DISABLED | FREE_RUNNING);
  ac_timer_wr_load(timer, 0);
  ac_timer_wr_control(timer, TIMER_ENABLED | FREE_RUNNING);
}

/**
 * Cause the IRQ to fire periodically
 *
 * Return 0 if successful, !0 if error
 */
ac_u32 ac_timer_rd_free_running(ac_u32 timer) {
  return -ac_timer_rd_value(timer);
}

/**
 * Cause the IRQ to fire periodically
 *
 * Return 0 if successful, !0 if error
 */
ac_u32 ac_timer_periodic(ac_u32 timer, ac_u32 period_in_micro) {
  #define PERIODIC TIMER_MODE_PERIODIC \
      | TIMER_PRE_SCALE_DIV_01 | TIMER_32BIT | TIMER_WRAPPING

  ac_timer_wr_control(timer, TIMER_DISABLED | TIMER_INT_DISABLED | PERIODIC);
  ac_u32 pic_timer = ((timer == 0) || (timer == 1)) ?
      PIC_TIMERS0_1 : PIC_TIMERS2_3;
  ac_interrupts_int_route_to_irq(pic_timer);
  ac_interrupts_int_enable(pic_timer);
  ac_timer_wr_load(timer, period_in_micro);
  ac_timer_wr_control(timer, TIMER_ENABLED | TIMER_INT_ENABLED | PERIODIC);
  return 0;
}

/**
 * Cause the IRQ to fire once
 *
 * Return 0 if successful, !0 if error
 */
ac_u32 ac_timer_one_shot(ac_u32 timer, ac_u32 time_in_micro) {
  #define ONE_SHOT TIMER_MODE_FREE_RUNNING \
      | TIMER_PRE_SCALE_DIV_01 | TIMER_32BIT | TIMER_ONE_SHOT

  ac_timer_wr_control(timer, TIMER_DISABLED | TIMER_INT_DISABLED | ONE_SHOT);
  ac_u32 pic_timer = ((timer == 0) || (timer == 1)) ? PIC_TIMERS0_1 : PIC_TIMERS2_3;
  ac_interrupts_int_route_to_irq(pic_timer);
  ac_interrupts_int_enable(pic_timer);
  ac_timer_wr_load(timer, time_in_micro);
  ac_timer_wr_control(timer, TIMER_ENABLED | TIMER_INT_ENABLED | ONE_SHOT);
  return 1;
}

