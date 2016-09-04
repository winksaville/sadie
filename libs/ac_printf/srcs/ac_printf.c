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
  ac_printf_format_proc_str format_proc_str;
  const char* str;
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
    AC_UNUSED(this);
    ac_putchar((ac_u8)(((ac_uptr)param) & 0xff));
    this->count += 1;
}

/**
 *  Write a character
 */
void ac_printf_write_char(ac_writer* writer, char ch) {
    writer->write_param(writer, cast_to_write_param(ch));
}

/**
 * Output a string
 */
static void write_str(ac_writer *writer, char *str) {
    char ch;

    while((ch = *str++)) {
        writer->write_param(writer, cast_to_write_param(ch));
    }
}

/**
 * Write a string
 */
void ac_printf_write_str(ac_writer* writer, char* str) {
  write_str(writer, str);
}

static void write_spaces(ac_writer* writer, ac_uint count) {
  while (count-- > 0) {
      writer->write_param(writer, cast_to_write_param(' '));
  }
}

/**
 * Output an unsigned int value
 */
static void write_uval(
        ac_writer* writer, ac_u64 val, ac_uint sz_val_in_bytes, ac_uint radix) {
    static const char val_to_char[] = "0123456789abcdef";
    char result[65];

    // Validate radix
    if ((radix <= 1) || (radix > sizeof(val_to_char))) {
        write_str(writer, "Bad Radix ");
        write_uval(writer, sz_val_in_bytes, radix, 10);
    } else {
        ac_sint idx = 0;
        while (idx < sizeof(result)) {
            result[idx++] = val_to_char[val % radix];
            val /= radix;
            if (val == 0) {
                break;
            }
        }

        // Zero padding
        ac_sint pad0count = 0;
        if (writer->leading_0) {
          // Fill upto width with 0
          pad0count = writer->min_width - idx;
          if (writer->sign_to_print != 0) {
            // Save room to print sign
            pad0count -= 1;
          }
        }
        while (pad0count-- > 0) {
            result[idx++] = '0';
        }

        // Print sign
        if (writer->sign_to_print != 0) {
          result[idx++] = writer->sign_to_print;
        }

        // Compute number of spaces needed if any
        ac_int padspaces = writer->min_width - idx;

        // If right justified then write sapces first
        if ((padspaces > 0) && (writer->left_justified == AC_FALSE)) {
          write_spaces(writer, padspaces);
        }

        // print the result in reverse order
        while (--idx >= 0) {
          writer->write_param(writer, cast_to_write_param(result[idx]));
        }

        // If left justified then write sapces last
        if ((padspaces > 0) && (writer->left_justified == AC_TRUE)) {
          write_spaces(writer, padspaces);
        }
    }
}

/**
 * Write an unsigned value
 */
void ac_printf_write_uval(
        ac_writer* writer, ac_u64 val, ac_uint sz_val_in_bytes, ac_uint radix) {
  write_uval(writer, val, sz_val_in_bytes, radix);
}

/**
 * Output an signed int val
 */
static void write_sval(
        ac_writer* writer, ac_s64 val, ac_uint sz_val_in_bytes, ac_uint radix) {
    if (val < 0) {
      writer->sign_to_print = '-';
      val = -val;
    }
    write_uval(writer, val, sizeof(ac_uint), radix);
}

/**
 * Output a chacter
 */
static void write_ch(ac_writer* writer, ac_u8 ch) {
  writer->write_param(writer, cast_to_write_param(ch));
}

/**
 * Write a signed value
 */
void ac_printf_write_sval(
        ac_writer* writer, ac_s64 val, ac_uint sz_val_in_bytes, ac_uint radix) {
    write_sval(writer, val, sz_val_in_bytes, radix);
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
 * Returns number of characters consumed
 */
static void formatter(ac_writer* writer, char const* format, ac_va_list args) {
    // Check inputs
    if (IS_AC_NULL(writer) || IS_AC_NULL(format)) {
        goto done;
    }

    if (writer->write_beg != AC_NULL) {
        writer->write_beg(writer);
    }
    ac_u8 ch;
    while ((ch = *format++) != 0) {
        if (ch != '%') {
            // Not the format escape character
            write_ch(writer, ch);
        } else {
            // Is a '%' so get the next character
            ac_u8 next_ch = *format++;

            // Get Flags
            writer->sign_to_print = 0;
            writer->left_justified = AC_FALSE;
            writer->alt_form = AC_FALSE;
            writer->leading_0 = AC_FALSE;
            ac_bool flags = AC_TRUE;
            while (flags) {
              switch (next_ch) {
                case 0:
                  goto done;
                case '+':
                  writer->sign_to_print = '+'; // Assume plus if sval will be changed to '-' if needed
                  next_ch = *format++;
                  ac_debug_printf("sign_to_print=%c\n", writer->sign_to_print);
                  break;
                case '-':
                  writer->left_justified = AC_TRUE;
                  next_ch = *format++;
                  ac_debug_printf("left_justified=%d\n", writer->left_justified);
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
                    goto done;
                case '%': {
                    // was %% just echo a '%'
                    write_ch(writer, next_ch);
                    break;
                }
                case 'c': {
                    char ch = (char)ac_va_arg(args, ac_uint);
                    if ((ch < 0x20) || (ch > 0x7F)) {
                      ch = 0xff;
                    }
                    write_ch(writer, ch);
                    break;
                }
                case 's': {
                    // Handle string specifier
                    char *s = ac_va_arg(args, char *);
                    write_str(writer, s);
                    break;
                }
                case 'i': {
                    write_sval(writer, ac_va_arg(args, ac_sint), sizeof(ac_sint), 10);
                    break;
                }
                case 'v': {
                    write_uval(writer, ac_va_arg(args, ac_uint), sizeof(ac_uint), 10);
                    break;
                }
                case 'y': {
                    write_uval(writer, ac_va_arg(args, ac_uint), sizeof(ac_uint), 16);
                    break;
                }
                case 'b': {
                    write_uval(writer, ac_va_arg(args, ac_u32), sizeof(ac_u32), 2);
                    break;
                }
                case 'd': {
                    write_sval(writer, ac_va_arg(args, ac_s32), sizeof(ac_s32), 10);
                    break;
                }
                case 'u': {
                    write_uval(writer, ac_va_arg(args, ac_u32), sizeof(ac_u32), 10);
                    break;
                }
                case 'x': {
                    write_uval(writer, ac_va_arg(args, ac_u32), sizeof(ac_u32), 16);
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
                        write_uval(writer, ac_va_arg(args, ac_u64), sizeof(ac_u64), 2);
                    } else if (ac_strncmp("d", format, 1) == 0) {
                        format += 1;
                        write_sval(writer, ac_va_arg(args, ac_s64), sizeof(ac_s64), 10);
                    } else if (ac_strncmp("u", format, 1) == 0) {
                        format += 1;
                        write_uval(writer, ac_va_arg(args, ac_u64), sizeof(ac_u64), 10);
                    } else if (ac_strncmp("x", format, 1) == 0) {
                        format += 1;
                        write_uval(writer, ac_va_arg(args, ac_u64), sizeof(ac_u64), 16);
                    } else {
                        if (longlong) {
                          write_str(writer, "%ll");
                        } else {
                          write_str(writer, "%l");
                        }
                    }
                    break;
                }
                case 'p': {
                    writer->leading_0 = AC_TRUE;
                    if (writer->min_width == 0) {
                      writer->min_width = sizeof(void*) * 2;
                    }
                    write_uval(writer, (ac_u64)(ac_uptr)ac_va_arg(args, void*), sizeof(void*), 16);
                    break;
                }
                case '{': {
                    // Collect a type name
                    char type_name[32];
                    ac_uint idx = 0;
                    ac_bool getting_type_name = AC_TRUE;
                    while (getting_type_name) {
                      next_ch = *format++;
                      switch (next_ch) {
                        case 0: {
                          type_name[idx] = 0;
                          write_ch(writer, '{');
                          write_str(writer, type_name);
                          write_str(writer, "<<'}' is missing>>");
                          goto done;
                        }
                        case '}': {
                          getting_type_name = AC_FALSE;
                          break;
                        }
                        default: {
                          if (idx >= (sizeof(type_name) - 1)) {
                            type_name[idx] = 0;
                            write_ch(writer, '{');
                            write_str(writer, type_name);
                            write_str(writer, "<<to long>>");
                            goto done;
                          } else {
                            type_name[idx++] = next_ch;
                          }
                          break;
                        }
                      }
                    }

                    ac_bool processed_type = AC_FALSE;
                    // Check the ac_printf_t's list
                    for (ac_uint i = 0; i < ac_printf_ts_max; i++) {
                      if ((ac_printf_ts[i].str != AC_NULL)
                          && (ac_strncmp(ac_printf_ts[i].str, type_name, sizeof(type_name)) == 0)) {
                          // Copy the args and pass the copy to format_proc
                          ac_va_list args_copy;
                          ac_va_copy(args_copy, args);

                          // Invoke format_proc_str
                          ac_u32 consumed = ac_printf_ts[i].format_proc_str(
                              writer, type_name, args_copy);

                          // Consume the args
                          while (consumed-- != 0) {
                            ac_va_arg(args, ac_uint);
                          }

                          processed_type = AC_TRUE;
                          break;
                      }
                    }

                    if (!processed_type) {
                        write_ch(writer, '{');
                        write_str(writer, type_name);
                        write_ch(writer, '}');
                    }
                    break;
                }
                default: {
                    // Check the ac_printf_t's list
                    ac_bool processed_format_ch = AC_FALSE;
                    for (ac_uint i = 0; i < ac_printf_ts_max; i++) {
                        if (ac_printf_ts[i].ch == next_ch) {
                            // Copy the args and pass the copy to format_proc
                            ac_va_list args_copy;
                            ac_va_copy(args_copy, args);

                            // Invoke format_proc
                            ac_u32 consumed = ac_printf_ts[i].format_proc(
                                writer, next_ch, args_copy);

                            // Consume the args
                            while (consumed-- != 0) {
                              ac_va_arg(args, ac_uint);
                            }

                            processed_format_ch = AC_TRUE;
                            break;
                        }
                    }

                    if (!processed_format_ch) {
                        write_ch(writer, ch);
                        write_ch(writer, next_ch);
                    }
                    break;
                }
            }
        }
    }
done:
    if (writer->write_end != AC_NULL) {
        writer->write_end(writer);
    }
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
 * Returns the number of characters printed
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
  ac_u32 count = writer->count;
  if (writer->count < (writer->max_len - 1)) {
    ((ac_u8*)(writer->data))[count++] = ((ac_u8)(((ac_uptr)param) & 0xff));
  }
  writer->count = count;
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
ac_uint ac_snprintf(ac_u8* out_buff, ac_uint out_buff_len,
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
 * TODO: Check for ch already registered, but also keep
 * it thread safe so it needs to be serialized.
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

/**
 * Register a format processor for string
 * TODO: Check for str already registered, but also keep
 * it thread safe so it needs to be serialized.
 *
 * @param fmt_proc is the format processing function
 * @param str is the format string which causes fn to be invoked.
 *
 * @return 0 if registered successfully
 */
ac_uint ac_printf_register_format_proc_str(ac_printf_format_proc_str format_proc_str,
    const char* str) {
  ac_bool ret_val;

  ac_uint idx = __atomic_fetch_add(&ac_printf_ts_count, 1, __ATOMIC_ACQUIRE);
  if (idx < ac_printf_ts_max) {
    ac_printf_ts[idx].format_proc_str = format_proc_str;
    ac_printf_ts[idx].str = str;
    ret_val = 0; // OK
  } else {
    __atomic_store_n(&ac_printf_ts_count, ac_printf_ts_max, __ATOMIC_RELEASE);
    ret_val = 1; // Error
  }

  return ret_val;
}
