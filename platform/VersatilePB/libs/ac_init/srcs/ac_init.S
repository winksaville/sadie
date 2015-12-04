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

  //.section .text, "ax"
  //.section .text, "ax"

  .global ac_init
ac_init:
  mov r0, #0x00   // Address base of the ARM Interrupt Vectors
  ldr r1, =vector_table
  ldr r3, =vector_table_end

  // Transfer the vector_table to low memory
loop:
  cmp r1, r3
  bge done
  ldr r2, [r1],#4 // Get next vecotor
  str r2, [r0],#4 // Store it
  b   loop

done:
  mov pc, lr      // Return

vector_table:
  ldr pc, addr_ac_exception_reset_handler
  ldr pc, addr_ac_exception_undef_handler
  ldr pc, addr_ac_exception_svc_handler
  ldr pc, addr_ac_exception_prefetch_abort_handler
  ldr pc, addr_ac_exception_data_abort_handler
  ldr pc, addr_ac_exception_undef_handler
  ldr pc, addr_ac_exception_irq_handler
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
addr_ac_exception_irq_handler:
  .word ac_exception_irq_handler
addr_ac_exception_fiq_handler:
  .word ac_exception_fiq_handler

vector_table_end:

