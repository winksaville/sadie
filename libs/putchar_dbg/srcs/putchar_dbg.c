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

#include "putchar_dbg.h"

#warning "compiling putchar_dbg.c"
#ifdef VersatilePB
#warning "compiling putchar_dbg.c -- VersatilePB"
volatile uint32_t* const pUart = (uint32_t*)0x101f1000;
#endif

#ifdef Posix
#warning "compiling putchar_dbg.c -- Posix"
#include <stdio.h>
#endif

void putchar_dbg(uint8_t ch) {
#ifdef VersatilePB
   #warning "compiling putchar_dbg.c -- VersatilePB write pUart"
  *pUart = (uint32_t)ch;
#endif
#ifdef Posix
   #warning "compiling putchar_dbg.c -- Posix call putchar"
  putchar(ch);
#endif
}
