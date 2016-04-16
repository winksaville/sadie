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

#ifndef ARCH_LIBS_INTERRUPTS_X86_incs_INTERRUPTS_X86_PRINT_H
#define ARCH_LIBS_INTERRUPTS_X86_incs_INTERRUPTS_X86_PRINT_H

#include <interrupts_x86.h>

// To use print_full_xxx_stack_frame the first and last statements
// in the interrupt handler must be INTR_SAVE_REGS and INTR_RESTORE_REGS
// respectively.

// Cause C compiler to save "all" registers for interrupt handlers
#define INTR_SAVE_REGS \
__asm__ volatile(""::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", \
                       "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15")

// We don't need to do anything for restoring
#define INTR_RESTORE_REGS

/**
 * Print full exception stack frame, add INTR_SAVE_REGS and
 * INTR_RESTORE_REGS to the interrupt handler.
 */
void print_full_expt_stack_frame(char* str, struct intr_frame* f);

/**
 * Print full interrupt stack frame, add INTR_SAVE_REGS and
 * INTR_RESTORE_REGS to the interrupt handler.
 */
void print_full_intr_stack_frame(char* str, struct intr_frame* f);

/**
 * Print the interrupt frame only
 */
void print_intr_frame(char* str, intr_frame* frame);

/**
 * Print an interrupt/exception gate
 */
void print_intr_gate(char *str, intr_gate* gate);

#endif
