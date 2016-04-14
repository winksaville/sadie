/*
 * copyright 2016 wink saville
 *
 * licensed under the apache license, version 2.0 (the "license");
 * you may not use this file except in compliance with the license.
 * you may obtain a copy of the license at
 *
 *     http://www.apache.org/licenses/license-2.0
 *
 * unless required by applicable law or agreed to in writing, software
 * distributed under the license is distributed on an "as is" basis,
 * without warranties or conditions of any kind, either express or implied.
 * see the license for the specific language governing permissions and
 * limitations under the license.
 */

/**
 * This will be using the PS/2 controller for now, eventually
 * we'll need to support keyboard via USB but thats for the
 * future. For PS/2 [see](http://wiki.osdev.org/%228042%22_PS/2_Controller)
 *
 * Hardware info:
 *   [computer-enginerring.org](http://www.computer-engineering.org/ps2keyboard/)
 *
 * [Scan code table](http://www.computer-engineering.org/ps2keyboard/scancodes2.html)
 */

#include <ac_keyboard_impl.h>

#include <io_x86.h>
#include <interrupts_x86.h>

#include <ac_thread.h>
#include <ac_tsc.h>
#include <ac_inttypes.h>
#include <ac_printf.h>

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

ac_bool channel1_enabled;
ac_sint channel1_device_type;
ac_bool channel2_enabled;
ac_sint channel2_device_type;

/**
 * Read keyboard status port
 *
 * @return status
 */
static inline ac_u8 keyboard_rd_status() {
  io_delay();
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
  io_delay();
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
  io_delay();
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
  io_delay();
  return keyboard_rd_data();
}

/**
 * Write the controller output. This is a blocking operaiton.
 */
static inline void keyboard_wr_controller_output(ac_u8 data) {
  keyboard_wr_cmd(0xD1);
  io_delay();
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
static inline ac_sint keyboard_send_device_get_response(ac_uint channel, ac_u64 timeout_ns, ac_u8 data) {
  ac_sint response;

  ac_u64 timeout_ticks = ac_tscrd() + ac_ns_to_ticks(timeout_ns);
  //ac_printf("keyboard_send_device_get_response:+channel=%d data=0x%x now=%ld timeout_ticks=%ld\n",
  //    channel, data, ac_tscrd(), timeout_ticks);
resend:
  while ((ac_sint)(timeout_ticks - ac_tscrd()) > 0ll) {
    //ac_printf("keyboard_send_device_get_response: channel=%d data=0x%x now=%ld timeout_ticks=%ld diff=%ld\n",
    //    channel, data, ac_tscrd(), timeout_ticks, timeout_ticks - ac_tscrd());
    keyboard_send_device(channel, data);
    while ((ac_sint)(timeout_ticks - ac_tscrd()) > 0ll) {
      response = keyboard_rd_data_with_timeout(ONE_MS);
      //ac_printf("keyboard_send_device_get_response: channel=%d data=0x%x response=0x%x now=%ld timeout_ticks=%ld diff=%ld\n",
      //    channel, data, response,ac_tscrd(), timeout_ticks, timeout_ticks - ac_tscrd());
      if (response == -1) {
        //ac_printf("keyboard_send_device_get_response: channel=%d data=0x%x TIMED OUT now=%ld\n", channel, data, ac_tscrd());
        continue;
      } else if (response == 0xfa) {
        //ac_printf("keyboard_send_device_get_response:-channel=%d data=0x%x OK ACK response=0x%x\n", channel, data, response);
        return response;
      } else if ((response == 0xff) || (response == 0x00)) {
        //ac_printf("keyboard_send_device_get_response: channel=%d data=0x%x ERROR, resend response=0x%x now=%ld\n", channel, data, response, ac_tscrd());
        goto resend;
      } else if (response == 0xfe) {
        //ac_printf("keyboard_send_device_get_response: channel=%d data=0x%x RESEND response=0x%x now=%ld\n", channel, data, response, ac_tscrd());
        goto resend;
      } else if ((response == 0xaa) || (response == 0xfc) || (response == 0xfd)) {
        if (data == 0xFF) {
          //ac_printf("keyboard_send_device_get_response:-channel=%d data=0x%x OK RESET response=0x%x\n", channel, data, response);
          return response;
        }
        continue;
      } else {
        //ac_printf("keyboard_send_device_get_response:-channel=%d data=0x%x UNKNOWN response=0x%x\n", channel, data, response);
        return response;
      }
    }
  }
  //ac_printf("keyboard_send_device_get_response:-channel=%d data=0x%x FULL TIME OUT\n", channel, data);
  return -1;
}

/**
 * Reset device. This is a blocking operation.
 *
 * @param channel to send reset too ether 1 or 2
 */
static inline ac_bool keyboard_reset_device(ac_uint channel) {
  //ac_printf("keyboard_reset_device:+channel=%d\n", channel);
  ac_sint response = keyboard_send_device_get_response(channel, 100 * ONE_MS, 0xff);
  if (response == -1) {
    return AC_FALSE;
  } else {
    if (response == 0xAA) {
      // According to the [TODO for Step 10](http://wiki.osdev.org/%228042%22_PS/2_Controller)
      // an 0xAA can be returned, in which case read the next byte.
      response = keyboard_rd_data();
    }
    ac_bool result = response == 0xFA;
    //ac_printf("keyboard_reset_device:-channel=%d data=0x%x result=%b\n", channel, response, result);
    return result;
  }
}

/**
 * Identify device. This is a blocking operation.
 *
 * @apram channel to identify device
 *
 * @return -1 if unknown
 */
static inline ac_sint keyboard_get_device_type(ac_u8 channel) {
  //ac_printf("keyboard_get_device_type:+channel=%d\n", channel);
  ac_sint result;

  // Send disable scanning command
  result = keyboard_send_device_get_response(channel, ONE_SEC, 0xf5);
  if (result == 0xFA) {
    // Send identity command
    result = keyboard_send_device_get_response(channel, ONE_SEC, 0xf2);
    if (result == 0xFA) {
      ac_u8 data = keyboard_rd_data_with_timeout(TEN_MS);
      if (data == 0xAB) {
        data = keyboard_rd_data_with_timeout(TEN_MS);
        result = (ac_sint)data;
      } else {
        result = -1;
      }
    }
    // Renable scanning
    keyboard_send_device_get_response(channel, ONE_SEC, 0xf4);
  } else {
    result = -1;
  }

  //ac_printf("keyboard_get_device_type:-channel=%d result=0x%x\n", channel, result);
  return result;
}

/**
 * Non blocking read of a character from keyboard
 *
 * @param keyboard_id, 0 is the default keyboard
 *
 * @return -1 if no character
 */
ac_sint ac_keyboard_rd_non_blocking(void) {
  return keyboard_rd_data_with_timeout(0);
}

/**
 * Read of a character from keyboard, block if none available
 *
 * @return -1 if no character
 */
ac_u8 ac_keyboard_rd(void) {
  return keyboard_rd_data();
}

/**
 * Initialize this module early, must be
 * called before keyboard_init
 */
__attribute__((__constructor__))
void ac_keyboard_early_init(void) {
  ac_printf("ac_keyboard_early_init:+\n");

  // Step 1: Initialize USB Controllers
  //ac_printf("Step 1: Initialzie UBS Controllers, SKIPPING for now\n");

  // Step 2: Determine if PS/2 controller exits
  //ac_printf("Step 2: Use ACPI to determine if PS/2 controller exists, SKIPPING for now\n");

  // Step 3: Disable devices
  //ac_printf("Step 3: Disable devices\n");
  keyboard_disable_channel1();
  keyboard_disable_channel2();

  // Step 4: Flush the output buffer
  //ac_printf("Step 4: Flush the output buffer\n");
  keyboard_rd_data_with_timeout(ONE_MS);

  // Step 5: Set the Controller Configuration
  //ac_printf("Step 5: Set the Controller Configuration\n");
  //ac_printf("  cur config=0x%x\n", keyboard_rd_config());
  keyboard_wr_config(
      CFG_PORT1_DISABLED | CFG_PORT2_DISABLED |
      CFG_PORT1_CLOCK_DISABLED | CFG_PORT2_CLOCK_DISABLED |
      CFG_PORT1_TRANSLATION_DISABLED);
  //ac_printf("  new config=0x%x\n", keyboard_rd_config());

  // Step 6: Perform Controller Self Test
  //ac_printf("Step 6: keyboard_post=%b\n", keyboard_post());

  // Step 7: Determine if there are 2 Channels
  ac_bool has_dual_channels = keyboard_has_dual_channels();
  //ac_printf("Step 7: keyboard_has_dual_channels=%b\n", has_dual_channels);

  // Step 8: Test Ports
  ac_bool test_channel1 = keyboard_test_channel1();
  ac_bool test_channel2 = has_dual_channels ? keyboard_test_channel2() : AC_FALSE;
  has_dual_channels = test_channel2;
  //ac_printf("Step 8: has_dual_channels=%b test channel1=%b test_channel2=%b\n",
  //    has_dual_channels, test_channel1, test_channel2);

  // Step 9: Enable the working ports, double checking they got enabled.
  channel1_enabled = test_channel1 ? keyboard_enable_channel1() : AC_FALSE;
  channel2_enabled = test_channel2 ? keyboard_enable_channel2() : AC_FALSE;
  //ac_printf("Step 9: channel1_enabled=%b channel2_enabled=%b\n",
  //    channel1_enabled, channel2_enabled);

  // Step 10: Reset devices
  ac_bool channel1_device_reset = channel1_enabled ? keyboard_reset_device(1) : AC_FALSE;
  ac_bool channel2_device_reset = channel2_enabled ? keyboard_reset_device(2) : AC_FALSE;
  //ac_printf("Step 10: channel1_device_reset=%b channel2_device_reset=%b\n",
  //    channel1_device_reset, channel2_device_reset);

  // Step 11: Get the device type
  channel1_device_type = channel1_device_reset ? keyboard_get_device_type(1) : -1;
  channel2_device_type = channel2_device_reset ? keyboard_get_device_type(2) : -1;
  //ac_printf("Step 11: channel1_device_type=%x channel2_device_type=%x\n",
  //    channel1_device_type, channel2_device_type);

  // Step 12: Flush buffers
  const ac_uint max_flush = 20;
  ac_uint cnt_flush;
  for (cnt_flush = 0; cnt_flush < max_flush; cnt_flush++) {
    ac_sint data = keyboard_rd_data_with_timeout(TEN_MS);
    if (data == -1) {
      // Timeout, we're done
      break;
    }
  }
  ac_printf("ac_keyboard_early_init: cnt_flush=%d\n", cnt_flush);

  ac_printf("ac_keyboard_early_init:-channel1_device_type=%x channel2_device_type=%x\n",
      channel1_device_type, channel2_device_type);
}
