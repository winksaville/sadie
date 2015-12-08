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

#include <ac_cpu_perf.h>

#include <ac_test.h>

ac_bool test_ac_cpu_perf() {
  ac_bool error = AC_FALSE;

#ifdef Posix

  ac_cpu_perf_zero_counters(AC_FALSE);

  ac_u64 cc1 = ac_cpu_perf_cycle_counter_rd();
  ac_u64 cc2 = ac_cpu_perf_cycle_counter_rd();
  ac_u64 diff = cc2 - cc1;
  AC_TEST(cc1 != 0);
  AC_TEST(cc2 != 0);
  AC_TEST(diff > 0);
  ac_printf("test_ac_cpu_perf: cc1=%llx cc2=%llx diff=%llx\n",
      cc1, cc2, diff);

  ac_u32 freq = ac_cpu_perf_cycle_counter_freq();
  AC_TEST(freq >= 1000000);
  ac_printf("test_ac_cpu_perf: freq=%d\n", freq);

#elif defined(VersatilePB)

  ac_printf("test_ac_cpu_perf: VersatilePB no tests on qemu"
      " as Cycle Counter is not available\n");

#else

  ac_printf("test_ac_cpu_perf: Unknown Platform\n");

#endif

  return error;
}

int main(void) {
  if (test_ac_cpu_perf()) {
      // Failed
      ac_printf("ERR\n");
      return 1;
  } else {
      // Succeeded
      ac_printf("OK\n");
      return 0;
  }
}

