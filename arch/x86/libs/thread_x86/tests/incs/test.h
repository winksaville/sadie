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

/**
 * Privates for use in testing
 */

#ifndef ARCH_X86_LIBS_THREAD_X86_TESTS_INCS_TEST_H
#define ARCH_X86_LIBS_THREAD_X86_TESTS_INCS_TEST_H

#include <ac_inttypes.h>

/**
 * @return timer_reschedule_isr_counter.
 */
ac_u64 get_timer_reschedule_isr_counter(void);

/**
 * Set timer_reschedule_isr_counter to value
 *
 * @param value is stored in counter
 */
void set_timer_reschedule_isr_counter(ac_u64 value);

/**
 * Remove zombie threads
 *
 * @return number of threads removed
 */
ac_uint remove_zombies(void);

/**
 * Test waiting tick on 1 or more threads.
 *
 * @params simultaneous_thread is the number of threads
 * @params ns is true if to use thread_wait_ns and false for thread_wait_ticks
 */
ac_bool test_thread_wait(ac_uint simultaneous_threads, ac_bool ns);

#endif
