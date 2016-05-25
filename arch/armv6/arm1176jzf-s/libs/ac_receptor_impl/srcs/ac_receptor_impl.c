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

#include <ac_receptor.h>

#include <ac_inttypes.h>

#include <ac_printf.h>

#define NDEBUG
#include <ac_debug_printf.h>

/**
 * Create a receptor and set its state to NOT signaled
 *
 * @return AC_NULL if unable to allocate a receptor
 */
ac_receptor_t ac_receptor_create(void) {
  return AC_NULL;
}

/**
 * Destroy a receptor
 */
void ac_receptor_destroy(ac_receptor_t receptor) {
}

/**
 * Wait for the receptor to be signaled only one entity can wait
 * on a receptor at a time. If the receptor has already been signaled
 * ac_receptor_wait will return immediately.
 *
 * @return 0 if successfully waited, !0 indicates an error
 */
ac_uint ac_receptor_wait(ac_receptor_t receptor) {
  return 1;
}

/**
 * Signal the receptor.
 *
 * @param receptor to signal
 */
void ac_receptor_signal(ac_receptor_t receptor) {
}

/**
 * Signal the receptor and if there is another thread waiting
 * yield the CPU os it might run.
 *
 * @param receptor to signal
 */
void ac_receptor_signal_yield_if_waiting(ac_receptor_t receptor) {
}

/**
 * Initialize this module early, must be
 * called before receptor_init
 */
void ac_receptor_early_init(void) {
  ac_printf("ac_receptor_early_init:-does nothing\n");
}

/**
 * Initialize this module
 */
void ac_receptor_init(ac_uint number_receptors) {
  ac_printf("ac_receptor_init:-does nothing number_receptors=%d\n", number_receptors);
}

