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
#include <ac_mpscfifo.h>
#include <ac_debug_printf.h>
#include <ac_test.h>

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

static ac ac1 = {
  .process_msg = &ac1_process_msg,
  .get_data = &ac1_get_data,
};



/**
 * Test threaded dispatching a message
 *
 * return AC_TRUE if an error.
 */
ac_bool test_threaded_dispatching() {
  ac_bool error = AC_FALSE;
  ac_debug_printf("test_threaded_dispatching:+\n");

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


  ac_debug_printf("test_threaded_dispatching: dispatch now\n");
  ac_bool processed_msgs = ac_dispatch(pd);
  ac_debug_printf("test_threaded_dispatching: dispatch complete\n");
  error |= AC_TEST(processed_msgs == AC_TRUE);

  ac_msg msg2 = {
    .cmd = 1,
    .arg = 3
  };
  ac_add_msg(pac1q, &msg2);

  ac_debug_printf("test_threaded_dispatching: rmv_ac\n");
  ac* pac2 = ac_dispatcher_rmv_ac(pd, pac1);
  error |= AC_TEST(pac2 == pac1);

  ac_debug_printf("test_threaded_dispatching:- error=%d\n", error);
  return error;
}


