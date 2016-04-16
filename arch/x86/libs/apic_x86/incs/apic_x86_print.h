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

#ifndef ARCH_X86_APIC_X86_INCS_APIC_X86_PRINT_H
#define ARCH_X86_APIC_X86_INCS_APIC_X86_PRINT_H

#include <ac_inttypes.h>

/**
 * Print 256 bit register
 */
void apci_u256_print(char* lstr, ac_u8* p);

/**
 * Print Interrupt Request Register, a 256 bit register
 */
void apic_irr_print(char* lstr);

/**
 * Print In-Service Register, a 256 bit register
 */
void apic_isr_print(char* lstr);

/**
 * Print Tigger Mode Register, a 256 bit register
 */
void apic_tmr_print(char* lstr);


#endif
