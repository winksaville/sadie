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

#define PROC_MODE_MASK  0x1F
#define USR_MODE        0x10
#define FIQ_MODE        0x11
#define IRQ_MODE        0x12
#define SVC_MODE        0x13
#define MON_MODE        0x16
#define ABT_MODE        0x17
#define HYP_MODE        0x1A
#define UND_MODE        0x1C
#define SYS_MODE        0x1F

  .global ac_init
ac_init:
  stmfd sp!, {lr}

  // Setup stacks for the various processor modes
  
  // Start out and use r0 to contain the current cpsr
  mrs r0, cpsr    // Read Program Status Register

  // Cannot setup user stack this way because
  // we can't change the mode bits in cpsr from
  // usr-mode back to system.
  //bic r1, r0, #PROC_MODE_MASK
  //orr r1, r1, #USR_MODE
  //msr cpsr, r1
  //ldr sp, =usr_stack_top

  // Change to IRQ_MODE and set its stack
  bic r1, r0, #PROC_MODE_MASK
  orr r1, r1, #IRQ_MODE
  msr cpsr, r1
  ldr sp, =irq_stack_top

  // Change to FIQ_MODE and set its stack
  bic r1, r0, #PROC_MODE_MASK
  orr r1, r1, #FIQ_MODE
  msr cpsr, r1
  ldr sp, =fiq_stack_top

  // Change to MON_MODE and set its stack
  bic r1, r0, #PROC_MODE_MASK
  orr r1, r1, #MON_MODE
  msr cpsr, r1
  ldr sp, =mon_stack_top

  // Change to ABT_MODE and set its stack
  bic r1, r0, #PROC_MODE_MASK
  orr r1, r1, #ABT_MODE
  msr cpsr, r1
  ldr sp, =abt_stack_top

  // Change to UND_MODE and set its stack
  bic r1, r0, #PROC_MODE_MASK
  orr r1, r1, #UND_MODE
  msr cpsr, r1
  ldr sp, =und_stack_top

  // Change to SVC_MODE and set its stack
  bic r1, r0, #PROC_MODE_MASK
  orr r1, r1, #SVC_MODE
  msr cpsr, r1
  ldr sp, =svc_stack_top

  // Change back to the original mode
  // which is system mode and continue
  msr cpsr, r0

  mov r0, #0x00   // Address base of the ARM Interrupt Vectors
  ldr r1, =vector_table
  ldr r3, =vector_table_end

  // Transfer the vector_table to low memory
loop:
  cmp r1, r3
  bge done
  ldr r2, [r1],#4 // Get next vector
  str r2, [r0],#4 // Store it
  b   loop

done:
  bl  ac_exception_init
  bl  ac_tsc_init
  bl  ac_receptor_early_init
  mov r0, #10
  bl  ac_thread_early_init
  //bl  ac_cpu_perf_init // Does NOT work on qemu
  bl  ac_interrupts_cpu_enable

  ldmfd sp!, {pc} // Return

vector_table:
  ldr pc, addr_ac_exception_reset_handler
  ldr pc, addr_ac_exception_undef_handler
  ldr pc, addr_ac_exception_svc_handler
  ldr pc, addr_ac_exception_prefetch_abort_handler
  ldr pc, addr_ac_exception_data_abort_handler
  ldr pc, addr_ac_exception_undef_handler
  ldr pc, addr_ac_exception_irq_wrapper
  ldr pc, addr_ac_exception_fiq_handler

addr_ac_exception_reset_handler:
  .word ac_exception_reset_handler
addr_ac_exception_undef_handler:
  .word ac_exception_undef_handler
addr_ac_exception_svc_handler:
  .word ac_exception_svc_handler
addr_ac_exception_prefetch_abort_handler:
  .word ac_exception_prefetch_abort_handler
addr_ac_exception_data_abort_handler:
  .word ac_exception_data_abort_handler
addr_ac_exception_irq_wrapper:
  .word ac_exception_irq_wrapper
addr_ac_exception_fiq_handler:
  .word ac_exception_fiq_handler

vector_table_end:

