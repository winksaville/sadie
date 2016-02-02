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
#include <io_x86.h>

#include <ac_inttypes.h>

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
#define RX_FIFO 0       // Receive Data register from FIFO (read only)
#define TX_FIFO 0       // Transmit Data register to FIFO (write only)
#define IER     1       // Interrupt Enable register (read/write)
#define IIR     2       // Interrupt Identification register (read only)
#define FCR     2       // FIFO Control Register (write only)
#define LCR     3       // Line Control Register (read/write)
#define MCR     4       // Modem Control Register (read/write)
#define LSR     5       // Line Status Register (read only)
#define MSR     6       // Modem Status Register (read only)
#define SCRATCH 7       // Scratch Register (read/write)

// Line Status Register bits
#define LSR_DATA_READY    0x01
#define LSR_OVERRUN_ERROR 0x02
#define LSR_PARITY_ERROR  0x04
#define LSR_FRAMING_ERROR 0x08
#define LSR_BREAK_INTR    0x10
#define LSR_TX_EMPTY      0x20
#define LSR_TX_RX_EMPTY   0x40
#define LSR_RX_ERROR      0x80

#define COM_TX_FIFO ((ac_u16)(COM1_BASE + TX_FIFO))
#define COM_LSR ((ac_u16)(COM1_BASE + LSR))

static void xmit_char(ac_u8 ch) {
  while ((inb(COM_LSR) & LSR_TX_EMPTY) == 0) {
    // Waiting for LSR_TX_EMPTY to be 1
  }
  outb(ch, COM_TX_FIFO);
}

void ac_putchar(ac_u8 ch)  {
  xmit_char(ch);

  // Needed for vt100 terminals. For instance, when the screen app
  // is connected to a serial port.
  if (ch == '\n') {
    xmit_char('\r');
  }
}

