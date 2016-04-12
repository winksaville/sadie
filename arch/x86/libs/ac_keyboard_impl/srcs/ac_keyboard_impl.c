/*
 * copyright 2016 wink saville
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

#include <ac_keyboard_impl.h>

#include <interrupts_x86.h>

#include <ac_inttypes.h>
#include <ac_printf.h>

/**
 * Non blocking read of a character from keyboard
 *
 * @return -1 if no character
 */
ac_sint ac_keyboard_read_non_blocking(void) {
  return -1;
}

/**
 * Initialize this module early, must be
 * called before keyboard_init
 */
__attribute__((__constructor__))
void ac_keyboard_early_init(void) {
  ac_printf("ac_keyboard_early_init:+\n");

  ac_printf("ac_keyboard_early_init:-\n");
}

/**
 * Initialize this module
 */
void ac_keyboard_init(void) {
  ac_printf("ac_keyboard_init:+\n");

  ac_printf("ac_keyboard_init:-\n");
}
