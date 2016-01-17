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

#include <ac_putchar.h>
#include <ac_io.h>

// From [here](//http://www.lammertbies.nl/comm/info/RS-232_io.html)
#define COM1_BASE 0x3F8
#define COM2_BASE 0x2F8
#define COM3_BASE 0x3E8
#define COM4_BASE 0x2E8

#define COM1_IRQ 4
#define COM2_IRQ 3
#define COM3_IRQ 4
#define COM4_IRQ 3

// and [here](http://www.lammertbies.nl/comm/info/serial-uart.html)
#define RX_FIFO 0
#define TX_FIFO 0

#define COM_TX_FIFO ((ac_u16)(COM1_BASE + TX_FIFO))

void ac_putchar(ac_u8 ch)  {
  outb(ch, COM_TX_FIFO);

  // Needed for vt100 terminals like when the screen app
  // is connected to a serial port such as with the when
  // testing with the x86 serial port.
  if (ch == '\n') {
    outb('\r', COM_TX_FIFO);
  }
}

