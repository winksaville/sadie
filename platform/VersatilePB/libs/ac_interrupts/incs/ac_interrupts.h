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

#ifndef SADIE_PLATFORM_VERSATILEPB_LIBS_AC_INTERRUPTS_INCS_AC_INETRRUPTS_H
#define SADIE_PLATFORM_VERSATILEPB_LIBS_AC_INTERRUPTS_INCS_AC_INETRRUPTS_H

#include <ac_inttypes.h>

/**
 * Enable the cpu's interrupts
 */
void ac_interrupts_cpu_enable();

/**
 * Disable the cpu's interrupts
 */
void ac_interrupts_cpu_disable();

/**
 * Read IRQ status register
 */
ac_u32 ac_interrupts_rd_irq_status();

/**
 * Read FIQ status register
 */
ac_u32 ac_interrupts_rd_fiq_status();

/**
 * Read raw interrupt status register
 */
ac_u32 ac_interrupts_rd_ris_status();

/**
 * Read Interrupt Select Register a 0 means an interrupt
 * will generate an IRQ and 1 means FIQ
 */
ac_u32 ac_interrupts_rd_int_select();

/**
 * Write Interrupt Select Register a 0 means an interrupt
 * will generate an IRQ and 1 means FIQ
 */
void ac_interrupts_wr_int_select(ac_u32 bits);


/**
 * Read Interrupt Enable Bits a 0 means an interrupt
 * is disabled and 1 is enabled
 */
ac_u32 ac_interrupts_rd_int_enable();

/**
 * Read Interrupt Enable Bits a 0 means an interrupt
 * is disabled and 1 is enabled
 */
void ac_interrupts_wr_int_enable(ac_u32 bits);

/**
 * Write Interrupt Clear Bit a 1 means an interrupt
 * is cleared a 0 is no effect
 */
void ac_interrupts_wr_int_clear(ac_u32 bits);

#endif
