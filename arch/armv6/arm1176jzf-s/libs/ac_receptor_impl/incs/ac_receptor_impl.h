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

#ifndef SADIE_ARCH_X86_LIBS_RECEPTOR_X86_INCS_AC_RECEPTOR_IMPL_H
#define SADIE_ARCH_X86_LIBS_RECEPTOR_X86_INCS_AC_RECEPTOR_IMPL_H

#include <ac_inttypes.h>

/**
 * Receptor structure
 */
typedef struct {
  ac_bool signaled;
} receptor_armv6;

typedef struct receptor_armv6* ac_receptor_t;

/**
 * Create a receptor and set its state to NOT signaled
 *
 * @return AC_NULL if unable to allocate a receptor
 */
ac_receptor_t ac_receptor_create(void);

/**
 * Destroy a receptor
 */
void ac_receptor_destroy(ac_receptor_t receptor);

/**
 * Wait for the receptor to be signaled only one entity can wait
 * on a receptor at a time. If the receptor has already been signaled
 * ac_receptor_wait will return immediately.
 *
 * @return 0 if successfull !0 if an error.
 */
ac_uint ac_receptor_wait(ac_receptor_t receptor);

/**
 * Signal the receptor.
 *
 * @param receptor to signal
 * @param if yield_if_waiting is true then this thread yields
 */
void ac_receptor_signal(ac_receptor_t receptor, ac_bool yield_if_waiting);

/**
 * Initialize this module early, must be
 * called before receptor_init
 */
void ac_receptor_early_init(void);

/**
 * Initialize this module defining the number of
 * sensors to support.
 */
void ac_receptor_init(ac_uint max_receptors);

#endif
