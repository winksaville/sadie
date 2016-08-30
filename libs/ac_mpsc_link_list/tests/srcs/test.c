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

#include <ac_mpsc_link_list.h>
#include <ac_mpsc_link_list_dbg.h>

#include <ac_msg.h>
#include <ac_msg_pool.h>
#include <ac_inttypes.h>
#include <ac_test.h>


/**
 * Test we can initialize and deinitialize AcMpscLinkList *
 * return !0 if an error.
 */
AcBool test_init_and_deinit_mpscfifo() {
  AcBool error = AC_FALSE;
  AcMpscLinkList list;

  ac_printf("test_init_and_deinit:+list=%p\n", &list);

  // Initialize
  ac_printf("test_init_deinit: invoke init list=%p\n", &list);
  error |= AC_TEST(AcMpscLinkList_init(&list) == AC_STATUS_OK);
  AcMpscLinkList_print("test_init_deinit: initialized list:", &list);

  error |= AC_TEST(list.head == &list.stub);
  error |= AC_TEST(list.tail == &list.stub);
  error |= AC_TEST(list.stub.next == AC_NULL);

  // Deinitialize
  AcMpscLinkList_print("test_init_deinit: invoke deinit list:", &list);
  AcMpscLinkList_deinit(&list);
  ac_printf("test_init_deinit: deinitialized list=%p\n", &list);

  error |= AC_TEST(list.head == AC_NULL);
  error |= AC_TEST(list.tail == AC_NULL);

  ac_printf("test_init_and_deinit:-error=%d\n", error);

  return error;
}

/**
 * Test we can add and remove msgs from a FIFO.
 *
 * return !0 if an error.
 */
AcBool test_add_rmv(void) {
  AcBool error = AC_FALSE;
  AcStatus status;
  AcMpscLinkList list;
  const ac_u32 data_size = 2;

  ac_printf("test_add_rmv:+list=%p\n", &list);

  // Initialize
  status = AcMpscLinkList_init(&list);
  AcMpscLinkList_print("test_add_rmv_ac_mem list:", &list);
  error |= AC_TEST(status == AC_STATUS_OK);

  // Create a message pool
  AcMsgPool pool;
  status = AcMsgPool_init(&pool, 2, data_size);

  // Add msg1
  AcMsg* msg1 = AcMsgPool_get_msg(&pool);
  error |= AC_TEST(msg1 != AC_NULL);
  msg1->extra[0] = 1;
  msg1->extra[1] = 2;
  AcMpscLinkList_add(&list, msg1);
  AcMpscLinkList_print("test_add_rmv: after add msg1 list:", &list);
  error |= AC_TEST(list.head->msg == msg1);
  error |= AC_TEST(list.head->next == AC_NULL);
  error |= AC_TEST(list.tail->next->msg == msg1);

  // Add msg2
  AcMsg* msg2 = AcMsgPool_get_msg(&pool);
  error |= AC_TEST(msg2 != AC_NULL);
  msg2->extra[0] = 3;
  msg2->extra[1] = 4;
  AcMpscLinkList_add(&list, msg2);
  AcMpscLinkList_print("test_add_rmv: after add msg2 list:", &list);
  error |= AC_TEST(list.head->msg == msg2);
  error |= AC_TEST(list.head->next == AC_NULL);
  error |= AC_TEST(list.tail->next->msg == msg1);

  // Remove msg1
  AcMsg* msg = AcMpscLinkList_rmv(&list);
  error |= AC_TEST(msg == msg1);
  error |= AC_TEST(msg1->extra[0] == 1);
  error |= AC_TEST(msg1->extra[1] == 2);
  AcMpscLinkList_print("test_add_rmv: after rmv msg1 list:", &list);

  // Remove msg2
  msg = AcMpscLinkList_rmv(&list);
  error |= AC_TEST(msg == msg2);
  error |= AC_TEST(msg2->extra[0] == 3);
  error |= AC_TEST(msg2->extra[1] == 4);
  AcMpscLinkList_print("test_add_rmv: after rmv msg1 list:", &list);

  // Remove from empty which should be null
  msg = AcMpscLinkList_rmv(&list);
  error |= AC_TEST(msg == AC_NULL);

  // Deinitialize
  AcMpscLinkList_deinit(&list);

  AcMsgPool_deinit(&pool);

  ac_printf("test_add_rmv:-error=%d\n", error);
  return error;
}

int main(void) {
  AcBool error = AC_FALSE;

  error |= test_init_and_deinit_mpscfifo();
  ac_printf("\n");
  error |= test_add_rmv();
  ac_printf("\n");

  if (!error) {
    // Succeeded
    ac_printf("OK\n");
  }

  return error;
}
