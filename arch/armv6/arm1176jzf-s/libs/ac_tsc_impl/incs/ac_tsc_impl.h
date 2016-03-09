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

#ifndef SADIE_ARCH_ARMV6_ARM1176JZF_S_LIBS_AC_TSC_INCS_AC_TSC_IMPL_H
#define SADIE_ARCH_ARMV6_ARM1176JZF_S_LIBS_AC_TSC_INCS_AC_TSC_IMPL_H

/**
 * From [here](http://stackoverflow.com/questions/3247373/how-to-measure-program-execution-time-in-arm-cortex-a8-processor)
 */

#include <ac_inttypes.h>

// Defined in ac_tsc_impl
extern ac_u32 ac_tsc_impl_arm_overflow;

/**
 * Return the current cycle counter value
 */
static inline ac_u64 ac_tscrd() {
  ac_u32 value;
  __asm__ volatile ("MRC p15, 0, %0, c9, c13, 0\t\n" : "=r"(value));  

  return ((ac_u64)ac_tsc_impl_arm_overflow << 32) | value;
}

#endif
