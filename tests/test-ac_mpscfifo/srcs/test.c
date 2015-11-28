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

#include <ac_inttypes.h>
#include <ac_mpscfifo.h>
#include <ac_mpscfifo_misc.h>


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

  ac_mpscfifo* pQ = ac_init_mpscfifo(&q, &stub);
  error |= TEST(pQ != AC_NULL, "expecting q");
  error |= TEST(pQ->phead == &stub, "phead not initialized");
  error |= TEST(pQ->ptail == &stub, "ptail not initialized");
  error |= TEST(pQ->phead->pnext == AC_NULL, "pStub->pnext not initialized");

  ac_msg *pStub = ac_deinit_mpscfifo(&q);
  error |= TEST(pStub == &stub, "pStub not retuned");
  error |= TEST(pQ->phead == AC_NULL, "phead not deinitialized");
  error |= TEST(pQ->ptail == AC_NULL, "ptail not deinitialized");

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

  ac_mpscfifo* pQ = ac_init_mpscfifo(&q, &stub);
  presult = ac_rmv_msg(pQ);
  error |= TEST(presult == AC_NULL, "expecting rmv from empty queue to return AC_NULL");

  msg.cmd = 1;
  ac_add_msg(pQ, &msg);
  error |= TEST(pQ->phead == &msg, "phead should point at msg");
  error |= TEST(pQ->ptail->pnext == &msg, "ptail->pnext should point at msg");

  presult = ac_rmv_msg(pQ);
  error |= TEST(presult != AC_NULL, "expecting Q is not empty");
  error |= TEST(presult != &msg, "expecting return msg to not have original address");
  error |= TEST(presult->cmd == 1, "expecting msg.cmd == 1");

  presult = ac_rmv_msg(pQ);
  error |= TEST(presult == AC_NULL, "expecting Q is empty");

  presult = ac_deinit_mpscfifo(&q);
  error |= TEST(presult == &msg, "expecting last stub to be address of msg");

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
