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

#include <ac_poweroff.h>

#include <ac_io.h>

void ac_poweroff(void) {
  // [From here](http://forum.osdev.org/viewtopic.php?t=16990)
  // didn't work but I'm hoping to find something easy.
  outw(0xB004, 0x2000);
}
