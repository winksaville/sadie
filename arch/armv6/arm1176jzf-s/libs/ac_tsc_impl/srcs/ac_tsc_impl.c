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
 * From [here](http://stackac_tsc_impl_arm_overflow.com/questions/3247373/how-to-measure-program-execution-time-in-arm-cortex-a8-processor)
 */

#include <ac_tsc.h>

#include <ac_inttypes.h>

// Set t AC_TRUE to init MCR's. Right now they
// are causing ac_exception_undef_handler and
// ac_exception_reset_handler to fire so they
// are NOT initialized.
#define INIT_MCRS AC_FALSE

// Divide counter by 64
#define DIVIDE_BY_64 AC_FALSE

static ac_u32 divider;
static ac_u32 freq;

// TODO: ac_tsc_impl_arm_overflow is off and is currently always zero.
// Needs to be public for use by ac_tsc_impl.h
ac_u32 ac_tsc_impl_arm_overflow;

/**
 * Zero counters and enable divider
 */
static void tsc_zero_counters(ac_bool enable_divider) {
}

/**
 * Frequency of ac_tsc in cycles per second.
 */
ac_u64 ac_tsc_freq() {
  return (ac_u64)freq;
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
 */
void ac_tsc_init(void) {
#if INIT_MCRS
  // Allow user mode to access
  __asm__ volatile ("MCR p15, 0, %0, C9, C14, 0\n\t" :: "r"(1));

  // For now disable counter ac_tsc_impl_arm_overflow
  __asm__ volatile ("MCR p15, 0, %0, C9, C14, 2\n\t" :: "r"(0x8000000f));
#endif


  ac_u32 value = 0x7; // Enable all counters and reset the to 0
#ifdef DIVIDE_BY_64
    divider = 64;
    value |= 0x8; // Divide by 64
#else
    divider = 1;
#endif

  // TODO: Measure the freq
  freq = 1000000 / divider;
  ac_tsc_impl_arm_overflow = 0;

#if INIT_MCRS
  __asm__ volatile ("MCR p15, 0, %0, c9, c12, 0\t\n" :: "r"(value));
  __asm__ volatile ("MCR p15, 0, %0, c9, c12, 1\t\n" :: "r"(0x8000000f));
  __asm__ volatile ("MCR p15, 0, %0, c9, c12, 3\t\n" :: "r"(0x8000000f));
#endif
  tsc_zero_counters(AC_FALSE);
}
