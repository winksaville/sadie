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

#include <ac_mpsc_fifo.h>
#include <ac_mpsc_fifo_dbg.h>

#include <ac_buff.h>
#include <ac_buff_dbg.h>
#include <ac_inttypes.h>
#include <ac_test.h>


/**
 * Test we can initialize and deinitialize AcMpscFifo *
 * return !0 if an error.
 */
static ac_bool test_init_and_deinit_mpscfifo() {
  ac_bool error = AC_FALSE;
  AcMpscFifo fifo;

  // Initialize
  error |= AC_TEST(AcMpscFifo_init(&fifo, 1) == AC_STATUS_OK);
  AcMpscFifo_print("test_init_deinit: fifo=", &fifo);

  error |= AC_TEST(fifo.head != AC_NULL);
  error |= AC_TEST(fifo.tail != AC_NULL);
  error |= AC_TEST(fifo.head->hdr.next == AC_NULL);

  // Deinitialize
  ac_printf("x\n");
  AcMpscFifo_deinit(&fifo);
  ac_printf("y\n");
  error |= AC_TEST(fifo.head == AC_NULL);
  error |= AC_TEST(fifo.tail == AC_NULL);

  ac_printf("-\n");
  return error;
}

/**
 * Test we can add and remove msgs from a FIFO.
 *
 * return !0 if an error.
 */
ac_bool test_add_rmv_msg() {
  ac_bool error = AC_FALSE;
  AcMpscFifo fifo;
  //AcBuff* result;
  ac_u32 data_size = 2;

  // Initialize
  AcMpscFifo_init(&fifo, data_size);

  // Add buff1
  AcBuff* buffs;
  AcBuff_alloc(&fifo, 2, data_size, data_size, &buffs);
  buffs[0].data[0] = 1;
  buffs[0].data[1] = 2;

  AcBuff_print("test_add_rmv_msg: buff[0]=", &buffs[0]);
#if 0
  AcMpscFifo_add_msg(&fifo, &buff1);
  ac_printf("test_add_rmv_msg: add buff1\n");
  AcMpscFifo_print(&fifo);
  error |= AC_TEST(fifo.head == &buff1);
  error |= AC_TEST(fifo.head->next == AC_NULL);
  error |= AC_TEST(fifo.tail->next == &buff1);

  // Add msg2
  AcBuff msg2 = { .arg1 = 2, .arg2 = 0x222 };
  AcMpscFifo_add_msg(&fifo, &msg2);
  ac_printf("test_add_rmv_msg: add msg2\n");
  AcMpscFifo_print(&fifo);
  error |= AC_TEST(fifo.head == &msg2);
  error |= AC_TEST(fifo.head->next == AC_NULL);
  error |= AC_TEST(fifo.tail->next == &buff1);

  // Remove buff1
  result = AcMpscFifo_rmv_msg(&fifo);
  ac_printf("test_add_rmv_msg: 1\n");
  AcBuff_print("result:   ", result);
  AcMpscFifo_print(&fifo);

  error |= AC_TEST(result != AC_NULL);
  error |= AC_TEST(result->next == AC_NULL);
  error |= AC_TEST(result->arg1 == 1);
  error |= AC_TEST(result->arg2 == 0x111);

  // Remove msg2
  result = AcMpscFifo_rmv_msg(&fifo);
  ac_printf("test_add_rmv_msg: 2\n");
  AcBuff_print("result:   ", result);
  AcMpscFifo_print(&fifo);

  error |= AC_TEST(result != AC_NULL);
  error |= AC_TEST(result->next == AC_NULL);
  error |= AC_TEST(result->arg1 == 2);
  error |= AC_TEST(result->arg2 == 0x222);

  // Remove from empty
  result = AcMpscFifo_rmv_msg(&fifo);
  error |= AC_TEST(result == AC_NULL);
#endif

  // Deinitialize
  AcMpscFifo_deinit(&fifo);

  return error;
}

/**
 * Test we can add and remove raw msgs from a FIFO.
 *
 * return !0 if an error.
 */

ac_bool test_add_rmv_msg_raw() {
  ac_bool error = AC_FALSE;
#if 0
  AcMpscFifo fifo;
  AcBuff* result;

  // Initialize
  AcMpscFifo_init(&fifo);

  // Add buff1
  AcBuff buff1 = { .arg1 = 1, .arg2 = 0x111 };
  ac_printf("test_add_rmv_msg_raw: add buff1\n");
  AcMpscFifo_add_msg(&fifo, &buff1);
  AcMpscFifo_print(&fifo);
  error |= AC_TEST(fifo.head == &buff1);
  error |= AC_TEST(fifo.head->next == AC_NULL);
  error |= AC_TEST(fifo.tail->next == &buff1);

  // Add msg2
  AcBuff msg2 = { .arg1 = 2, .arg2 = 0x222 };
  AcMpscFifo_add_msg(&fifo, &msg2);
  ac_printf("test_add_rmv_msg_raw: add msg2\n");
  AcMpscFifo_print(&fifo);
  error |= AC_TEST(fifo.head == &msg2);
  error |= AC_TEST(fifo.head->next == AC_NULL);
  error |= AC_TEST(fifo.tail->next == &buff1);

  // Remove stub
  result = AcMpscFifo_rmv_msg_raw(&fifo);
  ac_printf("test_add_rmv_msg_raw: stub\n");
  AcBuff_print("result:   ", result);
  AcMpscFifo_print(&fifo);

  error |= AC_TEST(result != AC_NULL);

  // Remove buff1
  result = AcMpscFifo_rmv_msg_raw(&fifo);
  ac_printf("test_add_rmv_msg_raw: 1\n");
  AcBuff_print("result:   ", result);
  AcMpscFifo_print(&fifo);

  error |= AC_TEST(result != AC_NULL);
  error |= AC_TEST(result->arg1 == 1);
  error |= AC_TEST(result->arg2 == 0x111);

  // Remove from empty
  result = AcMpscFifo_rmv_msg(&fifo);
  error |= AC_TEST(result == AC_NULL);

  // Deinit
  AcMpscFifo_deinit(&fifo);
#endif

  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  error |= test_init_and_deinit_mpscfifo();
  //error |= test_add_rmv_msg();
  //error |= test_add_rmv_msg_raw();

  if (!error) {
    // Succeeded
    ac_printf("OK\n");
  }

  return error;
}
