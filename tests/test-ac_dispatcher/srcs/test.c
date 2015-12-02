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

#define NDEBUG

#include <ac_dispatcher.h>

#include <ac_inttypes.h>
#include <ac_mpscfifo.h>
#include <ac_debug_printf.h>
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
    error |= AC_TEST(ac_dispatcher_rmv_ac(pd, (ac*)0x10000) == AC_NULL);

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
  ac* pac1;
  ac_mpscfifo q1;
  ac_msg stub1;

  ac_mpscfifo* pq1 = ac_init_mpscfifo(&q1, &stub1);
  error |= AC_TEST(pq1 != AC_NULL);

  pac1 = ac_dispatcher_add_acq(pd, &ac1, &q1);
  error |= AC_TEST(pac1 != AC_NULL);

  // Test that adding a second acq fails because we
  // only are allowing 1
  ac_mpscfifo q2;
  ac* pac2 = ac_dispatcher_add_acq(pd, &ac1, &q2);
  error |= AC_TEST(pac2 == AC_NULL);

  // Test we can remove pac1 and then add it back
  pac2 = ac_dispatcher_rmv_ac(pd, pac1);
  error |= AC_TEST(pac2 == pac1);
  pac1 = ac_dispatcher_add_acq(pd, &ac1, &q1);
  error |= AC_TEST(pac1 != AC_NULL);

  // And finally remove so we leave with the pd empty
  pac2 = ac_dispatcher_rmv_ac(pd, pac1);
  error |= AC_TEST(pac2 == pac1);

  // Return the dispatcher
  ac_dispatcher_ret(pd);

  return error ? 1 : 0;
}


static ac_bool ac1_process_msg(ac* this, ac_msg* pmsg) {
  ac_bool error = AC_FALSE;

  ac_debug_printf("ac1_process_msg:+ pmsg->cmd=%d, pmsg->arg=%d\n",
      pmsg->cmd, pmsg->arg);

  error |= AC_TEST(pmsg->cmd == 1);
  error |= AC_TEST(pmsg->arg > 1);

  pmsg->arg = (ac_u32)error;

  ac_debug_printf("ac1_process_msg:- pmsg->cmd=%d, pmsg->arg=%d\n",
      pmsg->cmd, pmsg->arg);

  return AC_TRUE;
}

static void * ac1_get_data(ac* this) {
  return AC_NULL;
}

ac ac1 = {
  .process_msg = &ac1_process_msg,
  .get_data = &ac1_get_data,
};


/**
 * Test dispatching a message
 *
 * return !0 if an error.
 */
static int test_dispatching() {
  ac_bool error = AC_FALSE;
  ac_debug_printf("test_dispatching:+\n");

  // Get a dispatcher
  ac_dispatcher* pd = ac_dispatcher_get(1);
  error |= AC_TEST(pd != AC_NULL);

  // Add an acq
  ac* pac1;
  ac_mpscfifo ac1q;
  ac_msg stub1;

  ac_mpscfifo* pac1q = ac_init_mpscfifo(&ac1q, &stub1);
  error |= AC_TEST(pac1q == &ac1q);

  pac1 = ac_dispatcher_add_acq(pd, &ac1, &ac1q);
  error |= AC_TEST(pac1 != AC_NULL);

  // Initialize message and add it to queue
  ac_msg msg1 = {
    .cmd = 1,
    .arg = 2
  };
  ac_add_msg(pac1q, &msg1);


  ac_debug_printf("test_dispatching: dispatch now\n");
  ac_bool processed_msgs = ac_dispatch(pd);
  ac_debug_printf("test_dispatching: dispatch complete\n");
  error |= AC_TEST(processed_msgs == AC_TRUE);

  ac_msg msg2 = {
    .cmd = 1,
    .arg = 3
  };
  ac_add_msg(pac1q, &msg2);

  ac_debug_printf("test_dispatching: rmv_ac\n");
  ac* pac2 = ac_dispatcher_rmv_ac(pd, pac1);
  error |= AC_TEST(pac2 == pac1);

  ac_debug_printf("test_dispatching:- error=%d\n", error);
  return error ? 1 : 0;
}

int main(void) {
  int result = 0;

  result |= test_dispatcher_get_ret();
  result |= test_dispatcher_add_rmv_acq();
  result |= test_dispatching();

  if (result != 0) {
      // Failed, don't print anything as there is enough already
      //ac_printf("ERR\n");
      return 1;
  } else {
      // Succeeded
      ac_printf("OK\n");
      return 0;
  }

  return result;
}
