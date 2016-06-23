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

#ifndef SADIE_LIBS_AC_BUFF_AC_BUFF_DBG_H
#define SADIE_LIBS_AC_BUFF_AC_BUFF_DBG_H

#include <ac_buff.h>

/**
 * Print a AcBuff and the specified number of bytes in data
 */
void AcBuff_print_count(const char* leader, AcBuff* buff, ac_u32 data_count_to_print);

/**
 * Print a AcBuff and all of the user data
 */
void AcBuff_print(const char* leader, AcBuff* buff);

#ifdef NDEBUG
  #define AcBuff_debug_print(leader, buff) ((void)(0))
#else
  #define AcBuff_debug_print(leader, buff) AcBuff_print(leader, buff)
#endif

#endif
