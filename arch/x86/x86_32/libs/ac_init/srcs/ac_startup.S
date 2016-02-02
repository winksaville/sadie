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

  // [From](http://wiki.osdev.org/Bare_Bones)
  // Constants for multiboot header
  .set MAGIC, 0x1BADB002          // A standard magic value
  .set FLAGS, 0x3                 // Bit0 align on page boundry,
                                  // Bit1 provide memory map
  .set CHECKSUM, -(MAGIC + FLAGS) // Checksum

  // The multiboot header
  .section .multiboot
  .align 4
  .long MAGIC
  .long FLAGS
  .long CHECKSUM

  // Start up code
  .section .text
  .global _start
_start:
  movl  $stack_top, %esp  // Initialize stack

  call  ac_init           // Initialize the system
  call  main              // Call main
  call  ac_poweroff       // Power off device
  call  reset_x86         // Reset if power off doesn't work

  // If we return from poweroff we hang
  cli                     // Turn off interrupts
hang:
  hlt                     // Halt
  jmp hang                // Loop if we unhalt

  // Stack
  .section .bootstrap_stack
  .skip 16384
stack_top:

