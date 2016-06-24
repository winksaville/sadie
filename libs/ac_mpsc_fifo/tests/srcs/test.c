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

  ac_printf("test_init_and_deinit:+fifo=%p\n", &fifo);

  // Initialize
  error |= AC_TEST(AcMpscFifo_init(&fifo, 1) == AC_STATUS_OK);
  AcMpscFifo_print("test_init_deinit:+fifo:", &fifo);

  error |= AC_TEST(fifo.head != AC_NULL);
  error |= AC_TEST(fifo.tail != AC_NULL);
  error |= AC_TEST(fifo.head->hdr.next == AC_NULL);

  // Deinitialize
  AcMpscFifo_deinit(&fifo);

  error |= AC_TEST(fifo.head == AC_NULL);
  error |= AC_TEST(fifo.tail == AC_NULL);

  ac_printf("test_init_and_deinit:-error=%d\n", error);

  return error;
}

/**
 * Test we can add and remove msgs from a FIFO.
 *
 * return !0 if an error.
 */
ac_bool test_add_rmv_ac_buff() {
  ac_bool error = AC_FALSE;
  AcStatus status;
  AcMpscFifo fifo;
  AcBuff* buff;
  ac_u32 data_size = 2;

  ac_printf("test_add_rmv_ac_buff:+fifo=%p\n", &fifo);

  // Initialize
  AcMpscFifo_init(&fifo, data_size);
  AcMpscFifo_print("test_add_rmv_ac_buff fifo:", &fifo);

  // Add buff1
  AcBuff* buffs;
  status = AcBuff_alloc(&fifo, 2, data_size, data_size, &buffs);
  error |= AC_TEST(status == 0);
  AcBuff_get_nth(buffs, 0)->data[0] = 1;
  AcBuff_get_nth(buffs, 0)->data[1] = 2;
  AcBuff_print("test_add_rmv_ac_buff: buffs[0]=", AcBuff_get_nth(buffs, 0));
  AcBuff_get_nth(buffs, 1)->data[0] = 3;
  AcBuff_get_nth(buffs, 1)->data[1] = 4;
  AcBuff_print("test_add_rmv_ac_buff: buffs[1]=", AcBuff_get_nth(buffs, 1));

  // Add first buff
  AcMpscFifo_add_ac_buff(&fifo, AcBuff_get_nth(buffs, 0));
  AcMpscFifo_print("test_add_rmv_ac_buff: after add buffs[0] fifo:", &fifo);
  error |= AC_TEST(fifo.head == AcBuff_get_nth(buffs, 0));
  error |= AC_TEST(fifo.head->hdr.next == AC_NULL);
  error |= AC_TEST(fifo.tail->hdr.next == AcBuff_get_nth(buffs, 0));

  // Add Second buff
  AcMpscFifo_add_ac_buff(&fifo, AcBuff_get_nth(buffs, 1));
  AcMpscFifo_print("test_add_rmv_ac_buff: after add buffs[1] fifo:", &fifo);
  error |= AC_TEST(fifo.head == AcBuff_get_nth(buffs, 1));
  error |= AC_TEST(fifo.head->hdr.next == AC_NULL);
  error |= AC_TEST(fifo.tail->hdr.next == AcBuff_get_nth(buffs, 0));

  // Remove first buff
  buff = AcMpscFifo_rmv_ac_buff(&fifo);
  AcBuff_print("test_add_rmv_ac_buff: after rmv buffs=", buff);
  error |= AC_TEST(buff != AC_NULL);
  error |= AC_TEST(buff->hdr.next == AC_NULL);
  error |= AC_TEST(buff->data[0] == 1);
  error |= AC_TEST(buff->data[1] == 2);
  AcMpscFifo_print("test_add_rmv_ac_buff: after rmv buffs[0] fifo:", &fifo);

  // Remove second buff
  buff = AcMpscFifo_rmv_ac_buff(&fifo);
  AcBuff_print("test_add_rmv_ac_buff: after rmv buffs=", buff);
  error |= AC_TEST(buff != AC_NULL);
  error |= AC_TEST(buff->hdr.next == AC_NULL);
  error |= AC_TEST(buff->data[0] == 3);
  error |= AC_TEST(buff->data[1] == 4);
  AcMpscFifo_print("test_add_rmv_ac_buff: after rmv buffs[1] fifo:", &fifo);

  // Remove from empty which should be null
  buff = AcMpscFifo_rmv_ac_buff(&fifo);
  error |= AC_TEST(buff == AC_NULL);

  // Deinitialize
  AcMpscFifo_deinit(&fifo);

  // Free buffs
  AcBuff_free(buffs);

  ac_printf("test_add_rmv_ac_buff:-error=%d\n", error);
  return error;
}

/**
 * Test we can add and remove raw msgs from a FIFO.
 *
 * return !0 if an error.
 */

ac_bool test_add_rmv_ac_buff_raw() {
  ac_bool error = AC_FALSE;
  AcStatus status;
  AcMpscFifo fifo;
  AcBuff* buff;
  ac_u32 data_size = 2;

  ac_printf("test_add_rmv_ac_buff:+fifo=%p\n", &fifo);

  // Initialize
  AcMpscFifo_init(&fifo, data_size);
  AcMpscFifo_print("test_add_rmv_ac_buff_raw: fifo:", &fifo);

  // Allocate 3 buffers
  AcBuff* buffs;
  status = AcBuff_alloc(&fifo, 3, data_size, data_size, &buffs);
  error |= AC_TEST(status == 0);
  AcBuff_get_nth(buffs, 0)->data[0] = 1;
  AcBuff_get_nth(buffs, 0)->data[1] = 2;
  AcBuff_print("test_add_rmv_ac_buff_raw: buffs[0]=", AcBuff_get_nth(buffs, 0));
  AcBuff_get_nth(buffs, 1)->data[0] = 3;
  AcBuff_get_nth(buffs, 1)->data[1] = 4;
  AcBuff_print("test_add_rmv_ac_buff_raw: buffs[1]=", AcBuff_get_nth(buffs, 1));
  AcBuff_get_nth(buffs, 2)->data[0] = 5;
  AcBuff_get_nth(buffs, 2)->data[1] = 6;
  AcBuff_print("test_add_rmv_ac_buff_raw: buffs[2]=", AcBuff_get_nth(buffs, 2));

  // Add first buff
  AcMpscFifo_add_ac_buff(&fifo, AcBuff_get_nth(buffs, 0));
  AcMpscFifo_print("test_add_rmv_ac_buff_raw: after add buffs[0] fifo:", &fifo);
  error |= AC_TEST(fifo.head == AcBuff_get_nth(buffs, 0));
  error |= AC_TEST(fifo.head->hdr.next == AC_NULL);
  error |= AC_TEST(fifo.tail->hdr.next == AcBuff_get_nth(buffs, 0));

  // Add second buff
  AcMpscFifo_add_ac_buff(&fifo, AcBuff_get_nth(buffs, 1));
  AcMpscFifo_print("test_add_rmv_ac_buff_raw: after add buffs[1] fifo:", &fifo);
  error |= AC_TEST(fifo.head == AcBuff_get_nth(buffs, 1));
  error |= AC_TEST(fifo.head->hdr.next == AC_NULL);
  error |= AC_TEST(fifo.tail->hdr.next == AcBuff_get_nth(buffs, 0));

  // Add third buff
  AcMpscFifo_add_ac_buff(&fifo, AcBuff_get_nth(buffs, 2));
  AcMpscFifo_print("test_add_rmv_ac_buff_raw: after add buffs[2] fifo:", &fifo);
  error |= AC_TEST(fifo.head == AcBuff_get_nth(buffs, 2));
  error |= AC_TEST(fifo.head->hdr.next == AC_NULL);
  error |= AC_TEST(fifo.tail->hdr.next == AcBuff_get_nth(buffs, 0));

  // First remove removes the stub since this is raw and the contents must be 0
  buff = AcMpscFifo_rmv_ac_buff_raw(&fifo);
  AcBuff_print("test_add_rmv_ac_buff_raw: first rmv buff=", buff);
  error |= AC_TEST(buff != AC_NULL);
  error |= AC_TEST(buff->hdr.next == AC_NULL);
  error |= AC_TEST(buff->hdr.user_size == 0);
  error |= AC_TEST(buff->data[0] == 0);
  error |= AC_TEST(buff->data[1] == 0);
  AcMpscFifo_print("test_add_rmv_ac_buff_raw: first rmv fifo:", &fifo);

  // Second remove removes the first buff, again because this is raw
  buff = AcMpscFifo_rmv_ac_buff_raw(&fifo);
  AcBuff_print("test_add_rmv_ac_buff_raw: second rmv buff=", buff);
  error |= AC_TEST(buff != AC_NULL);
  error |= AC_TEST(buff->hdr.next == AC_NULL);
  error |= AC_TEST(buff->hdr.user_size == 2);
  error |= AC_TEST(buff->data[0] == 1);
  error |= AC_TEST(buff->data[1] == 2);
  AcMpscFifo_print("test_add_rmv_ac_buff_raw: second rmv fifo:", &fifo);

  // Third remove removes the second buff, again because this is raw
  buff = AcMpscFifo_rmv_ac_buff_raw(&fifo);
  AcBuff_print("test_add_rmv_ac_buff_raw: third rmv buff=", buff);
  error |= AC_TEST(buff != AC_NULL);
  error |= AC_TEST(buff->hdr.next == AC_NULL);
  error |= AC_TEST(buff->hdr.user_size == 2);
  error |= AC_TEST(buff->data[0] == 3);
  error |= AC_TEST(buff->data[1] == 4);
  AcMpscFifo_print("test_add_rmv_ac_buff_raw: third rmv fifo:", &fifo);

  // Remove from empty which should be null
  buff = AcMpscFifo_rmv_ac_buff(&fifo);
  error |= AC_TEST(buff == AC_NULL);

  // Deinitialize
  AcMpscFifo_deinit(&fifo);

  // Free buffs
  AcBuff_free(buffs);

  ac_printf("test_add_rmv_ac_buff:-error=%d\n", error);
  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  error |= test_init_and_deinit_mpscfifo();
  error |= test_add_rmv_ac_buff();
  error |= test_add_rmv_ac_buff_raw();

  if (!error) {
    // Succeeded
    ac_printf("OK\n");
  }

  return error;
}
