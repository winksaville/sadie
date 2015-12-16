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

/**
 * The ac_irq_wrapper to allow [Reentrant interrupt for ARMv6 (non-vectored
 * interrupts]
 * (http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0471l/pge1358787021322.html).
 * This is accomplished by having irq interrupts use the stack of the
 * interrupted code rather than using dedicated irq stack.
 *
 * To accomplish this "easily" the code uses the system mode as outlined
 * [here]
 * (http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0471l/pge1358787017979.html)
 * And quoting:
 *
 * Corruption of the link register can be a problem when handling multiple
 * exceptions of the same type. ARMv4 and later architectures include a
 * privileged mode called System mode, to overcome this problem.
 *
 * System mode shares the same registers as User mode, it can run tasks that
 * require privileged access, and exceptions no longer overwrite the link
 * register.
 *
 * Some additonal documentation on using one stack is [here]
 * (http://students.mimuw.edu.pl/~zbyszek/asm/arm/ARMv6_Architecture.pdf)
 * And Quoting:
 *
 * Stack Handling and Mode Change support:
 *
 * New stack handling capabilities in ARMv6 avoid the need for multiple stacks.
 * The ARMv6 register model supports separate stacks in the different modes.
 * Many operating systems like to nest all their state saving and restoring
 * onto a single stack. Version 6 makes this much more efficient. The stack
 * handling capabilities are based on new crossmode
 * state-saving instructions:
 *
 *  - SRS #Mode - Save Return State onto stack belonging to ‘Mode’
 *  - RFE - Return From Exception
 *
 * The SRS instruction allows register 14 and the SPSR (Saved Processor Status
 * Register) for the current mode to be saved to a stack in a different mode.
 * The RFE instruction loads the PC and CPSR (Current Processor Status Register)
 * from the saved state.
 *
 * New instructions support fast mode changes in privileged modes. Instructions
 * cannot be used in user mode for security reasons.
 *
 *  - CPSID #Mode (and disable interrupts)
 *  - CPSIE #Mode (and enable interrupts)
 *
 * The CPS instructions allow software to move efficiently to a different mode
 * while enabling or disabling interrupts.
 *
 */

  .global ac_exception_irq_wrapper
ac_exception_irq_wrapper:
  sub   lr, lr, #4      // Adjust LR so it points at the correct PC
  srsfd sp!,#31         // Save LR_irq and SPSR_irq to System mode stack
  cps   #31             // Switch to System mode
  push  {R0-R3,R12}     // Store other AAPCS registers
  and   r1, sp, #4      // R1 will be 0 if already on 64 bit boundry
                        // or 4 if its not.
  sub   sp, sp, R1      // Substract 0 or 4 to align on 64 bit boundry
  push  {r1, lr}        // Push R1 to save realign facotr and LR to
                        // keep it on 64 bit boundry
  bl    ac_exception_irq_identify_and_clear_source
  cpsie i               // Enable IRQ
  bl    ac_exception_irq_handler // Invoke the irq handler
  cpsid  i              // Disable IRQ

  // Invoke scheduler passing sp as the first parameter
  mov   r0, sp
  bl    ac_thread_scheduler
  mov   sp, r0          // Replace sp with the return value

  pop   {r1,lr}         // Get realign factor back to R1 and pop LR
  add   sp, sp, r1      // Realign sp to old value
  pop   {r0-r3, r12}    // Restore registers
  rfefd sp!             // Return using RFE from System mode stack

