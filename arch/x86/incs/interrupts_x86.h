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

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <ac_inttypes.h>
#include <descriptors_x86.h>

/**
 * Interrupt stack frame
 *
 * This is the signature required for the compiler so
 * intr_handler and expt_handler are can be marked with
 * the __attribute__((__interrupt__))
 */
typedef struct intr_frame {
  ac_uptr ip;
  ac_uptr cs;
  ac_uptr flags;
  ac_uptr sp;
  ac_uptr ss;
} intr_frame;

typedef void (intr_handler)(struct intr_frame* frame);

typedef void (expt_handler) (struct intr_frame* frame, ac_uptr error_code);

void set_intr_handler(ac_u32 intr_num, intr_handler ih);

void set_expt_handler(ac_u32 intr_num, expt_handler eh);

idt_intr_gate* get_idt_intr_gate(ac_u32 intr_num);

void initialize_intr_descriptor_table(void);

#endif
