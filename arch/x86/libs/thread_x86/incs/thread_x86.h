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

#ifndef SADIE_ARCH_X86_LIBS_AC_THREAD_IMPL_INCS_THREAD_X86_H
#define SADIE_ARCH_X86_LIBS_AC_THREAD_IMPL_INCS_THREAD_X86_H

#include <interrupts_x86.h>

#include <ac_inttypes.h>
#include <ac_thread.h>

/**
 * The current thread yeilds the CPU to the next
 * ready thread.
 *
 * @param remove_pready, if AC_TRUE then while yielding
 *        the tcb pointed to by pready will be removed
 *        the pready list.
 */
void thread_yield(ac_bool remove_pready);

/**
 * Remove the thread from its current list
 *
 * @param hdl is an opaque thread handle
 *
 * @return 0 if successful
 */
ac_uint thread_remove_from_list(ac_thread_hdl_t hdl);

/**
 * Make the thread not ready,
 *
 * @param hdl is an opaque thread handle
 */
void thread_make_not_ready(ac_thread_hdl_t hdl);

/**
 * Make the thread ready
 *
 * @param hdl is an opaque thread handle
 *
 * @return 0 if successful
 */
ac_uint thread_make_ready(ac_thread_hdl_t hdl);

/**
 * Handle the reschedule interrupt. Defined in thread_x86_asm.S
 */
void reschedule_isr(struct intr_frame* frame);

/**
 * Handle the timer reschedule interrupt. Defined in thread_x86_asm.S
 */
void timer_reschedule_isr(struct intr_frame* frame);

/**
 * Get number of timer_reschedule_isr that have occurred.
 */
ac_u64 get_timer_reschedule_isr_counter(void);

/**
 * Print ready list
 */
void print_ready_list(const char* str);

#endif
