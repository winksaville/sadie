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
 * The timers on the VersatilePB is an SP804 the documenation is
 * [here](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0271d/index.html)
 */
#include <ac_timer.h>

#include <ac_inttypes.h>
#include <ac_printf.h>

// The base memory address for the VersatilePB
// [see](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0224i/I1042232.html)
#define PIC_BASE  ((ac_u8*)0x10140000)

// Register offsets from timer_base
// [see](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0271d/index.html)
#define PIC_IRQ_STATUS      0x00
#define PIC_FIQ_STATUS      0x04
#define PIC_RAW_STATUS      0x08
#define PIC_INT_SELECT      0x0C
#define PIC_INT_ENABLE      0x10
#define PIC_INT_ENABLE_CLR  0x14
#define PIC_SOFT_INT        0x18
#define PIC_SOFT_INT_CLEAR  0x1C
#define PIC_PROTECTION      0x20
#define PIC_VECT_ADDR       0x30
#define PIC_DEF_VECT_ADDR   0x34

#define REG_PTR_U32(reg) \
  ((volatile ac_u32*)(PIC_BASE + reg))

#define READ_REG_U32(reg) \
  (*REG_PTR_U32(reg))

#define WRITE_REG_U32(reg, value) \
  *REG_PTR_U32(reg) = value

#define REG_PTR_U8(reg) \
  ((volatile ac_u8*)(PIC_BASE + reg))

#define READ_REG_U8(reg) \
  (*REG_PTR_U8(reg))

#define WRITE_REG_U8(reg, value) \
  (*REG_PTR_U8(reg)) = value

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
 * Return the current interrupts routes, a 0 means an interrupt
 * will generate an IRQ and 1 means FIQ.
 */
ac_u32 ac_interrupts_rd_int_routes() {
  return __atomic_load_n(REG_PTR_U32(PIC_INT_SELECT), __ATOMIC_ACQUIRE);
}

/**
 * For bits == 1 make those interrupts route to the IRQ.
 */
ac_u32 ac_interrupts_int_route_to_irq(ac_u32 bits) {
  return __atomic_and_fetch(REG_PTR_U32(PIC_INT_SELECT), ~bits, __ATOMIC_ACQUIRE);
}

/**
 * For bits == 1 make those interrupts route to the FIQ.
 *
 * return the resulting value.
 */
ac_u32 ac_interrupts_int_route_to_fiq(ac_u32 bits) {
  return __atomic_or_fetch(REG_PTR_U32(PIC_INT_SELECT), bits, __ATOMIC_ACQUIRE);
}


/**
 * Read Interrupt Enable Bits a 1 means an interrupt
 * is enabled and 0 is disabled.
 *
 * return the resulting value.
 */
ac_u32 ac_interrupts_rd_int_enable() {
  //ac_printf("ac_interrupts_rd_int_enable: before=0x%x\n", READ_REG_U32(PIC_INT_ENABLE));
  //ac_u32 retVal = __atomic_load_n(REG_PTR_U32(PIC_INT_ENABLE), __ATOMIC_ACQUIRE);
  //ac_printf("ac_interrupts_rd_int_enable: after=0x%x ret=0x%x\n", READ_REG_U32(PIC_INT_ENABLE), retVal);
  //return retVal;
  return READ_REG_U32(PIC_INT_ENABLE);
}

/**
 * For bits == 1 enable the interrupt.
 *
 * return the resulting value.
 */
void ac_interrupts_int_enable(ac_u32 bits) {
  //return __atomic_or_fetch(REG_PTR_U32(PIC_INT_ENABLE), ~bits, __ATOMIC_ACQUIRE);
  WRITE_REG_U32(PIC_INT_ENABLE, bits);
}

/**
 * For bits == 1 disable the interrupt.
 *
 * return the resulting value.
 */
void ac_interrupts_int_disable(ac_u32 bits) {
  //return __atomic_and_fetch(REG_PTR_U32(PIC_INT_ENABLE), ~bits, __ATOMIC_ACQUIRE);
  WRITE_REG_U32(PIC_INT_ENABLE_CLR, bits);
}
