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

#ifndef SADIE_ARCH_X86_LIBS_IOAPIC_X86_INCS_IOAPIC_X86_PRINT_H
#define SADIE_ARCH_X86_LIBS_IOAPIC_X86_INCS_IOAPIC_X86_PRINT_H

#include <ioapic_x86.h>

/**
 * Print ioapic_redir
 *
 * @param str string to print first
 * @param redir is ioapci_redir to print
 * @param termstr terminating string to print
 */ 
void ioapic_redir_print(char* str, ioapic_redir redir, char* termstr);

#endif
