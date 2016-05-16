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

#include <io_x86.h>

#include <ac_bits.h>
#include <ac_inttypes.h>
#include <ac_printf.h>

#define NDEBUG
#include <ac_debug_printf.h>

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
 */
void ac_tsc_init(void) {
  // Program PIT (Programmable Interval Timer) which runs at 1,193,181.6666HZ
  // to run for a 100 of a second and see what the tsc duration is
  // to calculate the freq.

  // From: http://wiki.osdev.org/APIC_timer and
  //       http://wiki.osdev.org/Programmable_Interval_Timer

  // bit0 controls the PIT (8253) Channel 2 gate input, not sure about bit1
  ac_u8 port61 = inb_port(0x61);
  io_wait();
  port61 = (port61 & 0xfc) | 0x1; // bit1 = 0, bit0 = 1, Others unchanged.
  outb_port_value(0x61, port61);

  // Calculate the rate to Write PIT channel 2 data port
  ac_uint pit_freq = 1193181;
  ac_uint cal_time = 20; // 1/20th of a second
  ac_uint count_rate = pit_freq / cal_time;
  ac_debug_printf("ac_tsc_init: pit_freq=%d cal_time=1/%dsec count_rate=%d(0x%x)\n",
      pit_freq, cal_time, count_rate, count_rate);

  // PIT channel 2 will be running for 1/time of a second
  outb_port_value(0x42, AC_GET_LOWB(count_rate));
  io_wait();
  outb_port_value(0x42, AC_GET_HIB(count_rate));

  // First reading of tsc
  ac_u64 first = ac_tscrd();

  // Toggle PIT channel 2 gate input to start its counter
  outb_port_value(0x61, port61 & 0xfe);
  outb_port_value(0x61, port61 | 0x01);

  // Wait 1/time seconds until PIT finishes
  ac_bool bit5;
  do {
    bit5 = inb_port(0x61) & 0x20;
  } while (bit5 == 0);

  // Second reading of tsc
  ac_u64 second = ac_tscrd();

  // Calculate the frequency of tsc
  ac_u64 diff_ticks = second - first;
  freq = diff_ticks * cal_time;
  ac_printf("ac_tsc_init: freq=%ld cal_time=1/%dsec\n",
      freq, cal_time);
}
