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

#define AC_IO_WAIT_UNUSED_PORT (0x80)

static inline ac_u8 inb_port(ac_u16 port) {
  ac_u8 val;
  __asm volatile ( "inb %[port], %[val]" : [val]"=a"(val) : [port] "d"(port));
  return val;
}

static inline ac_u16 inw_port(ac_u16 port) {
  ac_u16 val;
  __asm volatile ( "inw %[port], %[val]" : [val]"=a"(val) : [port] "d"(port));
  return val;
}

static inline void outb_port_value(ac_u16 port, ac_u8 val) {
    __asm volatile ( "outb %[val], %[port]" : : [val] "a"(val), [port] "d"(port) );
}

static inline void outw_port_value(ac_u16 port, ac_u16 val) {
    __asm volatile ( "outw %[val], %[port]" : : [val] "a"(val), [port] "d"(port) );
}

static inline void io_wait(void) {
  // See http://wiki.osdev.org/Inline_Assembly/Examples#IO_WAIT0
  outb_port_value(AC_IO_WAIT_UNUSED_PORT, 0);
}

#endif
