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
 * Display memory
 */
void ac_print_mem(
    char* leader,         ///< leader if AC_NULL no leader
    void *mem,            ///< Address of first memory location
    AcU32 len_in_elems,   ///< Number of elements to dump
    AcU32 bytes_per_elem, ///< Bytes per element 1, 2, 4, 8
    char* format,         ///< Format string such as %x %d ...
    char* sep,            ///< Seperator between elements
    char* trailer) {      ///< Trailer if AC_NULL no trailer
  ac_printw_mem(AcPrintf_get_writer(), leader, mem, len_in_elems, bytes_per_elem,
      format, sep, trailer);
}

/**
 * Display memory  as hex bytes
 */
void ac_println_hex(
    char* leader,         ///< leader if AC_NULL no leader
    void *mem,            ///< Address of first memory location
    AcU32 len,            ///< Number of bytes to print
    char* sep) {          ///< Separator between bytes
  ac_print_mem(leader, mem, len, 1, "%02x", sep, "\n");
}

/**
 * Display memory  as decimal bytes
 */
void ac_println_dec(
    char* leader,         ///< leader if AC_NULL no leader
    void *mem,            ///< Address of first memory location
    AcU32 len,            ///< Number of bytes to print
    char* sep) {          ///< Separator between bytes
  ac_print_mem(leader, mem, len, 1, "%3d", sep, "\n");
}
