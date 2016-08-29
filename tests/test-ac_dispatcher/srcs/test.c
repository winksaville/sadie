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

#define NDEBUG

#include "test-ac_dispatcher/incs/tests.h"

#include <ac_dispatcher.h>

#include <ac_inttypes.h>
#include <ac_debug_printf.h>
#include <ac_msg.h>
#include <ac_msg_pool.h>
#include <ac_test.h>

typedef struct {
  AcComp comp;
  ac_u32 msg_count;
  ac_u32 ac_init_cmd_count;
  ac_u32 ac_deinit_cmd_count;
  ac_bool error;
} Ac1Comp;

static ac_bool ac1_process_msg(AcComp* ac, AcMsg* msg) {
  Ac1Comp* this = (Ac1Comp*)ac;

  ac_debug_printf("ac1_process_msg:+ msg->arg1=%lx, msg->arg2=%lx\n",
      msg->arg1, msg->arg2);

  ac_debug_printf("ac1_process_msg:+\n");

  if (msg->hdr.op.operation == AC_INIT_CMD) {
    ac_debug_printf("ac1_process_msg: AC_INIT_CMD\n");
    this->ac_init_cmd_count += 1;
    this->error |= AC_TEST(this->ac_init_cmd_count == 1);
    this->error |= AC_TEST(this->ac_deinit_cmd_count == 0);
  } else if (msg->hdr.op.operation == AC_DEINIT_CMD) {
    ac_debug_printf("ac1_process_msg: AC_DEINIT_CMD\n");
    this->ac_deinit_cmd_count += 1;
    this->error |= AC_TEST(this->ac_init_cmd_count == 1);
    this->error |= AC_TEST(this->ac_deinit_cmd_count == 1);
  } else {
    ac_debug_printf("ac1_process_msg: OTHER msg->arg1=%lx, msg->arg2=%lx\n",
        msg->arg1, msg->arg2);
    this->msg_count += 1;
    this->error |= AC_TEST(msg->hdr.op.operation == 1);
  }

  ac_debug_printf("ac1_process_msg: ret msg=%p\n", msg);
  AcMsgPool_ret_msg(msg);

  ac_debug_printf("ac1_process_msg:-error=%d\n", this->error);

  return AC_TRUE;
}


/**
 * Test dispatcher get and return
 *
 * return AC_TRUE if an error.
 */
/*static*/ ac_bool test_dispatcher_get_ret() {
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
/*static*/ int test_dispatcher_add_rmv_acq() {
  ac_bool error = AC_FALSE;

  // Get a dispatcher
  AcDispatcher* pd = AcDispatcher_get(1);
  error |= AC_TEST(pd != AC_NULL);

  // Add a acq
  Ac1Comp ac1 = {
    .comp = { .name=(ac_u8*)"ac1", .process_msg = &ac1_process_msg },
  };
  AcDispatchableComp* dc1;

  ac1.msg_count = 0;
  ac1.ac_init_cmd_count = 0;
  ac1.ac_deinit_cmd_count = 0;
  ac1.error = 0;
  dc1 = AcDispatcher_add_comp(pd, &ac1.comp);
  error |= AC_TEST(dc1 != AC_NULL);

  // Test that adding a second acq fails because we
  // only are allowing 1
  AcDispatchableComp* dc2 = AcDispatcher_add_comp(pd, &ac1.comp);
  error |= AC_TEST(dc2 == AC_NULL);

  // Test we can remove pac1 and then add it back
  AcComp* pac2 = AcDispatcher_rmv_comp(pd, dc1);
  error |= AC_TEST(pac2 == &ac1.comp);
  ac1.msg_count = 0;
  ac1.ac_init_cmd_count = 0;
  ac1.ac_deinit_cmd_count = 0;
  ac1.error = 0;
  dc1 = AcDispatcher_add_comp(pd, &ac1.comp);
  error |= AC_TEST(dc1 != AC_NULL);

  // And finally remove so we leave with the pd empty
  pac2 = AcDispatcher_rmv_comp(pd, dc1);
  error |= AC_TEST(pac2 == &ac1.comp);

  // Return the dispatcher
  AcDispatcher_ret(pd);

  return error;
}

/**
 * Test dispatching a message
 *
 * return AC_TRUE if an error.
 */
/*static*/ ac_bool test_dispatching() {
  ac_bool error = AC_FALSE;
  AcStatus status;
  ac_debug_printf("test_dispatching:+\n");

  // Get a dispatcher
  AcDispatcher* pd = AcDispatcher_get(1);
  error |= AC_TEST(pd != AC_NULL);

  Ac1Comp ac1 = {
    .comp = { .name=(ac_u8*)"ac1", .process_msg = &ac1_process_msg },
    .msg_count = 0,
    .ac_init_cmd_count = 0,
    .ac_deinit_cmd_count = 0,
    .error = 0
  };

  // Add an acq
  AcDispatchableComp* dc1;

  dc1 = AcDispatcher_add_comp(pd, &ac1.comp);
  error |= AC_TEST(dc1 != AC_NULL);

  // Initialize a msg pool
  AcMsgPool mp;
  status = AcMsgPool_init(&mp, 2, 0);
  error |= AC_TEST(status == AC_STATUS_OK);

  // Get a msg and dispatch it
  AcMsg* msg1;
  msg1 = AcMsgPool_get_msg(&mp);
  error |= AC_TEST(msg1 != AC_NULL);
  msg1->hdr.op.operation = 1;
  AcDispatcher_send_msg(dc1, msg1);


  ac_debug_printf("test_dispatching: dispatch now\n");
  ac_bool processed_msgs = AcDispatcher_dispatch(pd);
  ac_debug_printf("test_dispatching: dispatch complete\n");
  error |= AC_TEST(processed_msgs == AC_TRUE);

  // Get a second message and dispatch it
  AcMsg* msg2;
  msg2 = AcMsgPool_get_msg(&mp);
  error |= AC_TEST(msg2 != AC_NULL);
  msg2->hdr.op.operation = 1;
  AcDispatcher_send_msg(dc1, msg2);

  ac_debug_printf("test_dispatching: rmv_ac\n");
  AcComp* ac2 = AcDispatcher_rmv_comp(pd, dc1);
  error |= AC_TEST(ac2 == &ac1.comp);
  error |= AC_TEST(ac1.error == 0);

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
