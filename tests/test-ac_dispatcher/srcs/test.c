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

#include <ac_dispatcher.h>

#include <ac_inttypes.h>
#include <ac_mpscfifo.h>
#include <ac_test.h>


/**
 * Test dispatcher get and return
 *
 * return !0 if an error.
 */
static int test_dispatcher_get_ret() {
  ac_bool error = AC_FALSE;

  ac_dispatcher* pd = ac_dispatcher_get(10);
  error |= AC_TEST(pd != AC_NULL);
  if (pd != AC_NULL) {
    // We can request to remove a non-existant ac and null must be
    // returned. This must return AC_NULL because nothing has been
    // added.
    error |= AC_TEST(ac_dispatcher_rmv_acq(pd, (ac*)0x10000) == AC_NULL);

    // Any use to pd after this is ILLEGAL
    ac_dispatcher_ret(pd);
  }

  // Return AC_NULL should be a NOP
  ac_dispatcher_ret(AC_NULL);

  return error ? 1 : 0;
}

/**
 * Test dispatching a message
 *
 * return !0 if an error.
 */
static int test_dispatcher_add_rmv_acq() {
  ac_bool error = AC_FALSE;

  // Get a dispatcher
  ac_dispatcher* pd = ac_dispatcher_get(1);
  error |= AC_TEST(pd != AC_NULL);

  // Add a acq
  ac ac1;
  ac_mpscfifo q;
  ac_msg stub;

  ac_mpscfifo* pq = ac_init_mpscfifo(&q, &stub);
  error |= AC_TEST(pq != AC_NULL);

  ac* pac = ac_dispatcher_add_acq(pd, &ac1, &q);
  error |= AC_TEST(pac != AC_NULL);

  return error ? 1 : 0;
}

///**
// * Test we can add and remove from Q.
// *
// * return !0 if an error.
// */
//static int test_add_rmv_msg() {
//  ac_bool error = AC_FALSE;
//  ac_msg stub;
//  ac_msg msg;
//  ac_mpscfifo q;
//  ac_msg* presult;
//
//  stub.cmd = -1;
//
//  ac_mpscfifo* pq = ac_init_mpscfifo(&q, &stub);
//  presult = ac_rmv_msg(pq);
//  error |= AC_TEST(presult == AC_NULL);
//
//  msg.cmd = 1;
//  ac_add_msg(pq, &msg);
//  error |= AC_TEST(pq->phead == &msg);
//  error |= AC_TEST(pq->ptail->pnext == &msg);
//
//  presult = ac_rmv_msg(pq);
//  error |= AC_TEST(presult != AC_NULL);
//  error |= AC_TEST(presult != &msg);
//  error |= AC_TEST(presult->cmd == 1);
//
//  presult = ac_rmv_msg(pq);
//  error |= AC_TEST(presult == AC_NULL);
//
//  presult = ac_deinit_mpscfifo(&q);
//  error |= AC_TEST(presult == &msg);
//
//  return error ? 1 : 0;
//}

int main(void) {
  int result = 0;

  result |= test_dispatcher_get_ret();
  result |= test_dispatcher_add_rmv_acq();

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
