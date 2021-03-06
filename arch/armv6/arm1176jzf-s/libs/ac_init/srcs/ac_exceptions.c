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

#include <ac_attributes.h>
#include <ac_inttypes.h>
#include <ac_putchar.h>
#include <ac_timer.h>

void ac_exception_reset_handler(void) AC_ATTR_INTR("UNDEF");

void ac_exception_undef_handler(void) AC_ATTR_INTR("UNDEF");

void ac_exception_svc_handler(void) AC_ATTR_INTR("SWI");

void ac_exception_prefetch_abort_handler(void) AC_ATTR_INTR("ABORT");

void ac_exception_data_abort_handler(void) AC_ATTR_INTR("ABORT");

//void ac_exception_irq_handler(void) AC_ATTR_INTR("IRQ");

void ac_exception_fiq_handler(void) AC_ATTR_INTR("FIQ");

typedef struct {
  ac_bool available;
  ac_uptr param;
  identify_and_clear_source iacs;
  int_handler handler;
} irq_handler_obj;

#define MAX_HANDLERS 8

static ac_u32 irq_handler_count;
static irq_handler_obj irq_handlers[MAX_HANDLERS];

/**
 * Register a irq handler and its parameter.
 *
 * return 0 if OK
 */
ac_u32 ac_exception_irq_register(int_handler handler,
    identify_and_clear_source iacs, ac_uptr param) {
  ac_u32 status = 1;

  // Currently there is no unregtister so we're just racing
  // with the interrupt handler itself. The interrupt handler
  // will be looking at the handler only and if its not AC_NULL
  // assume is good. Thus we'll update that last when we add
  // a new entry.
  for (ac_u32 i = irq_handler_count; i < MAX_HANDLERS; i++) {
    ac_bool* pavailable = &irq_handlers[i].available;
    ac_bool expected = AC_TRUE;
    ac_bool ok = __atomic_compare_exchange_n(pavailable, &expected,
        AC_FALSE, AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
    if (ok) {
      irq_handlers[i].param = param;
      irq_handlers[i].iacs = iacs;
      int_handler* phandler = &irq_handlers[i].handler;
      __atomic_store_n(phandler, handler, __ATOMIC_RELEASE);
      irq_handler_count += 1;
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
  irq_handler_count = 0;
  for (ac_u32 i = 0; i < MAX_HANDLERS; i++) {
    irq_handlers[i].available = AC_TRUE;
    irq_handlers[i].param = 0;
    irq_handlers[i].iacs = AC_NULL;
    irq_handlers[i].handler = AC_NULL;
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

void ac_exception_irq_identify_and_clear_source(ac_uptr param) {
  //ac_putchar('I');
  //ac_putchar('\n');
  for (ac_u32 i = 0; i < irq_handler_count; i++) {
    // We use handler as if its set then icas is set.
    if (irq_handlers[i].handler != AC_NULL) {
      irq_handlers[i].iacs(irq_handlers[i].param);
    }
  }
}

void ac_exception_irq_handler(void) {
  //ac_putchar('I');
  //ac_putchar('\n');
  for (ac_u32 i = 0; i < irq_handler_count; i++) {
    if (irq_handlers[i].handler != AC_NULL) {
      irq_handlers[i].handler(irq_handlers[i].param);
    }
  }
}

void ac_exception_fiq_handler(void) {
  ac_putchar('F');
  ac_putchar('\n');
}

