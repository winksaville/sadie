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

#include <ac_mpscfifo.h>

#include <ac_inttypes.h>
#include <ac_test.h>


/**
 * Test we can initialize and deinitialize ac_mpscfifo
 *
 * return !0 if an error.
 */
static int test_init_and_deinit_mpscfifo() {
  ac_bool error = AC_FALSE;
  ac_msg stub;
  ac_mpscfifo q;

  stub.cmd = -1;

  ac_mpscfifo* pq = ac_init_mpscfifo(&q, &stub);
  error |= AC_TEST(pq != AC_NULL);
  error |= AC_TEST(pq == &q);
  error |= AC_TEST(pq->phead == &stub);
  error |= AC_TEST(pq->ptail == &stub);
  error |= AC_TEST(pq->phead->pnext == AC_NULL);

  ac_msg *pStub = ac_deinit_mpscfifo(pq);
  error |= AC_TEST(pStub == &stub);
  error |= AC_TEST(pq->phead == AC_NULL);
  error |= AC_TEST(pq->ptail == AC_NULL);

  return error ? 1 : 0;
}

/**
 * Test we can add and remove from Q.
 *
 * return !0 if an error.
 */
static int test_add_rmv_msg() {
  ac_bool error = AC_FALSE;
  ac_msg stub;
  ac_msg msg;
  ac_mpscfifo q;
  ac_msg* presult;

  stub.cmd = -1;

  ac_mpscfifo* pq = ac_init_mpscfifo(&q, &stub);
  presult = ac_rmv_msg(pq);
  error |= AC_TEST(presult == AC_NULL);

  msg.cmd = 1;
  ac_add_msg(pq, &msg);
  error |= AC_TEST(pq->phead == &msg);
  error |= AC_TEST(pq->ptail->pnext == &msg);

  presult = ac_rmv_msg(pq);
  error |= AC_TEST(presult != AC_NULL);
  error |= AC_TEST(presult != &msg);
  error |= AC_TEST(presult->cmd == 1);

  presult = ac_rmv_msg(pq);
  error |= AC_TEST(presult == AC_NULL);

  presult = ac_deinit_mpscfifo(&q);
  error |= AC_TEST(presult == &msg);

  return error ? 1 : 0;
}

int main(void) {
  int result = 0;

  result |= test_init_and_deinit_mpscfifo();
  result |= test_add_rmv_msg();

  if (result != 0) {
      // Failed
      ac_printf("ERR\n");
      return 1;
  } else {
      // Succeeded
      ac_printf("OK\n");
      return 0;
  }

  return result;
}
