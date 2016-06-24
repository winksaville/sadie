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

#include <ac_mpscfifo.h>
#include <ac_mpscfifo_dbg.h>

#include <ac_msg.h>
#include <ac_inttypes.h>
#include <ac_test.h>


/**
 * Test we can initialize and deinitialize ac_mpscfifo *
 * return !0 if an error.
 */
static ac_bool test_init_and_deinit_mpscfifo() {
  ac_bool error = AC_FALSE;
  ac_mpscfifo q;

  // Initialize
  ac_mpscfifo_init(&q);
  ac_printf("test_init_deinit: init\n");
  ac_mpscfifo_print(&q);

  error |= AC_TEST(q.phead != AC_NULL);
  error |= AC_TEST(q.ptail != AC_NULL);
  error |= AC_TEST(q.phead->pnext == AC_NULL);

  // Deinitialize
  ac_mpscfifo_deinit(&q);
  error |= AC_TEST(q.phead == AC_NULL);
  error |= AC_TEST(q.ptail == AC_NULL);

  return error;
}

/**
 * Test we can add and remove msgs from a FIFO.
 *
 * return !0 if an error.
 */
static ac_bool test_add_rmv_msg() {
  ac_bool error = AC_FALSE;
  ac_mpscfifo q;
  AcMsg* presult;

  // Initialize
  ac_mpscfifo_init(&q);

  // Add msg1
  AcMsg msg1 = { .arg1 = 1, .arg2 = 0x111 };
  ac_msg_print("test_add_rmv_msg: msg1=", &msg1);
  ac_mpscfifo_add_msg(&q, &msg1);
  ac_printf("test_add_rmv_msg: add msg1\n");
  ac_mpscfifo_print(&q);
  error |= AC_TEST(q.phead == &msg1);
  error |= AC_TEST(q.phead->pnext == AC_NULL);
  error |= AC_TEST(q.ptail->pnext == &msg1);

  // Add msg2
  AcMsg msg2 = { .arg1 = 2, .arg2 = 0x222 };
  ac_mpscfifo_add_msg(&q, &msg2);
  ac_printf("test_add_rmv_msg: add msg2\n");
  ac_mpscfifo_print(&q);
  error |= AC_TEST(q.phead == &msg2);
  error |= AC_TEST(q.phead->pnext == AC_NULL);
  error |= AC_TEST(q.ptail->pnext == &msg1);

  // Remove msg1
  presult = ac_mpscfifo_rmv_msg(&q);
  ac_printf("test_add_rmv_msg: 1\n");
  ac_msg_print("presult:   ", presult);
  ac_mpscfifo_print(&q);

  error |= AC_TEST(presult != AC_NULL);
  error |= AC_TEST(presult->pnext == AC_NULL);
  error |= AC_TEST(presult->arg1 == 1);
  error |= AC_TEST(presult->arg2 == 0x111);

  // Remove msg2
  presult = ac_mpscfifo_rmv_msg(&q);
  ac_printf("test_add_rmv_msg: 2\n");
  ac_msg_print("presult:   ", presult);
  ac_mpscfifo_print(&q);

  error |= AC_TEST(presult != AC_NULL);
  error |= AC_TEST(presult->pnext == AC_NULL);
  error |= AC_TEST(presult->arg1 == 2);
  error |= AC_TEST(presult->arg2 == 0x222);

  // Remove from empty
  presult = ac_mpscfifo_rmv_msg(&q);
  error |= AC_TEST(presult == AC_NULL);

  // Deinitialize
  ac_mpscfifo_deinit(&q);

  return error;
}

/**
 * Test we can add and remove raw msgs from a FIFO.
 *
 * return !0 if an error.
 */

static ac_bool test_add_rmv_msg_raw() {
  ac_bool error = AC_FALSE;
  ac_mpscfifo q;
  AcMsg* presult;

  // Initialize
  ac_mpscfifo_init(&q);

  // Add msg1
  AcMsg msg1 = { .arg1 = 1, .arg2 = 0x111 };
  ac_printf("test_add_rmv_msg_raw: add msg1\n");
  ac_mpscfifo_add_msg(&q, &msg1);
  ac_mpscfifo_print(&q);
  error |= AC_TEST(q.phead == &msg1);
  error |= AC_TEST(q.phead->pnext == AC_NULL);
  error |= AC_TEST(q.ptail->pnext == &msg1);

  // Add msg2
  AcMsg msg2 = { .arg1 = 2, .arg2 = 0x222 };
  ac_mpscfifo_add_msg(&q, &msg2);
  ac_printf("test_add_rmv_msg_raw: add msg2\n");
  ac_mpscfifo_print(&q);
  error |= AC_TEST(q.phead == &msg2);
  error |= AC_TEST(q.phead->pnext == AC_NULL);
  error |= AC_TEST(q.ptail->pnext == &msg1);

  // Add msg3
  AcMsg msg3 = { .arg1 = 3, .arg2 = 0x333 };
  ac_mpscfifo_add_msg(&q, &msg3);
  ac_printf("test_add_rmv_msg_raw: add msg3\n");
  ac_mpscfifo_print(&q);
  error |= AC_TEST(q.phead == &msg3);
  error |= AC_TEST(q.phead->pnext == AC_NULL);
  error |= AC_TEST(q.ptail->pnext == &msg1);

  // Remove stub
  presult = ac_mpscfifo_rmv_msg_raw(&q);
  ac_printf("test_add_rmv_msg_raw: stub\n");
  ac_msg_print("presult:   ", presult);
  ac_mpscfifo_print(&q);

  error |= AC_TEST(presult != AC_NULL);

  // Remove msg1
  presult = ac_mpscfifo_rmv_msg_raw(&q);
  ac_printf("test_add_rmv_msg_raw: 1\n");
  ac_msg_print("presult:   ", presult);
  ac_mpscfifo_print(&q);

  error |= AC_TEST(presult != AC_NULL);
  error |= AC_TEST(presult->arg1 == 1);
  error |= AC_TEST(presult->arg2 == 0x111);

  // Remove msg2
  presult = ac_mpscfifo_rmv_msg_raw(&q);
  ac_printf("test_add_rmv_msg_raw: 2\n");
  ac_msg_print("presult:   ", presult);
  ac_mpscfifo_print(&q);

  error |= AC_TEST(presult != AC_NULL);
  error |= AC_TEST(presult->arg1 == 2);
  error |= AC_TEST(presult->arg2 == 0x222);

  // Remove from empty
  presult = ac_mpscfifo_rmv_msg(&q);
  error |= AC_TEST(presult == AC_NULL);

  // Deinit
  ac_mpscfifo_deinit(&q);

  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  error |= test_init_and_deinit_mpscfifo();
  error |= test_add_rmv_msg();
  error |= test_add_rmv_msg_raw();

  if (!error) {
    // Succeeded
    ac_printf("OK\n");
  }

  return error;
}
