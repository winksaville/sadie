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

  // This will be placed in the .starup section which
  // must be the first section. The _Reset label is currently
  // not used and instead this code must be loactaed at
  // 0x10000 so that it is the first code executed.

  // Not only must this be the first section, but it must have
  // the 'a' flag. If 'a' is not present the objcopy will not copy
  // the section to the .bin file. The 'x' flag is not required
  // but seems reasonable as it marks the section execute only.
  //
  // In the future, if there is any data stored the 'w' flag should
  // be added to make the section writable.
  .section .startup, "ax"

  // Not needed now but would allow this section to be place
  // else where and executed by "Calling" _Reset
  .global _Reset
_Reset:
  // Change to system mode and set the stack which is defined in link.xxx.d
  mrs r0, cpsr    // Read Program Status Register
  bic r1, r0, #PROC_MODE_MASK
  orr r1, r1, #SYS_MODE
  msr cpsr, r1
  ldr sp, =sys_stack_top

  // Initialize things such as interrupts
  bl ac_init

  // Call main
  bl  main

  // Power off device
  bl  ac_poweroff

  // Loop endlessly if we get here
  b .

  //ldr pc, ac_exception_undef_handler
