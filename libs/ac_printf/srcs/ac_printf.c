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

#define NDEBUG

#include <ac_printf.h>
#include <ac_debug_printf.h>

#include <ac_inttypes.h>
#include <ac_string.h>
#include <ac_putchar.h>
#include <ac_arg.h>
//#include <ac_debug_assert.h>

typedef struct ac_printf_t {
  ac_printf_format_proc format_proc;
  ac_u8 ch;
} ac_printf_t;

#define AC_PRINTF_TS_MAX 16
static const ac_uint ac_printf_ts_max = AC_PRINTF_TS_MAX;
static ac_uint ac_printf_ts_count = 0;
static ac_printf_t ac_printf_ts[AC_PRINTF_TS_MAX];

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
    this->count += 1;
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
 * Output an unsigned int value
 */
static ac_u32 write_uval(
        ac_writer* writer, ac_u64 val, ac_uint sz_val_in_bytes, ac_uint radix) {
    static const char val_to_char[] = "0123456789abcdef";
    ac_u32 count = 0;
    char result[65];

    // Validate radix
    if ((radix <= 1) || (radix > sizeof(val_to_char))) {
        count = write_str(writer, "Bad Radix ");
        count += write_uval(writer, sz_val_in_bytes, radix, 10);
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
        if ((radix == 16) && writer->leading_0) {
            ac_sint pad0Count = (sz_val_in_bytes * 2) - count;
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
 * Write an unsigned value
 */
ac_u32 ac_printf_write_uval(
        ac_writer* writer, ac_u64 val, ac_uint sz_val_in_bytes, ac_uint radix) {
  return write_uval(writer, val, sz_val_in_bytes, radix);
}

/**
 * Output an signed int val
 */
static ac_u32 write_sval(
        ac_writer* writer, ac_s64 val, ac_uint sz_val_in_bytes, ac_uint radix) {
    ac_u32 count = 0;
    if (val < 0) {
        writer->write_param(writer, cast_to_write_param('-'));
        count += 1;
        val = -val;
    }
    return count + write_uval(writer, val, sizeof(ac_uint), 10);
}

/**
 * Write a signed value
 */
ac_u32 ac_printf_write_sval(
        ac_writer* writer, ac_s64 val, ac_uint sz_val_in_bytes, ac_uint radix) {
  return write_sval(writer, val, sz_val_in_bytes, radix);
}

/**
 * Get a number from format
 */
static ac_uint get_number(char ch, char** format) {
  char* f = *format;
  ac_uint val = 0;
  do {
    val *= 10;
    val += (ac_uint)(ch - '0');
    ch = *f++;
  } while ((ch >= '0') && (ch <= '9'));
  *format = --f;
  return val;
}


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
 * Returns number of characters consumed
 */
static ac_u32 formatter(ac_writer* writer, char const* format, ac_va_list args) {
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
            // Is a '%' so get the next character
            char next_ch = *format++;

            // Get Flags
            writer->sign = ' ';
            writer->alt_form = AC_FALSE;
            writer->leading_0 = AC_FALSE;
            ac_bool flags = AC_TRUE;
            while (flags) {
              switch (next_ch) {
                case 0:
                  count += 1;
                  goto done;
                case '+':
                case '-':
                  writer->sign = next_ch;
                  next_ch = *format++;
                  ac_debug_printf("sign=%c\n", writer->sign);
                  break;
                case '#':
                  writer->alt_form = AC_TRUE;
                  next_ch = *format++;
                  ac_debug_printf("alt_form=%d\n", writer->alt_form);
                  break;
                case '0':
                  writer->leading_0 = AC_TRUE;
                  next_ch = *format++;
                  ac_debug_printf("leading_0=%d\n", writer->leading_0);
                  break;
                default:
                  flags = AC_FALSE;
                  break;
              }
            }
            // Get min_width if any
            writer->min_width = 0;
            if ((next_ch >= '1') && (next_ch <= '9')) {
                writer->min_width = get_number(next_ch, (char**)&format);
                next_ch = *format++;
                ac_debug_printf("min_width=%d\n", writer->min_width);
            } else if (next_ch == '*') {
                writer->min_width = ac_va_arg(args, ac_uint);
                next_ch = *format++;
                ac_debug_printf("min_width=%d\n", writer->min_width);
            }
            // Get precision if any
            writer->precision = 0;
            if (next_ch == '.') {
                next_ch = *format++;
                if ((next_ch >= '0') && (next_ch <= '9')) {
                    writer->precision = get_number(next_ch, (char**)&format);
                    next_ch = *format++;
                } else if (next_ch == '*') {
                    writer->precision = ac_va_arg(args, ac_uint);
                    next_ch = *format++;
                }
                ac_debug_printf("precision=%d\n", writer->precision);
            }
            switch (next_ch) {
                case 0:
                    count += 1;
                    goto done;
                case '%': {
                    // was %% just echo a '%'
                    writer->write_param(writer, cast_to_write_param(next_ch));
                    count += 1;
                    break;
                }
                case 'c': {
                    char c = (char)ac_va_arg(args, ac_uint);
                    if ((c < 0x20) || (c > 0x7F)) {
                      c = 0xff;
                    }
                    writer->write_param(writer, cast_to_write_param(c));
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
                    count += write_uval(writer, ac_va_arg(args, ac_uint),
                       sizeof(ac_uint), 2);
                    break;
                }
                case 'd': {
                    count += write_sval(writer, ac_va_arg(args, ac_sint),
                       sizeof(ac_uint), 2);
                    break;
                }
                case 'u': {
                    count += write_uval(writer, ac_va_arg(args, ac_uint),
                        sizeof(ac_uint), 10);
                    break;
                }
                case 'x': {
                    count += write_uval(writer, ac_va_arg(args, ac_uint),
                        sizeof(ac_uint), 16);
                    break;
                }
                case 'l': {
                    ac_bool longlong = AC_FALSE;
                    // ll and l will be ac_u64
                    if (ac_strncmp("l", format, 1) == 0) {
                      longlong = AC_TRUE;
                      format += 1;
                    }
                    if (ac_strncmp("b", format, 1) == 0) {
                        format += 1;
                        count += write_uval(writer, ac_va_arg(args, ac_u64),
                            sizeof(ac_u64), 2);
                    } else if (ac_strncmp("d", format, 1) == 0) {
                        format += 1;
                        count += write_sval(writer, ac_va_arg(args, ac_u64),
                            sizeof(ac_u64), 10);
                    } else if (ac_strncmp("u", format, 1) == 0) {
                        format += 1;
                        count += write_uval(writer, ac_va_arg(args, ac_u64),
                            sizeof(ac_u64), 10);
                    } else if (ac_strncmp("x", format, 1) == 0) {
                        format += 1;
                        count += write_uval(writer, ac_va_arg(args, ac_u64),
                            sizeof(ac_u64), 16);
                    } else {
                        if (longlong) {
                          count += write_str(writer, "%ll");
                        } else {
                          count += write_str(writer, "%l");
                        }
                    }
                    break;
                }
                case 'p': {
                    writer->leading_0 = AC_TRUE;
                    count += write_uval(writer, (ac_u64)(ac_uptr)ac_va_arg(args, void*),
                        sizeof(void*), 16);
                    break;
                }
                default: {
                    // Check the ac_printf_t's list
                    ac_bool processed_format_ch = AC_FALSE;
                    for (ac_uint i = 0; i < ac_printf_ts_max; i++) {
                        if (ac_printf_ts[i].ch == next_ch) {
                            ac_printf_ts[i].format_proc(writer, next_ch, args);
                            count += 1;
                            processed_format_ch = AC_TRUE;
                            break;
                        }
                    }

                    if (!processed_format_ch) {
                        writer->write_param(writer, cast_to_write_param(ch));
                        writer->write_param(writer, cast_to_write_param(next_ch));
                        count += 1;
                    }
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
 * Print a formatted string to the writer.
 *
 * Returns executes writer->get_buff() which must at least
 * return an empty string, it will never be AC_NULL.
 */
const char* ac_formatter(ac_writer* writer, const char *format, ...) {
    ac_va_list args;

    ac_va_start(args, format);
    formatter(writer, format, args);
    ac_va_end(args);

    if (writer->get_buff != AC_NULL) {
        const char* ptr = writer->get_buff(writer);
        if (ptr == AC_NULL) {
          ptr = "";
        }
        return ptr;
    } else {
        return "";
    }
}

/**
 * Print a formatted string to the writer function.
 *
 * Returns writer->count which should be the number of characters printed
 */
ac_uint ac_printfw(ac_writer* writer, const char *format, ...) {
    ac_va_list args;

    ac_va_start(args, format);
    formatter(writer, format, args);
    ac_va_end(args);
    return writer->count;
}

/**
 * Print a formatted string to seL4_PutChar.
 *
 * Returns number of characters printed
 */
ac_uint ac_printf(const char *format, ...) {
    ac_va_list args;

    ac_writer writer = {
            .count = 0,
            .max_len = 0,
            .data = AC_NULL,
            .get_buff = ret_empty,
            .write_beg = AC_NULL,
            .write_param = write_char,
            .write_end = AC_NULL,
    };

    ac_va_start(args, format);
    formatter(&writer, format, args);
    ac_va_end(args);
    return writer.count;
}

static const char* sprintf_get_buff(ac_writer *writer) {
  ((ac_u8*)(writer->data))[writer->count] = 0;
  return writer->data;
}

static void sprintf_write_beg(ac_writer* writer) {
  writer->count = 0;
}

static void sprintf_write_param(ac_writer* writer, void* param) {
  if (writer->count < (writer->max_len - 1)) {
    ((ac_u8*)(writer->data))[writer->count++] = ((ac_u8)(((ac_uptr)param) & 0xff));
  }
}

static void sprintf_write_end(ac_writer* writer) {
  ((ac_u8*)(writer->data))[writer->count] = 0;
}

/**
 * Initialize a buffer writer
 */
ac_writer* ac_writer_buffer_init(ac_writer* writer, ac_u8* out_buff,
    ac_uint out_buff_len) {

  writer->count = 0;
  writer->max_len = out_buff_len;
  writer->data = out_buff;
  writer->get_buff = sprintf_get_buff;
  writer->write_beg = sprintf_write_beg;
  writer->write_param = sprintf_write_param;
  writer->write_end = sprintf_write_end;

  return writer;
}

/**
 * Print a formatted string to the output buffer.
 *
 * Returns number of characters printed
 */
ac_uint ac_sprintf(ac_u8* out_buff, ac_uint out_buff_len,
    const char *format, ...) {
  ac_va_list args;

  if (out_buff == AC_NULL) {
    return 0;
  }
  if (out_buff_len == 0) {
    return 0;
  }
  if (out_buff_len == 1) {
    out_buff[0] = 0;
    return 0;
  }

  ac_writer writer = {
          .count = 0,
          .max_len = out_buff_len,
          .data = out_buff,
          .get_buff = sprintf_get_buff,
          .write_beg = sprintf_write_beg,
          .write_param = sprintf_write_param,
          .write_end = sprintf_write_end,
  };

  ac_va_start(args, format);
  formatter(&writer, format, args);
  ac_va_end(args);
  return writer.count;
}

/**
 * Register a format processor for ch
 *
 * @param fmt_proc is the format processing function
 * @param ch is the format character which causes fn to be invoked.
 *
 * @return 0 if registered successfully
 */
ac_uint ac_printf_register_format_proc(ac_printf_format_proc format_proc, ac_u8 ch) {
  ac_bool ret_val;
  ac_uint idx = __atomic_fetch_add(&ac_printf_ts_count, 1, __ATOMIC_ACQUIRE);
  if (idx < ac_printf_ts_max) {
    ac_printf_ts[idx].format_proc = format_proc;
    ac_printf_ts[idx].ch = ch;
    ret_val = 0; // OK
  } else {
    __atomic_store_n(&ac_printf_ts_count, ac_printf_ts_max, __ATOMIC_RELEASE);
    ret_val = 1; // Error
  }
  return ret_val;
}
