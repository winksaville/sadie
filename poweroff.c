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

#include "inttypes.h"

void poweroff(void) {
  uint32_t* pUnlockResetReg = (uint32_t*)0x10000020;
  uint32_t* pResetReg = (uint32_t*)0x10000040;
  
  // If qemu is executed with -no-reboot option then
  // resetting the board will cause qemu to exit
  // and it won't be necessary to ctrl-a, x to exit.
  // See http://lists.nongnu.org/archive/html/qemu-discuss/2015-10/msg00057.html

  // For arm926ej-s you unlock the reset register
  // then reset the board, I'm resetting to level 6

  *pUnlockResetReg = 0xA05F;
  *pResetReg = 0x106;
}
