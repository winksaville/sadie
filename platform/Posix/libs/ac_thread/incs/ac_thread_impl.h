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

#ifndef SADIE_PLATFORM_POSIX_LIBS_AC_THREAD_INCS_AC_THREAD_IMPL_H
#define SADIE_PLATFORM_POSIX_LIBS_AC_THREAD_INCS_AC_THREAD_IMPL_H


#include <ac_inttypes.h>

/**
 * Minimum stack size
 */
//TODO: How to properly include PTHREAD_STACK_MIN
//#define AC_THREAD_STACK_MIN PTHREAD_STACK_MIN
#define AC_THREAD_STACK_MIN 16384

/**
 * Initialize module
 */
void ac_thread_init(ac_u32 max_threads);

/**
 * Set the default slice to ticks.
 *
 * This is an advisory value and maybe be ignored or
 * altered by the system.
 *
 * @param ticks is the number of ticks for each slice.
 */
inline static void AcThread_set_default_slice(ac_u64 ticks) {
  // AcThread_set_default_slice NOOP for AC_PLATFORM == Posix
}

/**
 * Get the default slice to ticks.
 *
 * @return the default number of ticks for each slice, zero if unknown.
 */
inline static ac_u64 AcThread_get_default_slice(void) {
  // AcThread_get_default_slice NOOP for AC_PLATFORM == Posix
  return 0;
}

/**
 * The current thread waits for some number of nanosecs.
 */
void ac_thread_wait_ns(ac_u64 nanosecs);


/**
 * The current thread waits for some number of ticks.
 */
void ac_thread_wait_ticks(ac_u64 ticks);

/**
 * The current thread yeilds the CPU to the next
 * ready thread.
 */
void ac_thread_yield(void);

/**
 * Create a thread and invoke the entry passing entry_arg. If
 * the entry routine returns the thread is considered dead
 * and will not be rescheduled and its stack is reclamined.
 * Any other global memory associated with the thread still
 * exists and is left untouched.
 *
 * @param stack_size is 0 a "default" stack size will be used.
 * @param entry is the routine to run
 * @param entry_arg is the argument passed to entry.
 *
 * @return a ac_thread_rslt contains a status and an opaque ac_thread_hdl_t.
 *         if rslt.status == 0 the thread was created and ac_thread_hdl_t
 *         is valid.
 */
ac_thread_rslt_t ac_thread_create(ac_size_t stack_size,
    void*(*entry)(void*), void* entry_arg);

#endif
