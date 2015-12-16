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

/**
 * From [here](http://stackoverflow.com/questions/3247373/how-to-measure-program-execution-time-in-arm-cortex-a8-processor)
 */

#include <ac_cpu_perf.h>

#include <ac_inttypes.h>

static ac_u32 divider;
static ac_u32 freq;
static ac_u32 overflow;

/**
 * Initialize module
 */
void ac_cpu_perf_init(void) {
  // Allow user mode to access
  __asm__ volatile ("MCR p15, 0, %0, C9, C14, 0\n\t" :: "r"(1));

  // For now disable counter overflow
  __asm__ volatile ("MCR p15, 0, %0, C9, C14, 2\n\t" :: "r"(0x8000000f));

  ac_cpu_perf_zero_counters(AC_FALSE);
}

/**
 * Zero counters and enable divider
 */
void ac_cpu_perf_zero_counters(ac_bool enable_divider) {
  ac_u32 value = 0x7; // Enable all counters and reset the to 0
  if (enable_divider) {
    divider = 64;
    value |= 0x8; // Divide by 64
  } else {
    divider = 1;
  }
  freq = 1000000;
  overflow = 0;
  __asm__ volatile ("MCR p15, 0, %0, c9, c12, 0\t\n" :: "r"(value));
  __asm__ volatile ("MCR p15, 0, %0, c9, c12, 1\t\n" :: "r"(0x8000000f));
  __asm__ volatile ("MCR p15, 0, %0, c9, c12, 3\t\n" :: "r"(0x8000000f));
}

/**
 * Frequency of ac_cpu_perf_cycle_counter in cycles per second.
 */
ac_u64 ac_cpu_perf_cycle_counter_freq() {
  return (ac_u64)freq / divider;
}

/**
 * Return the current cycle counter value
 */
ac_u64 ac_cpu_perf_cycle_counter_rd() {
  ac_u32 value;
  __asm__ volatile ("MRC p15, 0, %0, c9, c13, 0\t\n" : "=r"(value));  

  return ((ac_u64)overflow << 32) | value;
}
