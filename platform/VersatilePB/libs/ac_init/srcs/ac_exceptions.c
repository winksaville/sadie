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
  ac_putchar('I');
  ac_putchar('\n');
}

void ac_exception_fiq_handler(void) {
  ac_putchar('F');
  ac_putchar('\n');
}

