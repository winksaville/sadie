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

#ifndef SADIE_PLATFORM_VERSATILEPB_LIBS_AC_TIMER_INCS_AC_TIMER_H
#define SADIE_PLATFORM_VERSATILEPB_LIBS_AC_TIMER_INCS_AC_TIMER_H

#include <ac_inttypes.h>

/**
 * Return the number of timers
 */
ac_u32 ac_timer_get_count();

/**
 * Start free running counter
 *
 * Return 0 if successful, !0 if error
 */
void ac_timer_free_running(ac_u32 timer);

/**
 * Cause the IRQ to fire periodically
 *
 * Return 0 if successful, !0 if error
 */
ac_u32 ac_timer_rd_free_running(ac_u32 timer);

/**
 * Return the current timer value
 */
ac_u32 ac_timer_rd_value(ac_u32 timer);

/**
 * Return the current timer load value
 */
ac_u32 ac_timer_rd_load(ac_u32 timer);

/**
 * Write timer load value
 */
void ac_timer_wr_load(ac_u32 timer, ac_u32 value);

/**
 * Return the current timer value
 */
ac_u32 ac_timer_rd_value(ac_u32 timer);

/**
 * Return the current timer control value
 */
ac_u32 ac_timer_rd_control(ac_u32 timer);

/**
 * Write timer control value
 */
void ac_timer_wr_control(ac_u32 timer, ac_u32 value);

/**
 * Write timer interrupt clear register
 */
void ac_timer_wr_int_clr(ac_u32 timer);

/**
 * Return the current raw interrupt status
 */
ac_u32 ac_timer_rd_ris(ac_u32 timer);

/**
 * Return the current masked interrupt status
 */
ac_u32 ac_timer_rd_mis(ac_u32 timer);

/**
 * Return the current timer bgload value
 */
ac_u32 ac_timer_rd_bgload(ac_u32 timer);

/**
 * Write timer bgload value which will update the
 * timer load value the next current value is zero
 */
void ac_timer_wr_bgload(ac_u32 timer, ac_u32 value);

/**
 * Create an periodic timer
 */
ac_u32 ac_timer_periodic(ac_u32 timer, ac_u32 period_in_nano);

/**
 * Create a one shot timer
 */
ac_u32 ac_timer_one_shot(ac_u32 timer, ac_u32 period_in_nano);

#endif
