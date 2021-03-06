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

#ifndef SADIE_ARCH_X86_LIBS_AC_KEYBOARD_IMPL_INCS_KEYBOARD_IMPL_H
#define SADIE_ARCH_X86_LIBS_AC_KEYBOARD_IMPL_INCS_KEYBOARD_IMPL_H

#include <ac_inttypes.h>
#include <io_x86.h>
#include <native_x86.h>
#include <thread_x86.h>
#include <ac_tsc.h>

extern ac_bool channel1_enabled;
extern ac_sint channel1_device_type;
extern ac_bool channel2_enabled;
extern ac_sint channel2_device_type;

#define ONE_MS 1000000
#define TEN_MS (10 * ONE_MS)
#define ONE_SEC (1000 * ONE_MS)

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_CMD_PORT 0x64

#define CFG_RD_ONLY_MASK 0x8C

#define CFG_PORT1_DISABLED 0x00
#define CFG_PORT1_ENABLED  0x01
#define CFG_PORT2_DISABLED 0x00
#define CFG_PORT2_ENABLED  0x02
#define CFG_SYSTEM_FLAG_MASK 0x04

#define CFG_PORT1_CLOCK_DISABLED 0x00
#define CFG_PORT1_CLOCK_ENABLED  0x10
#define CFG_PORT2_CLOCK_DISABLED 0x00
#define CFG_PORT2_CLOCK_ENABLED  0x20
#define CFG_PORT1_TRANSLATION_DISABLED 0x00
#define CFG_PORT1_TRANSLATION_ENABLED 0x40

/**
 * Read keyboard status port
 *
 * @return status
 */
static inline ac_u8 keyboard_rd_status() {
  //io_delay();
  ac_u8 status = io_rd_u8(KEYBOARD_STATUS_PORT);
  //ac_printf("keyboard_rd_status:-=0x%x\n", status);
  return status;
}

/**
 * Read keyboard data port without waiting for data ready
 *
 * @return data
 */
static inline ac_u8 keyboard_rd_data_now() {
  //io_delay();
  ac_u8 data = io_rd_u8(KEYBOARD_DATA_PORT);
  //ac_printf("keyboard_rd_data_now:-0x%x\n", data);
  return data;
}

/**
 * Read keyboard data port waiting for data to be ready
 *
 * @return data
 */
static inline ac_u8 keyboard_rd_data() {
  //ac_printf("keyboard_rd_data: status=0x%x\n", keyboard_rd_status());
  while ((keyboard_rd_status() & 0x1) == 0x0);
  ac_u8 data = keyboard_rd_data_now();
  //ac_printf("keyboard_rd_data:-0x%x\n", data);
  return data;
}

// 10ms poll delay
#define POLL_DELAY (TEN_MS)

/**
 * Read keyboard data port waiting for data to be ready upto timeout
 *
 * @return -1 if timedout or the data
 */
static inline ac_sint keyboard_rd_data_with_timeout(ac_u64 ns) {
  ac_u8 status = keyboard_rd_status();
  //ac_printf("keyboard_rd_data_with_timeout:+status=0x%x timeout=%ld\n", keyboard_rd_status(), ns);
  ac_bool ready = (status & 0x1) == 0x1;

  while (!ready && (ns > 0)) {
    //ac_printf("keyboard_rd_data_with_timeout: status=0x%x waiting timeout=%ld\n", keyboard_rd_status(), ns);
    ac_thread_wait_ns(POLL_DELAY);
    ns = ns > POLL_DELAY ? ns - POLL_DELAY : 0;
    status = keyboard_rd_status();
    ready = (status & 0x1) == 0x1;
  }

  ac_sint data;
  if (!ready) {
    data = -1;
  } else {
    data = keyboard_rd_data_now();
  }
  //ac_printf("keyboard_rd_data_with_timeout:-0x%x\n", data);
  return data;
}

/**
 * Write to keyboard data port after waiting for it
 * to be ready to accept the new data.
 */
static inline void keyboard_wr_data(ac_u8 data) {
  //ac_printf("keyboard_wr_data: status=0x%x\n", keyboard_rd_status());
  while ((keyboard_rd_status() & 0x2) == 0x2);
  //io_delay();
  io_wr_u8(KEYBOARD_DATA_PORT, data);
  //ac_printf("keyboard_wr_data:-status=0x%x\n", keyboard_rd_status());
}

/**
 * Write to the keyboard command port. There is no status
 * port for commands so this never waits.
 */
static inline void keyboard_wr_cmd(ac_u8 cmd) {
  //ac_printf("keyboard_wr_cmd:+0x%x\n", cmd);
  while ((keyboard_rd_status() & 0x2) == 0x2);
  io_wr_u8(KEYBOARD_CMD_PORT, cmd);
  //ac_printf("keyboard_wr_cmd:-\n");
}

/**
 * Read keyboard configuration. This is a blocking operation
 *
 * @return configuration
 */
static inline ac_u8 keyboard_rd_config(void) {
//static inline ac_u8 keyboard_rd_config(void) {
  //ac_printf("keyboard_rd_config:\n");
  keyboard_wr_cmd(0x20);
  ac_u8 config = keyboard_rd_data();
  //ac_printf("keyboard_rd_config:-0x%x\n", config);
  return config;
}

/**
 * Write keyboard configuration. This is a blocking operation
 */
static inline void keyboard_wr_config(ac_u8 config) {
  //ac_printf("keyboard_wr_config:+config=0x%x\n", config);
  ac_u8 cur_config = keyboard_rd_config();
  config = (cur_config & CFG_RD_ONLY_MASK) | (config & ~CFG_RD_ONLY_MASK);
  //ac_printf("keyboard_wr_config: config=0x%x\n", config);
  keyboard_wr_cmd(0x60);
  keyboard_wr_data(config);
  //ac_printf("keyboard_wr_config:-config=0x%x\n", keyboard_rd_config());
}

/**
 * Reeate the controller output. This is a blocking operation.
 *
 * @return data
 */
static inline ac_u8 keyboard_rd_controller_output(void) {
  keyboard_wr_cmd(0xD0);
  //io_delay();
  return keyboard_rd_data();
}

/**
 * Write the controller output. This is a blocking operaiton.
 */
static inline void keyboard_wr_controller_output(ac_u8 data) {
  keyboard_wr_cmd(0xD1);
  //io_delay();
  keyboard_wr_cmd(data);
}

/**
 * Issue the Power On Self Test command. This is a blocking oerpation
 *
 * @return  AC_TRUE if test was successful.
 */
static inline ac_bool keyboard_post(void) {
  keyboard_wr_cmd(0xAA);
  ac_bool result = keyboard_rd_data() == 0x55;
  //ac_printf("keyboard_post:-%b\n", result);
  return result;
}

/**
 * Disable channel 1. This is a blocking operation.
 */
static inline void keyboard_disable_channel1(void) {
  //ac_printf("keyboard_disable_channel1:+\n");
  keyboard_wr_cmd(0xAD);
  //ac_u8 config = keyboard_rd_config();
  //ac_bool result = ((config & 0x10) == 0x10);
  //ac_printf("keyboard_disable_channel1:-%b\n", result);
}

/**
 * Enable channel 1. This is a blocking operation.
 *
 * @return  AC_TRUE if channel 1 was enabled
 */
static inline ac_bool keyboard_enable_channel1(void) {
  //ac_printf("keyboard_enable_channel1:+\n");
  keyboard_wr_cmd(0xAE);
  ac_u8 config = keyboard_rd_config();
  ac_bool result = ((config & 0x10) == 0x00);
  //ac_printf("keyboard_enable_channel1:-%d config=%x\n", result, config);
  return result;
}

/**
 * Disable channel 2.  This is a blocking operation.
 */
static inline void keyboard_disable_channel2(void) {
  //ac_printf("keyboard_disable_channel2:+\n");
  keyboard_wr_cmd(0xA7);
  //ac_u8 config = keyboard_rd_config();
  //ac_bool result = ((config & 0x20) == 0x20);
  //ac_printf("keyboard_disable_channel2:-%b\n", result);
}

/**
 * Enable channel 2. This is a blocking operation.
 *
 * @return  AC_TRUE if channel 2 was enabled
 */
static inline ac_bool keyboard_enable_channel2(void) {
  //ac_printf("keyboard_enable_channel2:+\n");
  keyboard_wr_cmd(0xA8);
  ac_u8 config = keyboard_rd_config();
  ac_bool result = ((config & 0x20) == 0x00);
  //ac_printf("keyboard_enable_channel2:-%b\n", result);
  return result;
}

/**
 * Determine if there are 2 channels. This is a blocking operation.
 *
 * @return  AC_TRUE if there are 2 channels but second channel is disabled
 */
static inline ac_bool keyboard_has_dual_channels(void) {
  ac_bool result = keyboard_enable_channel2();
  if (result) {
    keyboard_disable_channel2();
  }
  //ac_printf("keyboard_has_dual_channels:-%b\n", result);
  return result;
}

/**
 * Test Channel1. This is a blocking operation.
 *
 * @return AC_TRUE if channel1 passes its test
 */
static inline ac_bool keyboard_test_channel1(void) {
  keyboard_wr_cmd(0xAB);
  ac_u8 data = keyboard_rd_data();
  ac_bool result = data == 0;
  //ac_printf("keyboard_test_channel1 data=0x%d result=%d\n", data, result);
  return result;
}

/**
* Test Channel2. This is a blocking operation.
 *
 * @return AC_TRUE if channel2 passes its test
 */
static inline ac_bool keyboard_test_channel2(void) {
  keyboard_wr_cmd(0xA9);
  ac_u8 data = keyboard_rd_data();
  ac_bool result = data == 0;
  //ac_printf("keyboard_test_channel2 data=0x%d result=%d\n", data, result);
  return result;
}

/**
 * Write data to device. This is a blocking operation.
 */
static inline void keyboard_send_device(ac_uint channel, ac_u8 data) {
  //ac_printf("keyboard_send_device:+channel=%d data=0x%x\n", channel, data);
  if (channel == 1) {
    keyboard_wr_data(data);
  } else {
    keyboard_wr_cmd(0xD4);
    keyboard_wr_data(data);
  }
  //ac_printf("keyboard_send_device:-channel=%d data=0x%x\n", channel, data);
}

/**
 * Write data to device and wait for ACK or timeout. This is a blocking operation.
 */
ac_sint keyboard_send_device_get_response(ac_uint channel, ac_u64 timeout_ns, ac_u8 data);

/**
 * Reset device. This is a blocking operation.
 *
 * @param channel to send reset too ether 1 or 2
 */
ac_bool keyboard_reset_device(ac_uint channel);

/**
 * Identify device. This is a blocking operation.
 *
 * @apram channel to identify device
 *
 * @return -1 if unknown
 */
ac_sint keyboard_get_device_type(ac_u8 channel);

/**
 * Enable keyboard interrupts
 */
void keyboard_intr_enable(void);

/**
 * Disable keyboard interrupts
 */
void keyboard_intr_disable(void);

#endif
