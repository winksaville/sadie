/*
 * Copyright 2016 Wink Saville
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

#include <ac_printf.h>
#include <ac_debug_printf.h>
#include <ac_string.h>
#include <ac_test.h>
#include <ac_time.h>
#include <ac_tsc.h>

ac_bool test_time(ac_u64 test_freq) {
  ac_u64 save_f = ac_tsc_freq();
  ac_tsc_set_freq(test_freq);

  ac_u64 f = ac_tsc_freq();
  ac_bool error = AC_FALSE;
  ac_u8 buff[64];
  ac_u64 ticks;

  ac_printf("test_time: f=%ld save_f=%ld\n", f, save_f);

  ac_snprintf(buff, AC_ARRAY_COUNT(buff), "%t", f + (f / 10));
  ac_printf("buff=%s\n", buff);
  error |= AC_TEST(ac_strncmp("1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_snprintf(buff, AC_ARRAY_COUNT(buff), "%.1t", f + (f / 10));
  ac_printf("buff=%s\n", buff);
  error |= AC_TEST(ac_strncmp("1.1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_snprintf(buff, AC_ARRAY_COUNT(buff), "%0.1t", f + (f / 10));
  ac_printf("buff=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = 0;
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 0, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(0, 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m0s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(0, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m0.0s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 2, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(0, 2)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m0.00s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 3, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(0, 3)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m0.000s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 8, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(0, 8)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m0.00000000s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 9, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(0, 9)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m0.000000000s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 10, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(0, 10)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m0.0000000000s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = f / 10;
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 0, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(f/10, 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m0s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(f/10, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m0.1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 2, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(f/10, 2)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m0.10s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 9, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(f/10, 9)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m0.100000000s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = (f / 2) - 1;
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 0, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str((f/2)-1, 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m0s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str((f/2)-1, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m0.5s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = f / 2;
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 0, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(f/2, 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(f/2, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m0.5s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = f;
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 0, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(f, 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(f, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.0s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = f + (f / 10);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 0, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(f + (f/10), 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(f + (f/10), 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 2, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(f + (f/10), 2)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.10s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = f + (f / 2) - 1;
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 0, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(f + (f/2)-1, 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(f + (f/2)-1, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.5s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 2, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(f + (f/2)-1, 2)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.50s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 9, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(f + (f/2)-1, 9)=%s\n", buff);
  error |= AC_TEST((ac_strncmp("0y0d0h0m1.500000000", (char*)buff, 19) == 0)
               | (ac_strncmp("0y0d0h0m1.499999999", (char*)buff, 19) == 0));
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 10, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(f + (f/2)-1, 10)=%s\n", buff);
  error |= AC_TEST((ac_strncmp("0y0d0h0m1.500000000", (char*)buff, 19) == 0)
               | (ac_strncmp("0y0d0h0m1.499999999", (char*)buff, 19) == 0));
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 11, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(f + (f/2)-1, 11)=%s\n", buff);
  error |= AC_TEST((ac_strncmp("0y0d0h0m1.500000000", (char*)buff, 19) == 0)
               | (ac_strncmp("0y0d0h0m1.499999999", (char*)buff, 19) == 0));


  ticks = f + (f / 2);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 0, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(f + (f/2), 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m2s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(f + (f/2), 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.5s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 2, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(f + (f/2), 2)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.50s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 9, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(f + (f/2), 9)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.500000000s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 10, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(f + (f/2), 10)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.5000000000s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  // Test Years, Days, Hours, Seconds with LEADING_0 and NO_LEADING_0
  ticks = AC_SECS_PER_YEAR * f;
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(1y, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("1y0d0h0m0.0s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = (AC_SECS_PER_DAY * f) + (AC_SECS_PER_MIN * f);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(1d0h1m, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y1d0h1m0.0s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_duration_str(ticks, NO_LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(1d0h1m, NO_LEADING_0, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("1d0h1m0.0s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = (AC_SECS_PER_MIN * f) + (2 * f) + (f / 10);
  AcTime_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(1m2.1s, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h1m2.1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_duration_str(ticks, NO_LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_duration_str(1m2.1s, NO_LEADING_0, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("1m2.1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  // Restore freq
  ac_tsc_set_freq(save_f);
  return error;
}

ac_bool test_seconds(ac_u64 test_freq) {
  ac_u64 save_f = ac_tsc_freq();
  ac_tsc_set_freq(test_freq);

  ac_u64 f = ac_tsc_freq();
  ac_bool error = AC_FALSE;
  ac_u8 buff[64];
  ac_u64 ticks;

  ac_printf("test_seconds: f=%ld save_f=%ld\n", f, save_f);

  ac_snprintf(buff, AC_ARRAY_COUNT(buff), "%Ss", f + (f / 10));
  ac_printf("buff=%s\n", buff);
  error |= AC_TEST(ac_strncmp("1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_snprintf(buff, AC_ARRAY_COUNT(buff), "%.1Ss", f + (f / 10));
  ac_printf("buff=%s\n", buff);
  error |= AC_TEST(ac_strncmp("1.1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_snprintf(buff, AC_ARRAY_COUNT(buff), "%.1Ss", (f / 10));
  ac_printf("buff=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0.1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_snprintf(buff, AC_ARRAY_COUNT(buff), "%0.1Ss", (f / 10));
  ac_printf("buff=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0.1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = 0;
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 0, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(0, 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 1, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(0, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0.0s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 2, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(0, 2)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0.00s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 3, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(0, 3)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0.000s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 8, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(0, 8)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0.00000000s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 9, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(0, 9)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0.000000000s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 10, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(0, 10)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0.0000000000s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = f / 10;
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 0, AC_NULL, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(f/10, 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 1, AC_NULL, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(f/10, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0.1", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 2, AC_NULL, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(f/10, 2)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0.10", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 9, AC_NULL, buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(f/10, 9)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0.100000000", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = (f / 2) - 1;
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 0, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str((f/2)-1, 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 1, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str((f/2)-1, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0.5s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = f / 2;
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 0, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(f/2, 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 1, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(f/2, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0.5s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = f;
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 0, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(f, 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 1, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(f, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("1.0s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = f + (f / 10);
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 0, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(f + (f/10), 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 1, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(f + (f/10), 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("1.1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 2, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(f + (f/10), 2)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("1.10s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = f + (f / 2) - 1;
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 0, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(f + (f/2)-1, 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 1, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(f + (f/2)-1, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("1.5s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 2, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(f + (f/2)-1, 2)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("1.50s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 9, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(f + (f/2)-1, 9)=%s\n", buff);
  error |= AC_TEST((ac_strncmp("1.500000000", (char*)buff, 11) == 0)
               | (ac_strncmp("1.499999999", (char*)buff, 11) == 0));
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 10, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(f + (f/2)-1, 10)=%s\n", buff);
  error |= AC_TEST((ac_strncmp("1.500000000", (char*)buff, 11) == 0)
               | (ac_strncmp("1.499999999", (char*)buff, 11) == 0));
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 11, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(f + (f/2)-1, 11)=%s\n", buff);
  error |= AC_TEST((ac_strncmp("1.500000000", (char*)buff, 11) == 0)
               | (ac_strncmp("1.499999999", (char*)buff, 11) == 0));


  ticks = f + (f / 2);
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 0, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(f + (f/2), 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("2s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 1, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(f + (f/2), 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("1.5s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 2, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(f + (f/2), 2)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("1.50s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 9, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(f + (f/2), 9)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("1.500000000s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  AcTime_ticks_to_seconds_str(ticks, LEADING_0, 10, "s", buff, AC_ARRAY_COUNT(buff));
  ac_printf("AcTime_ticks_to_seconds_str(f + (f/2), 10)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("1.5000000000s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  // Restore freq
  ac_tsc_set_freq(save_f);
  return error;
}


int main(void) {
  ac_bool error = AC_FALSE;

  ac_u64 freq = ac_tsc_freq();
  ac_u64 test_nanos = AcTime_ticks_to_nanos(freq);
  ac_printf("test_nanos=%ld AC_SEC_IN_NS=%ld freq=%ld\n",
      test_nanos, AC_SEC_IN_NS, freq);
  error |= AC_TEST(test_nanos == AC_SEC_IN_NS);

  ac_u64 test_ticks = AcTime_nanos_to_ticks(2 * AC_SEC_IN_NS);
  ac_u64 expected_ticks = 2 * freq;
  ac_printf("test_ticks=%ld expected_ticks=%ld freq=%ld\n",
      test_ticks, expected_ticks, freq);
  error |= AC_TEST(test_ticks == expected_ticks);

  test_ticks = AcTime_nanos_to_ticks(AC_SEC_IN_NS / 100);
  expected_ticks = AC_U64_DIV_ROUND_UP(freq, 100);
  ac_printf("test_ticks=%ld expected_ticks=%ld freq=%ld\n",
      test_ticks, expected_ticks, freq);
  error |= AC_TEST(test_ticks == expected_ticks);

  // For testing we'll assume a 3.3GHz clock
  // This particular value had shown a bug but
  // potentially any value >= 1GHz should work.
  // 1GHz is minimum because we're testing
  // 9 digits of precision.
  error |= test_time(3303993540ll);
  error |= test_time(2000000000ll);
  error |= test_time(1000000000ll);

  error |= test_seconds(1000000000ll);

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
