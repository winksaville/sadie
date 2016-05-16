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

#ifndef SADIE_LIBS_INCS_AC_TSC_H
#define SADIE_LIBS_INCS_AC_TSC_H

#include <ac_inttypes.h>
#include <ac_intmath.h>

/**
 * Initialize Time Stamp Counter module
 */
void ac_tsc_init(void);

/**
 * Set the frequency that will be returned by ac_tsc_freq.
 * This is primiarly used for testing as freq us usually
 * initialized by ac_tsc_init.
 */
void ac_tsc_set_freq(ac_u64 f);

/**
 * Frequency of tsc in cycles per second.
 */
ac_u64 ac_tsc_freq();

#define AC_NANOSECS 1000000000

#define ac_ns_to_ticks(nanosecs) ({ \
  ac_u64 secs = nanosecs / AC_NANOSECS; \
  ac_u64 sub_secs = nanosecs % AC_NANOSECS; \
  ac_u64 ticks = secs * ac_tsc_freq(); \
  ticks += AC_U64_DIV_ROUND_UP(sub_secs * ac_tsc_freq(), AC_NANOSECS); \
  ticks; \
})

/**
 * Get the implemetation header which has the
 * ac_tscrd() call, hopefully inlineable.
 */
#include <ac_tsc_impl.h>

#endif
