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

/**
 * Debug code for mpscfifo
 */

#ifndef SADIE_LIBS_AC_MEM_AC_MEM_DBG_H
#define SADIE_LIBS_AC_MEM_AC_MEM_DBG_H

#include <ac_mem.h>

/**
 * Print a AcMem and the specified number of bytes in data
 */
void AcMem_print_count(const char* leader, AcMem* buff, ac_u32 data_count_to_print);

/**
 * Print a AcMem and all of the user data
 */
void AcMem_print(const char* leader, AcMem* buff);

#ifdef NDEBUG
  #define AcMem_debug_print(leader, buff) ((void)(0))
#else
  #define AcMem_debug_print(leader, buff) AcMem_print(leader, buff)
#endif

#endif
