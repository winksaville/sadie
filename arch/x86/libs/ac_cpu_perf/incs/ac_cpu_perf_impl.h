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

#ifndef ARCH_X86_LIBS_AC_CPU_PERF_INCS_AC_CPU_PERF_IMPL_H
#define ARCH_X86_LIBS_AC_CPU_PERF_INCS_AC_CPU_PERF_IMPL_H

#include <ac_inttypes.h>

/**
 * Return the current cycle counter value
 */
static inline ac_u64 cpu_perf_x86_rdtsc(void) {
  // Execute the rdtscp, read Time Stamp Counter, instruction
  // returns the 64 bit TSC value and writes ecx to tscAux value.
  // The tscAux value is the logical cpu number and can be used
  // to determine if the thread migrated to a different cpu and
  // thus the returned value is suspect.
  ac_u32 lo, hi;
  __asm__ volatile (
      "rdtsc\n\t"
      :"=a"(lo), "=d"(hi));
  // tscAux = aux
  return ((ac_u64)hi << 32) | (ac_u64)lo;
}

/**
 * Return the current cycle counter value plus auxilliary information
 */
static inline ac_u64 cpu_perf_x86_rdtscp(ac_u32* aux) {
  // Execute the rdtscp, read Time Stamp Counter, instruction
  // returns the 64 bit TSC value and writes ecx to tscAux value.
  // The tscAux value is the logical cpu number and can be used
  // to determine if the thread migrated to a different cpu and
  // thus the returned value is suspect.
  ac_u32 lo, hi;
  __asm__ volatile (
      "rdtscp\n\t"
      :"=a"(lo), "=d"(hi), "=rm"(*aux));
  // tscAux = aux
  return ((ac_u64)hi << 32) | (ac_u64)lo;
}

/**
 * Return the current cycle counter value
 */
static inline ac_u64 ac_cpu_perf_cycle_counter_rd(void) {
  return cpu_perf_x86_rdtsc();
}

#endif
