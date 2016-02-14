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

#include <ac_printf.h>

#include <ac_inttypes.h>
#include <ac_string.h>
#include <ac_putchar.h>
#include <ac_arg.h>
//#include <ac_debug_assert.h>

#define NO_LEADING_0 AC_FALSE
#define RADIX16_LEADING_0 AC_TRUE

/**
 * Write a character using seL4_PutChar
 */
static const char* ret_empty(ac_writer* this) {
    return "";
}


/**
 * Write a character using seL4_PutChar
 */
static void write_char(ac_writer* this, void* param) {
    (void)(this);
    ac_putchar((ac_u8)(((ac_uptr)param) & 0xff));
}

/**
 * Output a string
 */
static ac_u32 write_str(ac_writer *writer, char *str) {
    ac_u32 count = 0;
    char ch;

    while((ch = *str++)) {
        writer->write_param(writer, cast_to_write_param(ch));
        count += 1;
    }

    return count;
}

/**
 * Output an unsigned int bit value
 */
static ac_u32 write_uint(
        ac_writer* writer, ac_uint val, ac_bool radix16Leading0, ac_uint radix) {
    static const char val_to_char[] = "0123456789abcdef";
    ac_u32 count = 0;
    char result[65];

    // Validate radix
    if ((radix <= 1) || (radix > sizeof(val_to_char))) {
        count = write_str(writer, "Bad Radix ");
        count += write_uint(writer, radix, NO_LEADING_0, 10);
    } else {
        ac_sint idx;
        for (idx = 0; idx < sizeof(result); idx++) {
            result[idx] = val_to_char[val % radix];
            val /= radix;
            if (val == 0) {
                break;
            }
        }
        count = idx + 1;
        if ((radix == 16) && radix16Leading0) {
            ac_sint pad0Count = (sizeof(val) * 2) - count;
            count += pad0Count;
            while (pad0Count-- > 0) {
                writer->write_param(writer, cast_to_write_param('0'));
            }
        }
        for (; idx >= 0; idx--) {
            writer->write_param(writer, cast_to_write_param(result[idx]));
        }
    }
    return count;
}

/**
 * Output an unsigned 32 bit value
 */
static ac_u32 write_u32(
        ac_writer* writer, ac_u32 val, ac_bool radix16Leading0, ac_u32 radix) {
    static const char val_to_char[] = "0123456789abcdef";
    ac_u32 count = 0;
    char result[65];

    // Validate radix
    if ((radix <= 1) || (radix > sizeof(val_to_char))) {
        count = write_str(writer, "Bad Radix ");
        count += write_u32(writer, radix, NO_LEADING_0, 10);
    } else {
        ac_s32 idx;
        for (idx = 0; idx < sizeof(result); idx++) {
            result[idx] = val_to_char[val % radix];
            val /= radix;
            if (val == 0) {
                break;
            }
        }
        count = idx + 1;
        if ((radix == 16) && radix16Leading0) {
            ac_s32 pad0Count = (sizeof(val) * 2) - count;
            count += pad0Count;
            while (pad0Count-- > 0) {
                writer->write_param(writer, cast_to_write_param('0'));
            }
        }
        for (; idx >= 0; idx--) {
            writer->write_param(writer, cast_to_write_param(result[idx]));
        }
    }
    return count;
}

/**
 * Output an unsigned 64 bit value
 */
static ac_u32 write_u64_radix16(
        ac_writer* writer, ac_u64 val, ac_bool radix16Leading0) {
    ac_u32 count = 0;
    ac_u32 upper = (val >> 32) & 0xFFFFFFFF;
    ac_u32 lower = val & 0xFFFFFFFF;
    if ((upper != 0) || radix16Leading0) {
        count = write_u32(writer, upper, radix16Leading0, 16);
        count += write_u32(writer, lower, RADIX16_LEADING_0, 16);
    } else {
        count += write_u32(writer, lower, radix16Leading0, 16);
    }
    return count;
}

/**
 * Output a signed int bit value
 */
static ac_u32 write_sint(ac_writer* writer, ac_sint val, ac_u32 radix) {
    ac_u32 count = 0;
    if ((val < 0) && (radix == 10)) {
        writer->write_param(writer, cast_to_write_param('-'));
        count += 1;
        val = -val;
    }
    return count + write_uint(writer, val, NO_LEADING_0, radix);
}

/**
 * Print a formatted string to the writer function. This supports a
 * subset of the typical libc printf:
 *   - %% ::= prints a percent
 *   - %d ::= prints a positive or negative long base 10
 *   - %u ::= prints an ac_u32 base 10
 *   - %x ::= prints a ac_u32 base 16
 *   - %p ::= prints a pointer base 16 with 0x prepended
 *   - %s ::= prints a string
 *   - %llx ::= prints a ac_u64 base 16
 *
 * Returns number of characters consumed
 */
static ac_u32 formatter(ac_writer* writer, const char* format, ac_va_list args) {
    ac_u32 count = 0;

    // Check inputs
    if (IS_AC_NULL(writer) || IS_AC_NULL(formatter)) {
        goto done;
    }

    if (writer->write_beg != AC_NULL) {
        writer->write_beg(writer);
    }
    char ch;
    while ((ch = *format++) != 0) {
        if (ch != '%') {
            // Not the format escape character
            writer->write_param(writer, cast_to_write_param(ch));
            count += 1;
        } else {
            // Is a '%' so get the next character to decide the format
            char next_ch = *format++;
            if (next_ch == 0) {
                count += 1;
                goto done;
            }
            switch (next_ch) {
                case '%': {
                    // was %% just echo a '%'
                    writer->write_param(writer, cast_to_write_param(next_ch));
                    count += 1;
                    break;
                }
                case 's': {
                    // Handle string specifier
                    char *s = ac_va_arg(args, char *);
                    count += write_str(writer, s);
                    break;
                }
                case 'b': {
                    count += write_uint(writer, ac_va_arg(args, ac_uint), NO_LEADING_0, 2);
                    break;
                }
                case 'd': {
                    count += write_sint(writer, ac_va_arg(args, ac_sint), 10);
                    break;
                }
                case 'u': {
                    count += write_uint(writer, ac_va_arg(args, ac_uint), NO_LEADING_0, 10);
                    break;
                }
                case 'x': {
                    count += write_uint(writer, ac_va_arg(args, ac_uint), NO_LEADING_0, 16);
                    break;
                }
                case 'l': {
                    if (ac_strncmp("lx", format, 2) == 0) {
                        format += 2;
                        count += write_u64_radix16(writer, ac_va_arg(args, ac_u64), NO_LEADING_0);
                    } else {
                        count += write_str(writer, "%l");
                    }
                    break;
                }
                case 'p': {
                    ac_u32 sz_ptr = sizeof(void *);
                    if (sz_ptr == sizeof(ac_uint)) {
                      count += write_uint(writer, ac_va_arg(args, ac_uint), RADIX16_LEADING_0, 16);
                    } else if (sz_ptr == sizeof(ac_u32)) {
                      count += write_u32(writer, ac_va_arg(args, ac_u32), RADIX16_LEADING_0, 16);
                    } else if (sz_ptr == sizeof(ac_u64)) {
                      count += write_u64_radix16(writer, ac_va_arg(args, ac_u64), RADIX16_LEADING_0);
                    } else {
                      write_str(writer, "Bad ptr size, expecting sizeof(ac_uint), 32 or 64 bit pointers:");
                      write_uint(writer, sz_ptr, 10, NO_LEADING_0);
                    }
                    break;
                }
                default: {
                    writer->write_param(writer, cast_to_write_param(ch));
                    writer->write_param(writer, cast_to_write_param(next_ch));
                    count += 1;
                    break;
                }
            }
        }
    }
    if (writer->write_end != AC_NULL) {
        writer->write_end(writer);
    }

done:
    return count;
}

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
 * Returns number of characters printed
 */
const char* ac_formatter(ac_writer* writer, const char *format, ...) {
    ac_va_list args;

    ac_va_start(args, format);
    formatter(writer, format, args);
    ac_va_end(args);

    if (writer->get_buff != AC_NULL) {
        return writer->get_buff(writer);
    } else {
        return "";
    }
}

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
 * Returns number of characters printed
 */
ac_u32 ac_printfw(ac_writer* writer, const char *format, ...) {
    ac_va_list args;
    ac_u32 count;

    ac_va_start(args, format);
    count = formatter(writer, format, args);
    ac_va_end(args);
    return count;
}

/**
 * Print a formatted string to seL4_PutChar. This supports a
 * subset of the typical libc printf:
 *   - %% ::= prints a percent
 *   - %d ::= prints a positive or negative long base 10
 *   - %u ::= prints an ac_u32 base 10
 *   - %x ::= prints a ac_u32 base 16
 *   - %p ::= prints a ac_u32 assuming its a pointer base 16 with 0x prepended
 *   - %s ::= prints a string
 *   - %llx ::= prints a ac_u32 long base 16
 *
 * Returns number of characters printed
 */
ac_u32 ac_printf(const char *format, ...) {
    ac_va_list args;
    ac_u32 count;
    ac_writer writer = {
            .get_buff = ret_empty,
            .write_beg = AC_NULL,
            .write_param = write_char,
            .write_end = AC_NULL,
            .data = AC_NULL,
    };

    ac_va_start(args, format);
    count = formatter(&writer, format, args);
    ac_va_end(args);
    return count;
}

