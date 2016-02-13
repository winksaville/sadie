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

#include <ac_inttypes.h>
#include <ac_poweroff.h>
#include <reset_x86.h>

extern void main(void);
extern void ac_init(ac_uptr ptr, ac_uint uword);

__attribute__ ((__noreturn__))
void ac_startup(ac_uptr ptr, ac_uint uword) {
  ac_init(ptr, uword);
  main();
  ac_poweroff();

  /*
   * If poweroff didn't work, reset the cpu
   */
  reset_x86();
}
