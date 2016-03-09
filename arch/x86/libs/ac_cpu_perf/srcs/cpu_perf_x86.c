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

/**
 * Initialize module
 */
void ac_cpu_perf_init(void) {
}

/**
 * Zero counters and enable divider
 */
void ac_cpu_perf_zero_counters(ac_bool enable_divider) {
}

/**
 * Frequency of ac_cpu_perf_cycle_counter in cycles per second.
 */
ac_u64 ac_cpu_perf_cycle_counter_freq() {
  // Just guess for now
  return (ac_u64)2000000000;
}
