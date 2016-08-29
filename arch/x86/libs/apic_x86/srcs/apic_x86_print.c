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

#include <apic_x86_print.h>
#include <apic_x86.h>

#include <ac_inttypes.h>

#include <ac_printf.h>

/**
 * Print 256 bit register from most significant bit down to the least
 */
void apci_u256_print(char* lstr, ac_u8* p) {
  if (lstr == AC_NULL) {
    lstr = "";
  }
  ac_printf(lstr);
  for (ac_sint i = 7; i >= 0; i--) {
    if (i != 7) ac_printf(" ");
    ac_u32 u32 = *(ac_u32*)(p + (i * 16));
    for (ac_sint j = 3; j >= 0; j--) {
      ac_u8 byte = AC_GET_BITS(ac_u8, u32, 8, j * 8);
      if ((byte & 0xF0) == 0x00) {
        ac_printf("0"); // Need a leading zero
      }
      ac_printf("%x", byte);
    }
  }
  ac_printf("\n");
}

/**
 * Print Interrupt Request Register, a 256 bit register
 */
void apic_irr_print(char* lstr) {
  apci_u256_print(lstr, get_apic_irr_addr());
}

/**
 * Print In-Service Register, a 256 bit register
 */
void apic_isr_print(char* lstr) {
  apci_u256_print(lstr, get_apic_isr_addr());
}

/**
 * Print Tigger Mode Register, a 256 bit register
 */
void apic_tmr_print(char* lstr) {
  apci_u256_print(lstr, get_apic_tmr_addr());
}
