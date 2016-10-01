/*
 * Copyright 2015 Wink Saville
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SADIE_LIBS_AC_PRINTF_INCS_PRINTF_H
#define SADIE_LIBS_AC_PRINTF_INCS_PRINTF_H

#include <ac_arg.h>
#include <ac_inttypes.h>

// Cast to a void * for use with valid ac_Write_fn param
#define cast_to_write_param(x) ((void *)(ac_uptr)(x))

// Forward declaration of the ac_Writer typedef
typedef struct _ac_writer ac_writer;

// A function called before first WriteParam is called
typedef void (*ac_write_beg_fn)(ac_writer* this);

// A function to Write the parameter to the ac_Writer
typedef void (*ac_write_param_fn)(ac_writer* this, void* write_param);

// A function called after all WriteParam's have been called
typedef void (*ac_write_end_fn)(ac_writer* this);

// A function called to return the buffer associated with writer
// if the function cannot return the buffer it must return a
// pointer to a byte of 0, i.e. an empty string
typedef const char* (*ac_get_buff_fn)(ac_writer* this);

/**
 * An ac_writer which is used by ac_printf to print
 */
typedef struct _ac_writer {
    ac_uint count;                  // Number of character written since last write_beg call
    ac_uint max_len;                // max_len to output, only used under some conditons, for
                                    // instance by a buffer writer.
    void* data;                     // Typically a buffer or the writers control data
    ac_u8 sign_to_print;            // 0 if no size otherwise '-', '+'
    ac_bool left_justified;         // Flag left justifed = '-'
    ac_bool alt_form;               // Flag alt_form = AC_TRUE if '#' present, default AC_FALSE
    ac_bool leading_0;              // Flag leading_0 = AC_TRUE if '0' present, default AC_FALSE
    ac_uint min_width;              // Minimum width = numeric, default 0
    ac_uint precision;              // Precision = numeric, default 0
    ac_get_buff_fn get_buff;        // Called at anytime and the contents of the buffer
                                    // or an empty string.
    ac_write_end_fn write_beg;      // Called before first writeParam, optional maybe ac_Null
    ac_write_param_fn write_param;  // Called to write the parameter
    ac_write_end_fn write_end;      // Called after last writeParam, optional maybe ac_Null
} ac_writer;

/**
 * A procedure invoked when a format ch is processed
 *
 * Below is an example which assumes writes a signed
 * value in decimal that is a ac_u64. It returns
 * sizeof(ac_u64) / sizeof(ac_uint) if it had consumed
 * a ac_u64 and an ac_u32 than the value returned
 * should be (sizeof(ac_u64) + sizeof(ac_uint)) / sizeof(ac_uint).
 *
 * Of course this assumes sizeof(ac_uint) is the size of an ac_u32
 * on the stack!  if maybe necessary to use conditional compiliation to
 * get this correct, so BE CAREFUL.
 *
 * <code>
 *   static ac_u32 printf_ff_format_proc(ac_writer* writer, ac_u8 ch, ac_va_list args) {
 *     ac_u64 value = ac_va_arg(args, ac_u64);
 *     ac_printf_write_sval(writer, value, sizeof(ac_u64), 10);
 *     return sizeof(ac_u64) / sizeof(ac_uint);
 *   }
 * </code>
 *
 * @params writer to use for writing
 * @params ch is the format character invoking this call
 * @params args is a ac_va_list of arguments
 *
 * @return number of ac_uint args consumed during the invocation.
 */
typedef ac_u32 (*ac_printf_format_proc)(ac_writer* writer, ac_u8 ch, ac_va_list args);

typedef ac_u32 (*ac_printf_format_proc_str)(ac_writer* writer, const char* str, ac_va_list args);

/**
 *  Write a character
 */
void ac_printf_write_char(ac_writer* writer, char ch);

/**
 * Write a a string
 */
void ac_printf_write_str(ac_writer* writer, char* str);

/**
 * Write an unsigned value
 */
void ac_printf_write_uval(
        ac_writer* writer, ac_u64 val, ac_uint sz_val_in_bytes, ac_uint radix);

/**
 * Write a signed value
 */
void ac_printf_write_sval(
        ac_writer* writer, ac_s64 val, ac_uint sz_val_in_bytes, ac_uint radix);

/**
 * Register a format processor for ch
 *
 * @param fn is the format processing function
 * @param ch is the format character which causes fn to be invoked.
 *
 * @return 0 if registered successfully
 */
ac_uint ac_printf_register_format_proc(ac_printf_format_proc format_proc, ac_u8 ch);

/**
 * Register a format processor for string
 *
 * @param fn is the format processing function
 * @param str is the format string which causes fn to be invoked.
 *
 * @return 0 if registered successfully
 */
ac_uint ac_printf_register_format_proc_str(ac_printf_format_proc_str format_proc_str, const char* str);

/**
 * Print a formatted string to the writer. This supports a
 * subset of the typical libc printf:
 *
 * Below the items in braces are optional with '%' the introducer
 *
 * {<%>{flags}{min_width}{'.'precison}{len modifier}<format char>}
 *
 * flags :
 *   - sign = '+', '-', default ' '
 *   - alt_form = '#', default AC_FALSE
 *   - leading_0 = '0', default AC_FALSE
 *
 * min_width:
 *   - Numeric const value or '*', default 0
 *
 * precision:
 *   - Numeric const value or '*', default 0
 *
 * len modifier:
 *   - 'l' or 'll'
 *
 * format char:
 *   - %% ::= prints a percent
 *   - %s ::= prints a string
 *   - %p ::= prints a pointer base 16 with leading zero's
 *   - %b ::= prints a ac_u32 base 2
 *   - %d ::= prints a ac_s32 base 10
 *   - %u ::= prints a ac_u32 base 10
 *   - %x ::= prints a ac_u32 base 16
 *   - For %b, %d, %u, %x can be preceeded by "l" or "ll" to
 *   - print a 64 bit value in the requested radix.
 *   - %i ::= prints a ac_sint base 10
 *   - %v ::= prints a ac_uint base 10
 *   - %y ::= prints a ac_uint base 16
 *
 * Returns returns what writer->get_buff() returns if it exists and doesn't
 * return AC_NULL. If get_buff doesn't exists or does return AC_NULL at least
 * and enpty string is returned. So AC_NULL is NEVER returned.
 */
const char* ac_formatter(ac_writer* writer, const char *format, ...);

/**
 * Print a formatted string to the writer function. This supports a
 * subset of the typical libc printf:
 *
 * Below the items in braces are optional with '%' the introducer
 *
 * {<%>{flags}{min_width}{'.'precison}{len modifier}<format char>}
 *
 * flags :
 *   - sign = '+', '-', default ' '
 *   - alt_form = '#', default AC_FALSE
 *   - leading_0 = '0', default AC_FALSE
 *
 * min_width:
 *   - Numeric const value or '*', default 0
 *
 * precision:
 *   - Numeric const value or '*', default 0
 *
 * len modifier:
 *   - 'l' or 'll'
 *
 * format char:
 *   - %% ::= prints a percent
 *   - %s ::= prints a string
 *   - %p ::= prints a pointer base 16 with leading zero's
 *   - %b ::= prints a ac_u32 base 2
 *   - %d ::= prints a ac_s32 base 10
 *   - %u ::= prints a ac_u32 base 10
 *   - %x ::= prints a ac_u32 base 16
 *   - For %b, %d, %u, %x can be preceeded by "l" or "ll" to
 *   - print a 64 bit value in the requested radix.
 *   - %i ::= prints a ac_sint base 10
 *   - %v ::= prints a ac_uint base 10
 *   - %y ::= prints a ac_uint base 16
 *
 * Returns writer->count which should be the number of characters printed
 */
ac_uint ac_printfw(ac_writer* writer, const char *format, ...);

/**
 * Print a formatted string to seL4_PutChar. This supports a
 * subset of the typical libc printf:
 *
 * Below the items in braces are optional with '%' the introducer
 *
 * {<%>{flags}{min_width}{'.'precison}{len modifier}<format char>}
 *
 * flags :
 *   - sign = '+', '-', default ' '
 *   - alt_form = '#', default AC_FALSE
 *   - leading_0 = '0', default AC_FALSE
 *
 * min_width:
 *   - Numeric const value or '*', default 0
 *
 * precision:
 *   - Numeric const value or '*', default 0
 *
 * len modifier:
 *   - 'l' or 'll'
 *
 * format char:
 *   - %% ::= prints a percent
 *   - %s ::= prints a string
 *   - %p ::= prints a pointer base 16 with leading zero's
 *   - %b ::= prints a ac_u32 base 2
 *   - %d ::= prints a ac_s32 base 10
 *   - %u ::= prints a ac_u32 base 10
 *   - %x ::= prints a ac_u32 base 16
 *   - For %b, %d, %u, %x can be preceeded by "l" or "ll" to
 *   - print a 64 bit value in the requested radix.
 *   - %i ::= prints a ac_sint base 10
 *   - %v ::= prints a ac_uint base 10
 *   - %y ::= prints a ac_uint base 16
 *
 * Returns writer->count which should be the number of characters printed
 */
ac_uint ac_printf(const char *format, ...);

/**
 * @return the default ac_printf writer
 */
ac_writer* AcPrintf_get_writer(void);

/**
 * @return the default ac_printf writer after initializing to defaults
 */
ac_writer* AcPrintf_get_writer_inited(void);

/**
 * Print a formatted string to the output buffer. This supports a
 * subset of the typical libc printf:
 *
 * Below the items in braces are optional with '%' the introducer
 *
 * {<%>{flags}{min_width}{'.'precison}{len modifier}<format char>}
 *
 * flags :
 *   - sign = '+', '-', default ' '
 *   - alt_form = '#', default AC_FALSE
 *   - leading_0 = '0', default AC_FALSE
 *
 * min_width:
 *   - Numeric const value or '*', default 0
 *
 * precision:
 *   - Numeric const value or '*', default 0
 *
 * len modifier:
 *   - 'l' or 'll'
 *
 * format char:
 *   - %% ::= prints a percent
 *   - %s ::= prints a string
 *   - %p ::= prints a pointer base 16 with leading zero's
 *   - %b ::= prints a ac_u32 base 2
 *   - %d ::= prints a ac_s32 base 10
 *   - %u ::= prints a ac_u32 base 10
 *   - %x ::= prints a ac_u32 base 16
 *   - For %b, %d, %u, %x can be preceeded by "l" or "ll" to
 *   - print a 64 bit value in the requested radix.
 *   - %i ::= prints a ac_sint base 10
 *   - %v ::= prints a ac_uint base 10
 *   - %y ::= prints a ac_uint base 16
 *
 * Returns writer->count which should be the number of characters printed
 */
ac_uint ac_snprintf(ac_u8* out_buff, ac_uint out_buff_len, const char *format, ...);

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
    char* trailer);       ///< Trailer, if AC_NULL no trailer

/**
 * Display memory
 */
void ac_print_mem(
    char* leader,         ///< leader if AC_NULL no leader
    void *mem,            ///< Address of first memory location
    AcU32 len_in_elems,   ///< Number of elements to dump
    AcU32 bytes_per_elem, ///< Bytes per element 1, 2, 4, 8
    char* format,         ///< Format string such as %x %d ...
    char* sep,            ///< Seperate between elements
    char* trailer);       ///< Trailer if AC_NULL no trailer


/**
 * Display a buffer
 *
 * @param p = pointer to buffer
 * @param len = length of buffe3r
 */
void ac_print_buff(AcU8 *p, AcS32 len);

/**
 * Display memory as hex bytes
 */
void ac_println_hex(
    char* leader,         ///< leader if AC_NULL no leader
    void *mem,            ///< Address of first memory location
    AcU32 len,            ///< Number of bytes to print
    char* sep);           ///< Separator between bytes

/**
 * Display memory as decimal bytes
 */
void ac_println_dec(
    char* leader,         ///< leader if AC_NULL no leader
    void *mem,            ///< Address of first memory location
    AcU32 len,            ///< Number of bytes to print
    char* sep);           ///< Separator between bytes

#endif
