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

/**
 * Frequency of ac_cpu_perf_cycle_counter in cycles per second.
 */
ac_u64 ac_tsc_freq() {
  // Just guess for now
  return (ac_u64)2000000000;
}

/**
 * Initialize module
 */
void ac_tsc_init(void) {
}
