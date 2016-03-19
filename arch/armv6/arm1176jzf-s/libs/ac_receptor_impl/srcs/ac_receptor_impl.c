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
 * Create a receptor and set it state to signaled parameter
 *
 * @param signaled is the initial state of the receptor
 *
 * @return AC_NULL if unable to allocate a receptor
 */
ac_receptor_t ac_receptor_create(ac_bool signaled) {
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
 * @return 0 if successfully waited, !0 indicates an error
 */
ac_uint ac_receptor_signal(ac_receptor_t receptor) {
  return 1;
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

