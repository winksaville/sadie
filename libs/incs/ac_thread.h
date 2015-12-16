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

#ifndef SADIE_PLATFORM_VERSATILEPB_LIBS_AC_CPU_PERF_INCS_AC_THREAD_H
#define SADIE_PLATFORM_VERSATILEPB_LIBS_AC_CPU_PERF_INCS_AC_THREAD_H

#include <ac_inttypes.h>

#include <ac_thread_stack_min.h>

/**
 * Initialize module
 */
void ac_thread_init(ac_u32 max_threads);

/**
 * The current thread yeilds the CPU to the next
 * ready thread.
 */
void ac_thread_yield(void) __attribute__ ((naked))
                           __attribute__ ((noreturn));

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
 * @return 0 on success !0 if an error.
 */
ac_u32 ac_thread_create(ac_size_t stack_size,
    void*(*entry)(void*), void* entry_arg);

#endif
