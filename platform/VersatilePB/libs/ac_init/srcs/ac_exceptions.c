/*
 * copyright 2015 wink saville
 *
 * licensed under the apache license, version 2.0 (the "license");
 * you may not use this file except in compliance with the license.
 * you may obtain a copy of the license at
 *
 *     http://www.apache.org/licenses/license-2.0
 *
 * unless required by applicable law or agreed to in writing, software
 * distributed under the license is distributed on an "as is" basis,
 * without warranties or conditions of any kind, either express or implied.
 * see the license for the specific language governing permissions and
 * limitations under the license.
 */

#include <ac_exceptions.h>

#include <ac_inttypes.h>
#include <ac_putchar.h>
#include <ac_timer.h>

void ac_exception_reset_handler(void) __attribute__ ((interrupt ("UNDEF")));
void ac_exception_undef_handler(void) __attribute__ ((interrupt ("UNDEF")));
void ac_exception_svc_handler(void) __attribute__ ((interrupt ("SWI")));
void ac_exception_prefetch_abort_handler(void) __attribute__ ((interrupt ("ABORT")));
void ac_exception_data_abort_handler(void) __attribute__ ((interrupt ("ABORT")));
void ac_exception_irq_handler(void) __attribute__ ((interrupt ("IRQ")));
void ac_exception_fiq_handler(void) __attribute__ ((interrupt ("FIQ")));

typedef struct {
  ac_bool available;
  int_handler handler;
  void* param;
} irq_handler_obj;

#define MAX_HANDLERS 8
static irq_handler_obj irq_handlers[MAX_HANDLERS];

/**
 * Register a irq handler and its parameter.
 *
 * return 0 if OK
 */
ac_u32 ac_exception_irq_register(int_handler handler, void* param) {
  ac_u32 status = 1;

  // Currently there is no unregtister so we're just racing
  // with the interrupt handler itself. The interrupt handler
  // will be looking at the handler only and if its not AC_NULL
  // assume is good. Thus we'll update that last when we add
  // a new entry.
  for (ac_u32 i = 0; i < MAX_HANDLERS; i++) {
    ac_bool* pavailable = &irq_handlers[i].available;
    ac_bool expected = AC_TRUE;
    ac_bool ok = __atomic_compare_exchange_n(pavailable, &expected, AC_FALSE,
        AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
    if (ok) {
      irq_handlers[i].param = param;
      int_handler* phandler = &irq_handlers[i].handler;
      __atomic_store_n(phandler, handler, __ATOMIC_RELEASE);
      status = 0;
      break;
    }
  }
  return status;
}

/**
 * Initialize this module
 */
void ac_exception_init() {
  for (ac_u32 i = 0; i < MAX_HANDLERS; i++) {
    irq_handlers[i].available = AC_TRUE;
    irq_handlers[i].handler = AC_NULL;
    irq_handlers[i].param = AC_NULL;
  }
}

void ac_exception_reset_handler(void) {
  ac_putchar('R');
  ac_putchar('\n');
}

void ac_exception_undef_handler(void) {
  ac_putchar('U');
  ac_putchar('\n');
}

void ac_exception_svc_handler(void) {
  ac_putchar('S');
  ac_putchar('\n');
}

void ac_exception_prefetch_abort_handler(void) {
  ac_putchar('P');
  ac_putchar('\n');
}

void ac_exception_data_abort_handler(void) {
  ac_putchar('D');
  ac_putchar('\n');
}

void ac_exception_irq_handler(void) {
  //ac_putchar('I');
  //ac_putchar('\n');
  for (ac_u32 i = 0; i < MAX_HANDLERS; i++) {
    if (irq_handlers[i].handler != AC_NULL) {
      irq_handlers[i].handler(irq_handlers[i].param);
    }
  }
}

void ac_exception_fiq_handler(void) {
  ac_putchar('F');
  ac_putchar('\n');
}

