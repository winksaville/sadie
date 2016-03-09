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

#include <ac_inttypes.h>

#include <ac_printf.h>
#include <ac_test.h>

ac_u64 empty_loop(ac_u64 loops) {
  ac_u64 tsc0;
  ac_u64 tsc1;

  tsc0 = ac_cpu_perf_cycle_counter_rd();
  do {
    __asm__ volatile("":::"memory");
  } while (--loops > 0);
  tsc1 = ac_cpu_perf_cycle_counter_rd();

  ac_printf("empty_loop:\n");
  ac_printf(" tsc0=%ld\n", tsc0);
  ac_printf(" tsc1=%ld\n", tsc1);
  ac_printf(" diff=%ld\n", tsc1 - tsc0);

  return tsc1 - tsc0;
}

ac_u64 counter_loop(ac_u64 loops) {
  ac_u64 tsc0;
  ac_u64 tsc1;

  tsc0 = ac_cpu_perf_cycle_counter_rd();
  do {
    __asm__ volatile("":::"memory");
    ac_cpu_perf_cycle_counter_rd();
  } while (--loops > 0);
  tsc1 = ac_cpu_perf_cycle_counter_rd();

  ac_printf("counter_loop:\n");
  ac_printf(" tsc0=%ld\n", tsc0);
  ac_printf(" tsc1=%ld\n", tsc1);
  ac_printf(" diff=%ld\n", tsc1 - tsc0);

  return tsc1 - tsc0;
}

ac_u64 rdtscp_loop(ac_u64 loops) {
  ac_u64 tsc0;
  ac_u64 tsc1;
  ac_u32 aux;

  tsc0 = ac_cpu_perf_cycle_counter_rd();
  do {
    __asm__ volatile("":::"memory");
    cpu_perf_x86_rdtscp(&aux);
  } while (--loops > 0);
  tsc1 = ac_cpu_perf_cycle_counter_rd();

  ac_printf("rdtscp_loop:\n");
  ac_printf(" aux =%ld\n", aux);
  ac_printf(" tsc0=%ld\n", tsc0);
  ac_printf(" tsc1=%ld\n", tsc1);
  ac_printf(" diff=%ld\n", tsc1 - tsc0);

  return tsc1 - tsc0;
}

ac_bool test_tsc(void) {
  ac_u64 tsc0;
  ac_u64 tsc1;
  ac_bool error = AC_FALSE;

  tsc0 = ac_cpu_perf_cycle_counter_rd();
  tsc1 = ac_cpu_perf_cycle_counter_rd();

  ac_printf("ac_cpu_perf_cycle_counter_rd:\n");
  ac_printf(" tsc0=%ld\n", tsc0);
  ac_printf(" tsc1=%ld\n", tsc1);
  ac_printf(" diff=%ld\n", tsc1 - tsc0);

  error |= AC_TEST(tsc0 != 0);
  error |= AC_TEST(tsc0 != tsc1);

  tsc0 = cpu_perf_x86_rdtsc();
  tsc1 = cpu_perf_x86_rdtsc();

  ac_printf("cpu_perf_x86_rdtsc:\n");
  ac_printf(" tsc0=%ld\n", tsc0);
  ac_printf(" tsc1=%ld\n", tsc1);
  ac_printf(" diff=%ld\n", tsc1 - tsc0);

  error |= AC_TEST(tsc0 != 0);
  error |= AC_TEST(tsc0 != tsc1);

  return error;
}

void run_timings(void) {
  volatile ac_u64 loops = 1000000000;

  ac_u64 empty_lp = empty_loop(loops);
  ac_u64 counter_lp = counter_loop(loops);
  ac_u64 diff = counter_lp - empty_lp;
  ac_u64 div = diff / loops;
  ac_u64 mod = diff % loops;
  
  ac_printf("run_timings: loops=%ld\n", loops);
  ac_printf(" empty_lp=%ld\n", empty_lp);
  ac_printf(" counter_lp=%ld\n", counter_lp);
  ac_printf(" diff=%ld\n", diff);
  ac_printf(" div=%ld\n", div);
  ac_printf(" mod=%ld\n", mod);
  ac_printf(" ticks per operation (diff + mod) / loops (tpo)=%ld\n",
      (diff + mod) / loops);

  ac_u64 rdtscp_lp = rdtscp_loop(loops);
  diff = rdtscp_lp - empty_lp;
  div = diff / loops;
  mod = diff % loops;

  ac_printf(" rdtscp_lp=%ld\n", counter_lp);
  ac_printf(" diff=%ld\n", diff);
  ac_printf(" div=%ld\n", div);
  ac_printf(" mod=%ld\n", mod);
  ac_printf(" ticks per operation (diff + mod) / loops (tpo)=%ld\n",
      (diff + mod) / loops);
}

int main(void) {
  ac_bool error = AC_FALSE;

  if (!error) {
    error |= test_tsc();
  }

  run_timings();

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
