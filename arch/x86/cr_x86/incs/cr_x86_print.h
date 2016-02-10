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

#ifndef ARCH_X86_CR_X86_INCS_CR_X86_PRINT_H
#define ARCH_X86_CR_X86_INCS_CR_X86_PRINT_H

#include <cr_x86.h>

#include <ac_inttypes.h>

/**
 * Print cr0
 */
void print_cr0(char* str, ac_uint cr0);

/**
 * Print cr3
 */
void print_cr3(char* str, ac_uint cr3);

/**
 * Print cr4
 */
void print_cr4(char* str, ac_uint cr4);

#endif

