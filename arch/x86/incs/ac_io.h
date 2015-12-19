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

#ifndef SADIE_ARCH_X86_64_INCS_AC_IO_H
#define SADIE_ARCH_X86_64_INCS_AC_IO_H

#include <ac_inttypes.h>

static inline ac_u8 inb(ac_u16 port) {
  ac_u8 val;
  __asm volatile ( "inb %0, %1" :"=r"(val) : "d"(port));
  return val;
}

static inline ac_u16 inw(ac_u16 port) {
  ac_u16 val;
  __asm volatile ( "inw %0, %1" : "=r"(val) : "d"(port));
  return val;
}

static inline void outb(ac_u8 val, ac_u16 port) {
    __asm volatile ( "outb %0, %1" : : "a"(val), "d"(port) );
}

static inline void outw(ac_u16 val, ac_u16 port) {
    __asm volatile ( "outw %0, %1" : : "a"(val), "d"(port) );
}

#endif
