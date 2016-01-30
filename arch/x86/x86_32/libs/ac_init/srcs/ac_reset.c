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

#include <ac_inttypes.h>

__attribute__ ((__noreturn__))
void ac_reset(void) {
  // [The easiest way to reset i386/x86_64 system]
  // (http://www.pagetable.com/?p=140)
  //  Initialize idt register to null then perform
  //  an interrupt. This causes a triple fault and
  //  resets the CPU.
  ac_u64 null_idtr = 0;
  __asm__ volatile ("lidt %0; int3" :: "m" (null_idtr));
  while (1) {
    __asm__ volatile ("cli; hlt;");
  }
}
