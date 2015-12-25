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

/*
 * An i386 linker script
 *
 * Modeled after [BareBones](http://wiki.osdev.org/Bare_Bones)
 *
 * One difference was I put .bootstrap_stack in a separate
 * section because I was getting a warning from the linker:
 *
 *   warning: section `.bss' type changed to PROGBITS
 *
 * As it turns out the warning was because in Bare Bones
 * there were uninitialized variables defined in the program
 * but not in mine.
 *
 * This was solved by putting the stack in a separate section
 * which seems like a good solution.
 */

ENTRY(_start)

SECTIONS {
  /* Start at image 1 meg */
  . = 1M;

  /* multiboot section must be first then text */
  .text BLOCK(4K) : ALIGN(4K) {
    *(.multiboot)
    *(.text)
  }

  /* Read-write data (initialized) */
  .data BLOCK(4K) : ALIGN(4K) {
    *(.data)
  }

  /* Read-write data (uninitialized) */
  .bss BLOCK(4K) : ALIGN(4K) {
    *(COMMON)
    *(.bss)
  }

  /* Stack separate */
  .stack BLOCK(4K) : ALIGN(4K) {
    *(.bootstrap_stack)
  }

  /* Other sections will be here */
}
