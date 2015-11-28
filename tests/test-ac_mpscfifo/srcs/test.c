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
  error |= TEST(pQ->pHead == &stub, "pHead not initialized");
  error |= TEST(pQ->pTail == &stub, "pTail not initialized");
  error |= TEST(pQ->pHead->pNext == AC_NULL, "pStub->pNext not initialized");

  ac_msg *pStub = ac_deinit_mpscfifo(&q);
  error |= TEST(pStub == &stub, "pStub not retuned");
  error |= TEST(pQ->pHead == AC_NULL, "pHead not deinitialized");
  error |= TEST(pQ->pTail == AC_NULL, "pTail not deinitialized");

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
  ac_msg* pResult;

  stub.cmd = -1;

  ac_mpscfifo* pQ = ac_init_mpscfifo(&q, &stub);
  pResult = ac_rmv_msg(pQ);
  error |= TEST(pResult == AC_NULL, "expecting rmv from empty queue to return AC_NULL");

  msg.cmd = 1;
  ac_add_msg(pQ, &msg);
  error |= TEST(pQ->pHead == &msg, "pHead should point at msg");
  error |= TEST(pQ->pTail->pNext == &msg, "pTail->pNext should point at msg");

  pResult = ac_rmv_msg(pQ);
  error |= TEST(pResult != AC_NULL, "expecting Q is not empty");
  error |= TEST(pResult != &msg, "expecting return msg to not have original address");
  error |= TEST(pResult->cmd == 1, "expecting msg.cmd == 1");

  pResult = ac_rmv_msg(pQ);
  error |= TEST(pResult == AC_NULL, "expecting Q is empty");

  pResult = ac_deinit_mpscfifo(&q);
  error |= TEST(pResult == &msg, "expecting last stub to be address of msg");

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
