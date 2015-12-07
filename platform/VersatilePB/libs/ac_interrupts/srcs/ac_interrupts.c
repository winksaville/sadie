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

/**
 * The timers on the VersatilePB are SP804 with the documenation
 * [here](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0271d/index.html)
 */
#include <ac_timer.h>

#include <ac_inttypes.h>

// The base memory address for the VersatilePB is
// [here](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0224i/I1042232.html)
#define PIC_BASE  ((ac_u8*)0x10140000)

// Register offsets from timer_base[0]
// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0271d/index.html
#define PIC_IRQ_STATUS      0x00
#define PIC_FIQ_STATUS      0x04
#define PIC_RAW_STATUS      0x08
#define PIC_INT_SELECT      0x0C
#define PIC_INT_ENABLE      0x10
#define PIC_INT_CLEAR       0x14
#define PIC_SOFT_INT        0x18
#define PIC_SOFT_INT_CLEAR  0x1C
#define PIC_PROTECTION      0x20
#define PIC_VECT_ADDR       0x30
#define PIC_DEF_VECT_ADDR   0x34


#define READ_REG_U32(reg) \
  (*(volatile ac_u32*)(PIC_BASE + reg))

#define WRITE_REG_U32(reg, value) \
  (*(volatile ac_u32*)(PIC_BASE + reg)) = value

#define READ_REG_U8(reg) \
  (*(volatile ac_u8*)(PIC_BASE + reg))

#define WRITE_REG_U8(reg, value) \
  (*(volatile ac_u8*)(PIC_BASE + reg)) = value

/**
 * Read IRQ status register
 */
ac_u32 ac_interrupts_rd_irq_status() {
  return READ_REG_U32(PIC_IRQ_STATUS);
}

/**
 * Read FIQ status register;
 */
ac_u32 ac_interrupts_rd_fiq_status() {
  return READ_REG_U32(PIC_FIQ_STATUS);
}

/**
 * Read raw interrupt status register
 */
ac_u32 ac_interrupts_rd_ris_status() {
  return READ_REG_U32(PIC_FIQ_STATUS);
}

/**
 * Read Interrupt Select Register a 0 means an interrupt
 * will generate an IRQ and 1 means FIQ
 */
ac_u32 ac_interrupts_rd_int_select() {
  return READ_REG_U32(PIC_INT_SELECT);
}

/**
 * Write Interrupt Select Register a 0 means an interrupt
 * will generate an IRQ and 1 means FIQ
 */
void ac_interrupts_wr_int_select(ac_u32 bits) {
  WRITE_REG_U32(PIC_INT_SELECT, bits);
}


/**
 * Read Interrupt Enable Bits a 0 means an interrupt
 * is disabled and 1 is enabled
 */
ac_u32 ac_interrupts_rd_int_enable() {
  READ_REG_U32(PIC_INT_ENABLE);
}

/**
 * Read Interrupt Enable Bits a 0 means an interrupt
 * is disabled and 1 is enabled
 */
void ac_interrupts_wr_int_enable(ac_u32 bits) {
  WRITE_REG_U32(PIC_INT_ENABLE, bits);
}

/**
 * Write Interrupt Clear Bit a 1 means an interrupt
 * is cleared a 0 is no effect
 */
void ac_interrupts_wr_int_clear(ac_u32 bits) {
  WRITE_REG_U32(PIC_INT_CLEAR, bits);
}

