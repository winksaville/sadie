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

#ifndef ARCH_X86_PAGE_TABLE_X86_INCS_PRINT_PAGE_TABLE_X86_H
#define ARCH_X86_PAGE_TABLE_X86_INCS_PRINT_PAGE_TABLE_X86_H


#include <page_table_x86.h>
#include <cr_x86.h>

#include <ac_inttypes.h>

void print_page_table(union cr3_u cr3u, enum page_mode mode);

#endif
