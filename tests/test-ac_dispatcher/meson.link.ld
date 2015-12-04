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

ENTRY(_Reset)
SECTIONS
{
  . = 0x10000;
  .startup : { *( .startup ) }
  . = . + 0;
  .text : { *(.text*) }
  .data : { *(.data) }
  .bss : { *(.bss COMMON) }
  . = ALIGN(8);
  . = . + 0x10000; /* 64kB stack */
  svc_stack_top = .;
  . = . + 0x1000;
  usr_stack_top = .;
  . = . + 0x1000;
  irq_stack_top  = .;
  . = . + 0x1000;
  fiq_stack_top = .;
  . = . + 0x1000;
  mon_stack_top  = .;
  . = . + 0x1000;
  abt_stack_top = .;
  . = . + 0x1000;
  und_stack_top = .;
  . = . + 0x1000;
  sys_stack_top = .;
}
