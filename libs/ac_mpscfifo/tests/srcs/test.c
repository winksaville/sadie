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
  ac_msg stub = { .cmd = 0xf, .pool = (void*)0xff, .pextra = (void*)0xfff, .arg = 0xffff };
  ac_mpscfifo* pq = ac_mpscfifo_init(&q, &stub);
  ac_printf("test_init_deinit: init\n");
  ac_mpscfifo_print(pq);

  error |= AC_TEST(pq != AC_NULL);
  error |= AC_TEST(pq == &q);
  error |= AC_TEST(pq->phead == &stub);
  error |= AC_TEST(pq->ptail == &stub);
  error |= AC_TEST(pq->phead->pnext == AC_NULL);

  // Deinitialize
  ac_msg *pstub = ac_mpscfifo_deinit(pq);
  error |= AC_TEST(pstub == &stub);
  error |= AC_TEST(pq->phead == AC_NULL);
  error |= AC_TEST(pq->ptail == AC_NULL);

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
  ac_msg* presult;

  // Initialize
  ac_msg stub = { .cmd = 0xf, .pool = (void*)0xff, .pextra = (void*)0xfff, .arg = 0xffff };
  ac_mpscfifo* pq = ac_mpscfifo_init(&q, &stub);

  // Add msg1
  ac_msg msg1 = { .cmd = 1, .pool = (void*)0x11, .pextra = (void*)0x111, .arg = 0x1111 };
  ac_mpscfifo_add_msg(pq, &msg1);
  ac_printf("test_add_rmv_msg: add msg1\n");
  ac_mpscfifo_print(pq);
  error |= AC_TEST(pq->phead == &msg1);
  error |= AC_TEST(pq->phead->pnext == AC_NULL);
  error |= AC_TEST(pq->ptail == &stub);
  error |= AC_TEST(pq->ptail->pnext == &msg1);

  // Add msg2
  ac_msg msg2 = { .cmd = 2, .pool = (void*)0x22, .pextra = (void*)0x222, .arg = 0x2222 };
  ac_mpscfifo_add_msg(pq, &msg2);
  ac_printf("test_add_rmv_msg: add msg2\n");
  ac_mpscfifo_print(pq);
  error |= AC_TEST(pq->phead == &msg2);
  error |= AC_TEST(pq->phead->pnext == AC_NULL);
  error |= AC_TEST(pq->ptail == &stub);
  error |= AC_TEST(pq->ptail->pnext == &msg1);

  // Remove msg1
  presult = ac_mpscfifo_rmv_msg(pq);
  ac_printf("test_add_rmv_msg: 1\n");
  ac_printf("presult:   ");
  ac_msg_print(presult);
  ac_mpscfifo_print(pq);

  error |= AC_TEST(presult != AC_NULL);
  error |= AC_TEST(presult->pnext == AC_NULL);
  error |= AC_TEST(presult->cmd == 1);
  error |= AC_TEST(presult->pool == (void*)0x11);
  error |= AC_TEST(presult->pextra == (void*)0x111);
  error |= AC_TEST(presult->arg == 0x1111);

  // Remove msg2
  presult = ac_mpscfifo_rmv_msg(pq);
  ac_printf("test_add_rmv_msg: 2\n");
  ac_printf("presult:   ");
  ac_msg_print(presult);
  ac_mpscfifo_print(pq);

  error |= AC_TEST(presult != AC_NULL);
  error |= AC_TEST(presult->pnext == AC_NULL);
  error |= AC_TEST(presult->cmd == 2);
  error |= AC_TEST(presult->pool == (void*)0x22);
  error |= AC_TEST(presult->pextra == (void*)0x222);
  error |= AC_TEST(presult->arg == 0x2222);

  // Remove from empty
  presult = ac_mpscfifo_rmv_msg(pq);
  error |= AC_TEST(presult == AC_NULL);

  // Deinitialize
  presult = ac_mpscfifo_deinit(&q);
  error |= AC_TEST(presult == &msg2);

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
  ac_msg* presult;

  // Initialize
  ac_msg stub = { .cmd = 0xf, .pool = (void*)0xff, .pextra = (void*)0xfff, .arg = 0xffff };
  ac_mpscfifo* pq = ac_mpscfifo_init(&q, &stub);

  // Add msg1
  ac_msg msg1 = { .cmd = 1, .pool = (void*)0x11, .pextra = (void*)0x111, .arg = 0x1111 };
  ac_mpscfifo_add_msg(pq, &msg1);
  ac_printf("test_add_rmv_msg_raw: add msg1\n");
  ac_mpscfifo_print(pq);
  error |= AC_TEST(pq->phead == &msg1);
  error |= AC_TEST(pq->phead->pnext == AC_NULL);
  error |= AC_TEST(pq->ptail == &stub);
  error |= AC_TEST(pq->ptail->pnext == &msg1);

  // Add msg2
  ac_msg msg2 = { .cmd = 2, .pool = (void*)0x22, .pextra = (void*)0x222, .arg = 0x2222 };
  ac_mpscfifo_add_msg(pq, &msg2);
  ac_printf("test_add_rmv_msg_raw: add msg2\n");
  ac_mpscfifo_print(pq);
  error |= AC_TEST(pq->phead == &msg2);
  error |= AC_TEST(pq->phead->pnext == AC_NULL);
  error |= AC_TEST(pq->ptail == &stub);
  error |= AC_TEST(pq->ptail->pnext == &msg1);

  // Remove stub
  presult = ac_mpscfifo_rmv_msg_raw(pq);
  ac_printf("test_add_rmv_msg_raw: stub 'f'\n");
  ac_printf("presult:   ");
  ac_msg_print(presult);
  ac_mpscfifo_print(pq);

  error |= AC_TEST(presult != AC_NULL);
  error |= AC_TEST(presult->cmd == 0xf);
  error |= AC_TEST(presult->pool == (void*)0xff);
  error |= AC_TEST(presult->pextra == (void*)0xfff);
  error |= AC_TEST(presult->arg == 0xffff);

  // Remove msg1
  presult = ac_mpscfifo_rmv_msg_raw(pq);
  ac_printf("test_add_rmv_msg_raw: 1\n");
  ac_printf("presult:   ");
  ac_msg_print(presult);
  ac_mpscfifo_print(pq);

  error |= AC_TEST(presult != AC_NULL);
  error |= AC_TEST(presult->cmd == 1);
  error |= AC_TEST(presult->pool == (void*)0x11);
  error |= AC_TEST(presult->pextra == (void*)0x111);
  error |= AC_TEST(presult->arg == 0x1111);

  // Remove from empty
  presult = ac_mpscfifo_rmv_msg(pq);
  error |= AC_TEST(presult == AC_NULL);

  // Deinit
  presult = ac_mpscfifo_deinit(&q);
  error |= AC_TEST(presult == &msg2);

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
