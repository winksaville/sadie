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
  error |= AC_TEST(pd != AC_NULL, "expecting pd != AC_NULL");
  if (pd != AC_NULL) {
    // We can request to remove a non-existant ac and null must be
    // returned. This must return AC_NULL because nothing has been
    // added.
    error |= AC_TEST(ac_dispatcher_rmv_acq(pd, (ac*)0x10000) == AC_NULL,
        "expecting AC_NULL");

    // Any use to pd after this is ILLEGAL
    ac_dispatcher_ret(pd);
  }

  // Attempting to return AC_NULL should be a NOP
  ac_dispatcher_ret(AC_NULL);

  return error ? 1 : 0;
}

///**
// * Test dispatching a message
// *
// * return !0 if an error.
// */
//static int test_dispatcher_add_rmv_acq() {
//  ac_bool error = AC_FALSE;
//
//  ac_dispatcher* pd = ac_dispatcher_get(10);
//  error |= AC_TEST(pd != AC_NULL, "expecting pd != AC_NULL");
//
//  ackk
//
//  if (pd != AC_NULL) {
//    // max_count must == 10, although pd->max_count should not be used
//    // or modified. In the future pd may be opaque.
//    error |= AC_TEST(pd->max_count == 10, "expecting max_count == 10");
//
//    // We can request to remove a non-existant ac and null must be
//    // returned. This must return AC_NULL because nothing has been
//    // added.
//    error |= AC_TEST(ac_dispatcher_rmv_acq(pd, (ac*)0x10000) == AC_NULL,
//        "expecting AC_NULL");
//
//    // Any use to pd after this is ILLEGAL
//    ac_dispatcher_ret(pd);
//  }
//
//  // Attempting to return AC_NULL should be a NOP
//  ac_dispatcher_ret(AC_NULL);
//
//  return error ? 1 : 0;
//}
//
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
//  error |= AC_TEST(presult == AC_NULL, "expecting rmv from empty queue to return AC_NULL");
//
//  msg.cmd = 1;
//  ac_add_msg(pq, &msg);
//  error |= AC_TEST(pq->phead == &msg, "phead should point at msg");
//  error |= AC_TEST(pq->ptail->pnext == &msg, "ptail->pnext should point at msg");
//
//  presult = ac_rmv_msg(pq);
//  error |= AC_TEST(presult != AC_NULL, "expecting Q is not empty");
//  error |= AC_TEST(presult != &msg, "expecting return msg to not have original address");
//  error |= AC_TEST(presult->cmd == 1, "expecting msg.cmd == 1");
//
//  presult = ac_rmv_msg(pq);
//  error |= AC_TEST(presult == AC_NULL, "expecting Q is empty");
//
//  presult = ac_deinit_mpscfifo(&q);
//  error |= AC_TEST(presult == &msg, "expecting last stub to be address of msg");
//
//  return error ? 1 : 0;
//}

int main(void) {
  int result = 0;

  result |= test_dispatcher_get_ret();

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
