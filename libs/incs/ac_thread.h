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

#ifndef SADIE_LIBS_INCS_AC_THREAD_H
#define SADIE_LIBS_INCS_AC_THREAD_H

#include <ac_inttypes.h>

/**
 * Minimum stack size
 */
//#define AC_THREAD_STACK_MIN 0x1000

/**
 * An opaque implemenation defined value returned
 * by ac_thread_create and passed as a parmeter
 * to other methods.
 */
typedef ac_uptr ac_thread_hdl_t;

/**
 * The result from ac_thread_create.
 */
typedef struct {
  ac_uint status;         // status == 0 on success
  ac_thread_hdl_t hdl;    // handle to the thread if successful
} ac_thread_rslt_t;


/**
 * Initialize this module early phase, must be
 * called before ac_thread_init
 */
//void ac_thread_early_init();

/**
 * Initialize module
 */
//void ac_thread_init(ac_u32 max_threads);

/**
 * The current thread yeilds the CPU to the next
 * ready thread.
 */
//void ac_thread_yield(void);

/**
 * The current thread waits for some number of nanosecs.
 */
//void ac_thread_wait_ns(ac_u64 nanosecs);

/**
 * The current thread waits for some number of ticks.
 */
//void ac_thread_wait_ticks(ac_u64 ticks);

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
//ac_thread_rslt_t ac_thread_create(ac_size_t stack_size,
//    void*(*entry)(void*), void* entry_arg);

#include <ac_thread_impl.h>

#endif
