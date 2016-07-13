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

//#define NDEBUG

#include "test-ac_dispatcher/incs/tests.h"

#include <ac_dispatcher.h>

#include <ac_inttypes.h>
#include <ac_debug_printf.h>
#include <ac_msg_pool.h>
#include <ac_test.h>


/**
 * Test dispatcher get and return
 *
 * return AC_TRUE if an error.
 */
static ac_bool test_dispatcher_get_ret() {
  ac_bool error = AC_FALSE;

  AcDispatcher* d = AcDispatcher_get(10);
  error |= AC_TEST(d != AC_NULL);
  if (d != AC_NULL) {
    AcDispatcher_ret(d);
  }

  // Return AC_NULL should be a NOP
  AcDispatcher_ret(AC_NULL);

  return error;
}

/**
 * Test dispatching a message
 *
 * return AC_TRUE if an error.
 */
static int test_dispatcher_add_rmv_acq() {
  ac_bool error = AC_FALSE;

  // Get a dispatcher
  AcDispatcher* pd = AcDispatcher_get(1);
  error |= AC_TEST(pd != AC_NULL);

  // Add a acq
  AcComp ac1;
  AcDispatchableComp* dc1;

  dc1 = AcDispatcher_add_comp(pd, &ac1);
  error |= AC_TEST(dc1 != AC_NULL);

  // Test that adding a second acq fails because we
  // only are allowing 1
  AcDispatchableComp* dc2 = AcDispatcher_add_comp(pd, &ac1);
  error |= AC_TEST(dc2 == AC_NULL);

  // Test we can remove pac1 and then add it back
  AcComp* pac2 = AcDispatcher_rmv_comp(pd, dc1);
  error |= AC_TEST(pac2 == &ac1);
  dc1 = AcDispatcher_add_comp(pd, &ac1);
  error |= AC_TEST(dc1 != AC_NULL);

  // And finally remove so we leave with the pd empty
  pac2 = AcDispatcher_rmv_comp(pd, dc1);
  error |= AC_TEST(pac2 == &ac1);

  // Return the dispatcher
  AcDispatcher_ret(pd);

  return error;
}


static ac_bool ac1_process_msg(AcComp* this, AcMsg* pmsg) {
  ac_bool error = AC_FALSE;

  ac_debug_printf("ac1_process_msg:+ pmsg->arg1=%ld, pmsg->arg2=%ld\n",
      pmsg->arg1, pmsg->arg2);

  error |= AC_TEST(pmsg->arg1 == 1);
  error |= AC_TEST(pmsg->arg2 > 1);

  pmsg->arg2 = error;

  ac_debug_printf("ac1_process_msg:- pmsg->arg1=%ld, pmsg->arg2=%ld\n",
      pmsg->arg1, pmsg->arg2);

  return AC_TRUE;
}

static AcComp ac1 = {
  .process_msg = &ac1_process_msg,
};


/**
 * Test dispatching a message
 *
 * return AC_TRUE if an error.
 */
static ac_bool test_dispatching() {
  ac_bool error = AC_FALSE;
  AcStatus status;
  ac_debug_printf("test_dispatching:+\n");

  // Get a dispatcher
  AcDispatcher* pd = AcDispatcher_get(1);
  error |= AC_TEST(pd != AC_NULL);

  // Add an acq
  AcDispatchableComp* dc1;

  dc1 = AcDispatcher_add_comp(pd, &ac1);
  error |= AC_TEST(dc1 != AC_NULL);

  // Initialize a msg pool
  AcMsgPool mp;
  status = AcMsgPool_init(&mp, 2);
  error |= AC_TEST(status == AC_STATUS_OK);

  // Get a msg and dispatch it
  AcMsg* msg1;
  status = AcMsgPool_get_msg(&mp, &msg1);
  error |= AC_TEST(status == AC_STATUS_OK);
  msg1->arg1 = 1;
  msg1->arg2 = 2;
  AcDispatcher_send_msg(dc1, msg1);


  ac_debug_printf("test_dispatching: dispatch now\n");
  ac_bool processed_msgs = AcDispatcher_dispatch(pd);
  ac_debug_printf("test_dispatching: dispatch complete\n");
  error |= AC_TEST(processed_msgs == AC_TRUE);

  // Get a second message and dispatch it
  AcMsg* msg2;
  status = AcMsgPool_get_msg(&mp, &msg2);
  error |= AC_TEST(status == AC_STATUS_OK);
  msg2->arg1 = 1;
  msg2->arg2 = 3;
  AcDispatcher_send_msg(dc1, msg2);

  ac_debug_printf("test_dispatching: rmv_ac\n");
  AcComp* ac2 = AcDispatcher_rmv_comp(pd, dc1);
  error |= AC_TEST(ac2 == &ac1);

  ac_debug_printf("test_dispatching:- error=%d\n", error);
  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  error |= test_dispatcher_get_ret();
  error |= test_dispatcher_add_rmv_acq();
  error |= test_dispatching();
#if AC_PLATFORM == VersatilePB
  ac_printf("py: threading not working on VersatilePB, skip test_threaded_dispatching()\n");
#else
  error |= test_threaded_dispatching();
#endif

  if (!error) {
      // Succeeded
      ac_printf("OK\n");
      return 0;
  }

  return error;
}
