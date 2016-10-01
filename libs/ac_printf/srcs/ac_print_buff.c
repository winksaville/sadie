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

//#define NDEBUG
#include <ac_debug_printf.h>
#include <ac_printf.h>
#include <ac_memset.h>
#include <ac_memcpy.h>
#include <ac_string.h>

/**
 * Display a buffer
 *
 * @param p = pointer to buffer
 * @param len = length of buffe3r
 */
void ac_print_buff(AcU8 *p, AcS32 len) {
  while (len > 0) {
    ac_printf("%p: ", p);

    if (len > 8) {
      ac_print_mem(AC_NULL, p, 8, 1, "%02x", " ", "  ");
      len -= 8;
      p += 8;
    }

    AcU32 cnt;
    if (len > 8) {
      cnt = 8;
    } else {
      cnt = len;
    }
    ac_print_mem(AC_NULL, p, cnt, 1, "%02x", " ", "\n");
    len -= cnt;
    p += cnt;
  }
}
