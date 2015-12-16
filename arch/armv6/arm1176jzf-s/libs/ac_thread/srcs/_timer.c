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

#define NDEBUG

#include <_timer.h>

#include <ac_timer.h>
#include <ac_exceptions.h>
#include <ac_debug_printf.h>

// Timer 0 reserved for our periodic timer
#define AC_RUNTIME_PERIODIC_TIMER 0

typedef struct {
  ac_u32  timer;
  ac_bool  source;
} irq_param;

irq_param periodic_param = {
  .timer = AC_RUNTIME_PERIODIC_TIMER,
  .source = AC_FALSE,
};


/**
 * Indentify and clear source of interrupt.
 * After returning interrupts will be enabled
 * so we use __atomic operations on source.
 */
static void periodic_iacs(ac_uptr param) {
  irq_param* pirq_param = (irq_param*)param;
  ac_u32 timer_ris = ac_timer_rd_ris(pirq_param->timer);
  if ((timer_ris & 0x1) != 0) {
    // Set pirq_param->source to AC_TRUE to indicate this did fire
    ac_bool* psource = &pirq_param->source;
    __atomic_store_n(psource, AC_TRUE, __ATOMIC_RELEASE);
    ac_timer_wr_int_clr(pirq_param->timer);
    ac_debug_printf("periodic: %d cleared\n", pirq_param->timer);
  }
}

/**
 * Handle the periodic interrupt.
 *
 * NOTE: Interrupts are enabled so __atomic operations are used.
 */
static void periodic_handler(ac_uptr param) {
  irq_param* pirq_param = (irq_param*)param;

  // Test if pirq_param->source is AC_TRUE which means this did fire
  ac_bool ac_true = AC_TRUE;
  ac_bool* psource = &pirq_param->source;
  ac_bool ok = __atomic_compare_exchange_n(psource, &ac_true, AC_FALSE,
      AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
  if (ok) {
    // Yes, this interupt occurred.
    ac_debug_printf("periodic: %d handled\n", pirq_param->timer);
  }
}

/**
 * Start periodic timer.
 *
 * @param period_in_micro_secs
 */
void ac_start_periodic_timer(ac_u32 period_in_micro_secs) {
  // Register our handler and iacs
  ac_exception_irq_register(&periodic_handler, &periodic_iacs,
      (ac_uptr)&periodic_param);

  // Start the timer
  ac_timer_periodic(periodic_param.timer, period_in_micro_secs);
}
