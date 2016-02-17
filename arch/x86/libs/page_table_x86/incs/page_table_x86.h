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

#ifndef ARCH_X86_PAGE_TABLE_X86_INCS_PAGE_TABLE_X86_H
#define ARCH_X86_PAGE_TABLE_X86_INCS_PAGE_TABLE_X86_H

#include <ac_inttypes.h>

enum page_mode {
  PAGE_MODE_32BIT,
  PAGE_MODE_PAE,
  PAGE_MODE_64BIT,
};

/**
 * Return the current cpu page_mode
 */
enum page_mode get_page_mode(void);

/**
 * Returns the address of the table and the associated flags
 * and in 
 */
union cr3_u get_page_table(void);


#endif
