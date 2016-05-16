/*
 * copyright 2015 wink saville
 *
 * licensed under the apache license, version 2.0 (the "license");
 * you may not use this file except in compliance with the license.
 * you may obtain a copy of the license at
 *
 *     http://www.apache.org/licenses/license-2.0
 *
 * unless required by applicable law or agreed to in writing, software
 * distributed under the license is distributed on an "as is" basis,
 * without warranties or conditions of any kind, either express or implied.
 * see the license for the specific language governing permissions and
 * limitations under the license.
 */

#include <ac_tsc.h>

#include <ac_inttypes.h>
#include <ac_printf.h>

#define NDEBUG
#include <ac_debug_printf.h>

// Specific at least 199309L for nanosleep
#define _POSIX_C_SOURCE 199506L
#include <time.h>

static ac_u64 freq;

/**
 * Frequency of ac_cpu_perf_cycle_counter in cycles per second.
 */
ac_u64 ac_tsc_freq() {
  return freq;
}

/**
 * Set the frequency that will be returned by ac_tsc_freq.
 * This is primiarly used for testing as freq us usually
 * initialized by ac_tsc_init.
 */
void ac_tsc_set_freq(ac_u64 f) {
  freq = f;
}

/**
 * Initialize module
 *
 */
__attribute__((__constructor__)) // Automatically called before main
void ac_tsc_init(void) {
  ac_debug_printf("ac_tsc_init: pit_freq=%d cal_time=1/%dsec count_rate=%d(0x%x)\n",
      pit_freq, cal_time, count_rate, count_rate);

  // We'll wait 1/20 of a second
  ac_u64 cal_time = 20;
  const struct timespec req = { .tv_sec = 0, .tv_nsec = 1000000000 / cal_time };

  // First reading of tsc
  ac_u64 first = ac_tscrd();

  // Wait for 1/cal_time seconds
  nanosleep(&req, AC_NULL);

  // Second reading of tsc
  ac_u64 second = ac_tscrd();

  // Calculate the frequency of tsc
  ac_u64 diff_ticks = second - first;
  freq = diff_ticks * cal_time;
  ac_printf("ac_tsc_init: freq=%ld cal_time=1/%dsec\n",
      freq, cal_time);
}
