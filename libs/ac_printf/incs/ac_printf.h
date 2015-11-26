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
 * An ac_Writer which has a function that processes the parameter
 * using any information needed in the ac_Writer.
 */
typedef struct _ac_writer {
    ac_get_buff_fn get_buff;        // Called at anytime and must return an empty string
    ac_write_end_fn write_beg;      // Called before first writeParam, optional maybe ac_Null
    ac_write_param_fn write_param;  // Called to write the parameter
    ac_write_end_fn write_end;      // Called after last writeParam, optional maybe ac_Null
    void* data;                     // Typically a pointer to the writers control data
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
 * Returns a pointer to the buffer or pointer to an empty string "".
 */
const char* ac_formatter(ac_writer* writer, const char *format, ...);

/**
 * Print a formatted string to the writer. This supports a
 * subset of the typical libc printf:
 *   - %% ::= prints a percent
 *   - %d ::= prints a positive or negative long base 10
 *   - %u ::= prints an ac_Uint32 base 10
 *   - %x ::= prints a ac_Uint32 base 16
 *   - %p ::= prints a ac_Uint32 assuming its a pointer base 16 with 0x prepended
 *   - %s ::= prints a string
 *   - %llx ::= prints a ac_Uint32 long base 16
 *
 * Returns number of characters printed
 */
ac_u32 ac_printfw(ac_writer* writer, const char *format, ...);

/**
 * Print a formatted string to ac_putchar. This supports a
 * subset of the typical libc printf:
 *   - %% ::= prints a percent
 *   - %d ::= prints a positive or negative long base 10
 *   - %u ::= prints an ac_Uint32 base 10
 *   - %x ::= prints a ac_Uint32 base 16
 *   - %p ::= prints a ac_Uint32 assuming its a pointer base 16 with 0x prepended
 *   - %s ::= prints a string
 *   - %llx ::= prints a ac_Uint32 long base 16
 *
 * Returns number of characters printed
 */
ac_u32 ac_printf(const char *format, ...);

#endif
