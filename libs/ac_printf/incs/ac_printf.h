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
    ac_get_buff_fn get_buff;        // Called at anytime and the contents of the buffer
                                    // or an empty string.
    ac_write_end_fn write_beg;      // Called before first writeParam, optional maybe ac_Null
    ac_write_param_fn write_param;  // Called to write the parameter
    ac_write_end_fn write_end;      // Called after last writeParam, optional maybe ac_Null
} ac_writer;

/**
 * Print a formatted string to the writer. This supports a
 * subset of the typical libc printf:
 *   - %% ::= prints a percent
 *   - %d ::= prints a positive or negative long base 10
 *   - %u ::= prints an ac_u32 base 10
 *   - %x ::= prints a ac_u32 base 16
 *   - %p ::= prints a ac_u32 assuming its a pointer base 16 with 0x prepended
 *   - %s ::= prints a string
 *   - %llx ::= prints a ac_u32 long base 16
 *
 * Returns returns what writer->get_buff() returns if it exists and doesn't
 * return AC_NULL. If get_buff doesn't exists or does return AC_NULL at least
 * and enpty string is returned. So AC_NULL is NEVER returned.
 */
const char* ac_formatter(ac_writer* writer, const char *format, ...);

/**
 * Print a formatted string to the writer function. This supports a
 * subset of the typical libc printf:
 *   - %% ::= prints a percent
 *   - %s ::= prints a string
 *   - %p ::= prints a pointer base 16 with leading zero's
 *   - %b ::= prints a ac_uint base 2
 *   - %d ::= prints a ac_sint base 10
 *   - %u ::= prints a ac_uint base 10
 *   - %x ::= prints a ac_uint base 16
 *   - For %b, %d, %u, %x can be preceeded by "l" or "ll" to
 *   - print a 64 bit value in the requested radix.
 *
 * Returns writer->count which should be the number of characters printed
 */
ac_uint ac_printfw(ac_writer* writer, const char *format, ...);

/**
 * Print a formatted string to seL4_PutChar. This supports a
 * subset of the typical libc printf:
 *   - %% ::= prints a percent
 *   - %s ::= prints a string
 *   - %p ::= prints a pointer base 16 with leading zero's
 *   - %b ::= prints a ac_uint base 2
 *   - %d ::= prints a ac_sint base 10
 *   - %u ::= prints a ac_uint base 10
 *   - %x ::= prints a ac_uint base 16
 *   - For %b, %d, %u, %x can be preceeded by "l" or "ll" to
 *   - print a 64 bit value in the requested radix.
 *
 * Returns number of characters printed
 */
ac_uint ac_printf(const char *format, ...);

/**
 * Print a formatted string to the output buffer. This supports a
 * subset of the typical libc printf:
 *   - %% ::= prints a percent
 *   - %s ::= prints a string
 *   - %p ::= prints a pointer base 16 with leading zero's
 *   - %b ::= prints a ac_uint base 2
 *   - %d ::= prints a ac_sint base 10
 *   - %u ::= prints a ac_uint base 10
 *   - %x ::= prints a ac_uint base 16
 *   - For %b, %d, %u, %x can be preceeded by "l" or "ll" to
 *   - print a 64 bit value in the requested radix.
 *
 * Returns number of characters printed
 */
ac_uint ac_sprintf(ac_u8* out_buff, ac_uint out_buff_len, const char *format, ...);

#endif
