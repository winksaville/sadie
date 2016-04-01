/*
 * copyright 2016 wink saville
 *
 * licensed under the apache license, version 2.0 (the "license");
 * you may not use this file except in compliance with the license.
 * you may obtain a copy of the license at
 *
 *     http://www.apache.org/licenses/license-2.0
 *
 * unless required by applicable law or agreed to in writing, software
 * distributed under the license is distributed on an "as is" basis,
 * without warranties or conditions of any kind, either express or implied.
 * see the license for the specific language governing permissions and
 * limitations under the license.
 */

#ifndef SADIE_ARCH_X86_LIBS_THREAD_X86_INCS_WAITING_TCB_H
#define SADIE_ARCH_X86_LIBS_THREAD_X86_INCS_WAITING_TCB_H

#include <thread_x86.h>

#include <ac_inttypes.h>

/**
 * Return the next tcb that is waiting its turn to become ready.
 * The tcb is NOT removed.
 */
tcb_x86* waiting_tcb_peek_intr_disabled(void);

/**
 * Add tcb to waitlist.
 *
 * @param ptcb is the tcb to add
 * @param absolute_tsc is the absolute tsc to wait until
 */
void waiting_tcb_add_intr_disabled(tcb_x86* ptcb, ac_u64 absolute_tsc);

/**
 * Remove the next tcb.
 */
void waiting_tcb_remove_intr_disabled(void);

/**
 * Print the waiting tcbs
 */
void print_waiting_tcbs(void);

/**
 * Update the number of waiting tcbs. Note this is slow and
 * currently disables interrupts during the transition.
 *
 * @param new_max is the new maximum number of waiting tcbs
 */
void waiting_tcbs_update_max(ac_uint new_max);

/**
 * Initialize the waiting tcb data structures
 *
 * @param _max is the new maximum number of waiting tcbs
 */
void waiting_tcbs_init(ac_uint max);

#endif
