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

void print_intr_frame(char* str, intr_frame* frame);

void print_idt_intr_gate(char *str, idt_intr_gate* gate);

#endif
