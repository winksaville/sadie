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

#include <ac_tsc.h>

#include <ac_inttypes.h>

#include <ac_printf.h>
#include <ac_test.h>

ac_u64 empty_loop(ac_u64 loops) {
  ac_u64 tsc0;
  ac_u64 tsc1;

  tsc0 = ac_tscrd();
  do {
    __asm__ volatile("":::"memory");
  } while (--loops > 0);
  tsc1 = ac_tscrd();

  ac_printf("empty_loop:\n");
  ac_printf(" tsc0 = %ld\n", tsc0);
  ac_printf(" tsc1 = %ld\n", tsc1);
  ac_printf(" diff_ticks = %ld\n", tsc1 - tsc0);

  return tsc1 - tsc0;
}

ac_u64 rdtsc_loop(ac_u64 loops) {
  ac_u64 tsc0;
  ac_u64 tsc1;

  tsc0 = ac_tscrd();
  do {
    __asm__ volatile("":::"memory");
    ac_tscrd();
  } while (--loops > 0);
  tsc1 = ac_tscrd();

  ac_printf("rdtsc_loop:\n");
  ac_printf(" tsc0 = %ld\n", tsc0);
  ac_printf(" tsc1 = %ld\n", tsc1);
  ac_printf(" diff_ticks = %ld\n", tsc1 - tsc0);

  return tsc1 - tsc0;
}

ac_u64 rdtscp_loop(ac_u64 loops) {
  ac_u64 tsc0;
  ac_u64 tsc1;
  ac_u32 aux;

  tsc0 = ac_tscrd();
  do {
    __asm__ volatile("":::"memory");
    ac_tscrd_aux(&aux);
  } while (--loops > 0);
  tsc1 = ac_tscrd();

  ac_printf("rdtscp_loop:\n");
  ac_printf(" aux = 0x%x\n", aux);
  ac_printf(" tsc0 = %ld\n", tsc0);
  ac_printf(" tsc1 = %ld\n", tsc1);
  ac_printf(" diff_ticks = %ld\n", tsc1 - tsc0);

  return tsc1 - tsc0;
}

ac_bool test_tsc(void) {
  ac_u64 tsc0;
  ac_u64 tsc1;
  ac_bool error = AC_FALSE;

  tsc0 = ac_tscrd();
  tsc1 = ac_tscrd();

  ac_printf("ac_tscrd:\n");
  ac_printf(" tsc0 = %ld\n", tsc0);
  ac_printf(" tsc1 = %ld\n", tsc1);
  ac_printf(" diff_ticks = %ld\n", tsc1 - tsc0);

  error |= AC_TEST(tsc0 != 0);
  error |= AC_TEST(tsc0 != tsc1);

  tsc0 = ac_tscrd();
  tsc1 = ac_tscrd();

  ac_printf("ac_tscrd:\n");
  ac_printf(" tsc0 = %ld\n", tsc0);
  ac_printf(" tsc1 = %ld\n", tsc1);
  ac_printf(" diff_ticks = %ld\n", tsc1 - tsc0);

  error |= AC_TEST(tsc0 != 0);
  error |= AC_TEST(tsc0 != tsc1);

  return error;
}

void run_timings(void) {
  volatile ac_u64 loops = 10000000;

  ac_u64 empty_ticks = empty_loop(loops);
  ac_u64 rdtsc_ticks = rdtsc_loop(loops);
  ac_u64 diff_ticks = rdtsc_ticks - empty_ticks;
  ac_u64 div_ticks = diff_ticks / loops;
  ac_u64 mod_ticks = diff_ticks % loops;
  
  ac_printf("run_timings: loops = %ld\n", loops);
  ac_printf(" empty_ticks = %ld\n", empty_ticks);
  ac_printf(" rdtsc_ticks = %ld\n", rdtsc_ticks);
  ac_printf(" diff_ticks = %ld\n", diff_ticks);
  ac_printf(" div_ticks = %ld\n", div_ticks);
  ac_printf(" mod_ticks = %ld\n", mod_ticks);
  ac_printf(" ticks per operation (diff_ticks + mod_ticks) / loops (tpo) = %ld\n",
      (diff_ticks + mod_ticks) / loops);

  ac_u64 rdtscp_ticks = rdtscp_loop(loops);
  diff_ticks = rdtscp_ticks - empty_ticks;
  div_ticks = diff_ticks / loops;
  mod_ticks = diff_ticks % loops;

  ac_printf(" rdtscp_ticks = %ld\n", rdtscp_ticks);
  ac_printf(" diff_ticks = %ld\n", diff_ticks);
  ac_printf(" div_ticks = %ld\n", div_ticks);
  ac_printf(" mod_ticks = %ld\n", mod_ticks);
  ac_printf(" ticks per operation (diff_ticks + mod_ticks) / loops (tpo) = %ld\n",
      (diff_ticks + mod_ticks) / loops);
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
