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
 * Display memory using writer
 */
void ac_printw_mem(
    ac_writer* writer,    ///< writer
    char* leader,         ///< leader if AC_NULL no leader
    void *mem,            ///< Address of first memory location
    AcU32 len_in_elems,   ///< Number of elements to dump
    AcU32 bytes_per_elem, ///< Bytes per element 1, 2, 4, 8
    char* format,         ///< Format string such as %x %d ...
    char* sep,            ///< Seperator between elements, if AC_NULL no seperator
    char* trailer) {      ///< Trailer, if AC_NULL no trailer
  if (leader != AC_NULL) {
    ac_printfw(writer, leader);
  }
  unsigned char* p = (unsigned char*)mem;

  for (int i = 0; i < len_in_elems; i++) {
    if ((i != 0) && (sep != AC_NULL)) ac_printf("%s", sep);
    switch(bytes_per_elem) {
      case 8: ac_printfw(writer, format, ((AcU64*)p)[i]); break;
      case 4: ac_printfw(writer, format, ((AcU32*)p)[i]); break;
      case 2: ac_printfw(writer, format, ((AcU16*)p)[i]); break;
      case 1:
      default: ac_printfw(writer, format, ((AcU8*)p)[i]); break;
    }
  }

  if (trailer != AC_NULL) {
    ac_printfw(writer, trailer);
  }
}
