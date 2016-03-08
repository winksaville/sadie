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

#ifndef ARCH_X86_AC_THREAD_INCS_AC_THREAD_X86_H
#define ARCH_X86_AC_THREAD_INCS_AC_THREAD_X86_H

#include <ac_thread.h>

#include <ac_inttypes.h>

/**
 * @return timer_reschedule_isr_counter.
 */
ac_u64 get_timer_reschedule_isr_counter();

/**
 * Initialize this module early phase, must be
 * called before ac_thread_init
 */
void ac_thread_early_init();

/**
 * Initialize this module
 */
void ac_thread_init(ac_u32 max_threads);

#endif
