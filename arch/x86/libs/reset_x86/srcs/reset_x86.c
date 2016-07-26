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

#include <interrupts_x86.h>
#include <native_x86.h>

#include <ac_inttypes.h>
#include <ac_printf.h>

#if 0
typedef unsigned short uint16_t;
typedef unsigned long long uint64_t;

// This does not work and is the original code from
// [The easiest way to reset i386/x86_64 system]
// (http://www.pagetable.com/?p=140). The reson it
// doesn't work is the lidt instruction caused a
// general protection fault because the object lidt
// loads is two fields a u16 and a u64.

__attribute__ ((__noreturn__))
void reset_x86(void) {
  uint64_t null_idtr = 0;
  __asm__ volatile ("lidt %0; int3" :: "m" (null_idtr));

  // Loop with interrupts off if it doesn't work
  __asm__ volatile ("cli");
  while (1) {
    __asm__ volatile ("hlt");
  }
}
#endif

#if 0
typedef unsigned short uint16_t;
typedef unsigned long long uint64_t;

// This does work and is what I posted as the solution
// to the above code on [The easiest way to reset i386/x86_64 system]
// (http://www.pagetable.com/?p=140).

__attribute__ ((__noreturn__))
void reset_x86(void) {
  struct {
    uint16_t limit;
    uint64_t address;
  } null_idtr;

  null_idtr.limit = 0;
  null_idtr.address = 0;

  __asm__ volatile ("lidt %0; int3" :: "m" (null_idtr));

  // Loop with interrupts off if it doesn't work
  while (1) {
    hlt();
  }
}
#endif

#if 1
__attribute__ ((__noreturn__))
void reset_x86(void) {

  // From [The easiest way to reset i386/x86_64 system]
  // (http://www.pagetable.com/?p=140). The original code
  // caused an GP fault (exception 13), but this code
  // works.

  IdtPtr null_idtr;

  null_idtr.limit = 0;
  null_idtr.iig = 0;

  cli();
  set_idt(&null_idtr);
  intr(3);

  // Loop with interrupts off if it doesn't work
  while (1) {
    hlt();
  }
}
#endif
