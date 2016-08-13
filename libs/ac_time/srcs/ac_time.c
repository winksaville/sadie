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

#include <ac_time.h>

#include <ac_arg.h>
#include <ac_assert.h>
#include <ac_inttypes.h>
#include <ac_intmath.h>
#include <ac_string.h>
#include <ac_tsc.h>
#include <ac_printf.h>
#include <ac_debug_printf.h>

/**
 * Convert nanos seconds to ticks
 *
 * @param: nanos
 *
 * @return: number of ticks
 */
ac_u64 AcTime_nanos_to_ticks(ac_u64 nanos) {
  ac_u64 secs = nanos / AC_SEC_IN_NS;
  ac_u64 sub_secs = nanos % AC_SEC_IN_NS;

  return (secs * ac_tsc_freq()) +
    AC_U64_DIV_ROUND_UP(sub_secs * ac_tsc_freq(), AC_SEC_IN_NS);
}

/**
 * Convert ticks to nano seconds
 *
 * @param: ticks
 *
 * @return: number of nano seconds
 */
ac_u64 AcTime_ticks_to_nanos(ac_u64 ticks) {
  ac_u64 freq = ac_tsc_freq();
  ac_u64 secs = ticks / freq;
  ac_u64 sub_sec_ticks = ticks % freq;
  return (secs * AC_SEC_IN_NS) +
    AC_U64_DIV_ROUND_UP(sub_sec_ticks * AC_SEC_IN_NS, freq);
}

static ac_u64 round_ticks_based_on_precision(ac_u64 ticks, ac_u64 freq, ac_uint precision) {
  // Round ticks up by freq / (2 * ((precision + 1) * 10))
  //   precison = 0 up = freq / 2
  //   precison = 1 up = freq / 20
  //   precison = 2 up = freq / 200
  ac_u64 up = 2;
  for (ac_uint i = 0; i < precision; i++) {
    up *= 10;
  }
  ac_u64 rounded_freq = (up <= freq) ? (AC_U64_DIV_ROUND_UP(freq, up)) : 0;
  ac_u64 rounded = ticks + rounded_freq;
  ac_debug_printf("ticks=%lu precision=%d freq=%ld up=%lu rounded_freq=%lu rounded=%lu\n",
      ticks, precision, freq, up, rounded_freq, rounded);
  return rounded;
}

/**
 * Convert sub second ticks to string.
 * Note sss must be >= precision in length of sss!!!
 */
static void sub_sec_to_str(ac_u64 sub_sec_ticks, ac_u64 freq, ac_uint precision, ac_u8* sss) {
  ac_uint i;
  if (precision > 0) {
    ac_debug_printf(" freq=%ld precision=%ld\n", freq, precision);
    // Peform "long" division one decimal digit at a time.
    for (i = 0; (sub_sec_ticks > 0) && (i < precision); i++) {
      ac_debug_printf("sub_sec_ticks=%ld", sub_sec_ticks);
      sub_sec_ticks *= 10;
      ac_debug_printf(" sub_sec_ticks*10=%ld", sub_sec_ticks);
      ac_u8 digit = sub_sec_ticks / freq;
      ac_debug_printf(" digit=%ld digit*freq=%ld", digit, digit * freq);
      sss[i] = '0' + digit;
      ac_debug_printf(" sss[%d]=%c\n",  i, sss[i]);
      sub_sec_ticks -= (digit * freq);
    }
    for (; (i < precision); i++) {
      sss[i] = '0';
      ac_debug_printf("precision=%d sss[%d]=%c\n", precision, i, sss[i]);
    }
    sss[i] = 0;
  }
}

/**
 * Convert ticks to a duration string of the form XyXdXhXmX.Ys
 * So 3 years 5 days 1 hour 7 minutes 3.201 seconds is
 * printed as 3y5d1h7m3.201s
 *
 * @param: ticks is value in ac_tsc_freq
 * @param: leading_0 is true if leading zero values are to be printed
 * @param: precision is number of digits after seconds decimal place
 * @param: out_buf is output buffer
 * @param: out_buf_len is the length of the output buffer
 *
 * @return number of characters written to out_buf not including trailing 0
 *         if out_buf_len == 0 nothing is written
 *         if out_buf_len == 1 only a trailing zero is written
 */
ac_uint AcTime_ticks_to_duration_str(ac_u64 ticks, ac_bool leading_0, ac_uint precision,
    ac_u8* out_buff, ac_uint out_buff_len) {
  ac_u64 freq = ac_tsc_freq();
  ac_u8 sss[32];

  if (precision >= (AC_ARRAY_COUNT(sss) - 1)) {
    precision = AC_ARRAY_COUNT(sss) - 1;
  }

  ticks = round_ticks_based_on_precision(ticks, freq, precision);

  ac_u64 secs = ticks / freq;
  ac_u64 sub_sec_ticks = ticks % freq;
  ac_u64 mins = 0;
  ac_u64 hrs = 0;
  ac_u64 days = 0;
  ac_u64 years = 0;

  ac_debug_printf("freq=%lu precison=%d secs=%lu sub_sec_ticks=%lu\n",
      freq, precision, secs, sub_sec_ticks);
  if (secs >= AC_SECS_PER_YEAR) {
    years = secs / AC_SECS_PER_YEAR;
    secs = secs % AC_SECS_PER_YEAR;
  }
  if (secs >= AC_SECS_PER_DAY) {
    days = secs / AC_SECS_PER_DAY;
    secs = secs % AC_SECS_PER_DAY;
  }
  if (secs >= AC_SECS_PER_HR) {
    hrs = secs / AC_SECS_PER_HR;
    secs = secs % AC_SECS_PER_HR;
  }
  if (secs >= AC_SECS_PER_MIN) {
    mins = secs / AC_SECS_PER_MIN;
    secs = secs % AC_SECS_PER_MIN;
  }
  ac_debug_printf("years=%lu days=%lu hrs=%lu mins=%lu secs=%lu sub_sec_ticks=%lu\n",
      years, days, hrs, mins, secs, sub_sec_ticks);

  // Conditionally printng years, days, hrs, mins
  ac_uint count = 0;
  ac_bool printing = leading_0;
  if (printing || (years != 0)) {
    count += ac_sprintf(out_buff, out_buff_len, "%ldy", years);
    printing = AC_TRUE;
  }
  if (printing || (days != 0)) {
    count += ac_sprintf(&out_buff[count], out_buff_len - count, "%ldd", days);
    printing = AC_TRUE;
  }
  if (printing || (hrs != 0)) {
    count += ac_sprintf(&out_buff[count], out_buff_len - count, "%ldh", hrs);
    printing = AC_TRUE;
  }
  if (printing || (mins != 0)) {
    count += ac_sprintf(&out_buff[count], out_buff_len - count, "%ldm", mins);
    printing = AC_TRUE;
  }

  // Always print seconds
  count += ac_sprintf(&out_buff[count], out_buff_len - count, "%ld", secs);
  if (precision > 0) {
    sub_sec_to_str(sub_sec_ticks, freq, precision, sss);

    // Print specified number of sub seconds
    count += ac_sprintf(&out_buff[count], out_buff_len - count, ".%s", sss);
  }
  count += ac_sprintf(&out_buff[count], out_buff_len - count, "s");

  return count;
}

/**
 * Add 't' as a ac_printf format character for time.
 */
static ac_u32 ac_printf_time_format_proc(ac_writer* writer, ac_u8 ch, ac_va_list args) {
  ac_u8 buff[64];

  AcTime_ticks_to_duration_str(ac_va_arg(args, ac_u64), writer->leading_0,
        writer->precision, buff, AC_ARRAY_COUNT(buff));
  ac_printf_write_str(writer, (char*)buff);
  return sizeof(ac_u64) / sizeof(ac_uint);
}

/**
 * Convert ticks to 'seconds' float string as whole.fraction
 *
 * @param: ticks is value in ac_tsc_freq
 * @param: leading_0 is true if leading zero values are to be printed
 * @param: precision is number of digits after seconds decimal place
 * @param: out_buf is output buffer
 * @param: out_buf_len is the length of the output buffer
 *
 * @return number of characters written to out_buf not including trailing 0
 *         if out_buf_len == 0 nothing is written
 *         if out_buf_len == 1 only a trailing zero is written
 */
ac_uint AcTime_ticks_to_seconds_str(ac_u64 ticks, ac_bool leading_0, ac_uint precision,
    char* units_str, ac_u8* out_buff, ac_uint out_buff_len) {
  ac_u64 freq = ac_tsc_freq();
  ac_u8 sss[32];

  if (precision >= (AC_ARRAY_COUNT(sss) - 1)) {
    precision = AC_ARRAY_COUNT(sss) - 1;
  }

  ticks = round_ticks_based_on_precision(ticks, freq, precision);

  ac_u64 secs = ticks / freq;
  ac_u64 sub_sec_ticks = ticks % freq;

  // Always print seconds
  ac_u64 count = ac_sprintf(&out_buff[0], out_buff_len, "%lu", secs);
  if (precision > 0) {
    sub_sec_to_str(sub_sec_ticks, freq, precision, sss);

    // Print specified number of sub seconds
    count += ac_sprintf(&out_buff[count], out_buff_len - count, ".%s", sss);
  }
  if (units_str != AC_NULL) {
    count += ac_sprintf(&out_buff[count], out_buff_len - count, "%s", units_str);
  }

  return count;
}

/**
 * Add 'S' as a ac_printf format character for time.
 */
static ac_u32 ac_printf_seconds_format_proc(ac_writer* writer, ac_u8 ch, ac_va_list args) {
  ac_u8 buff[64];

  AcTime_ticks_to_seconds_str(ac_va_arg(args, ac_u64), writer->leading_0,
        writer->precision, "", buff, AC_ARRAY_COUNT(buff));
  ac_printf_write_str(writer, (char*)buff);
  return sizeof(ac_u64) / sizeof(ac_uint);
}

static ac_bool early_init = AC_FALSE;

/**
 * Initialize ac_time early.
 */
__attribute__((constructor))
void AcTime_early_init(void) {
  ac_uint error = ac_printf_register_format_proc(ac_printf_time_format_proc, 't');
  ac_assert(error == 0);
  error = ac_printf_register_format_proc(ac_printf_seconds_format_proc, 'S');
  ac_assert(error == 0);
  early_init = AC_TRUE;
  ac_printf("AcTime_early_init:-\n");
}

/**
 * Initialize ac_time.
 */
void AcTime_init(void) {
  if (!early_init) {
    AcTime_early_init();
  }
}
