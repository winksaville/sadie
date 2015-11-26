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

#ifndef SADIE_LIBS_AC_DEBUG_PRINTF_INCS_PRINTF_H
#define SADIE_LIBS_AC_DEBUG_PRINTF_INCS_PRINTF_H

#ifdef NDEBUG

/** NOP ac_debug_printfw as NDEBUG is defined */
#define ac_debug_printfw(...) ((void)(0))

/** NOP ac_debug_printf as NDEBUG is defined */
#define ac_debug_printf(...) ((void)(0))

#else

#include <ac_printf.h>

/**
 * Conditional printf to the writer. This supports a
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
#define ac_debug_printfw(writer, ...) ac_printf(writer, __VA_ARGS__)

/**
 * Conditional printf a "terminal". This supports a
 * subset of the typical libc printf:
 *   - %% ::= prints a percent
 *   - %d ::= prints a positive or negative long base 10
 *   - %u ::= prints an unsigned long base 10
 *   - %x ::= prints a unsigned long base 16
 *   - %p ::= prints a unsigned long assuming its a pointer base 16 with 0x prepended
 *   - %s ::= prints a string
 *   - %llx ::= prints a unsigned long long base 16
 */
#define ac_debug_printf(...) ac_printf(__VA_ARGS__)

#endif

#endif
