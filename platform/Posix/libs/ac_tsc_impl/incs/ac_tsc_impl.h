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

#ifndef ARCH_X86_LIBS_AC_TSC_IMPL_INCS_AC_TSC_IMPL_H
#define ARCH_X86_LIBS_AC_TSC_IMPL_INCS_AC_TSC_IMPL_H

#include <native_x86.h>

#include <ac_inttypes.h>

/**
 * Return the time stamp counter plus auxilliary information
 */
static inline ac_u64 ac_tscrd_aux(ac_u32* aux) {
  return rdtsc_aux(aux);
}

/**
 * Return the time stack counter
 */
static inline ac_u64 ac_tscrd(void) {
  return rdtsc();
}

#endif
