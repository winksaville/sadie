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

#include <ac_debug_printf.h>
#include <ac_inttypes.h>
#include <ac_msg_pool.h>
#include <ac_receptor.h>
#include <ac_status.h>
#include <ac_thread.h>
#include <ac_test.h>

static ac_bool t1_process_msg(AcComp* this, AcMsg* pmsg) {
  ac_bool error = AC_FALSE;

  ac_debug_printf("t1_process_msg:+ pmsg->arg1=%ld, pmsg->arg2=%ld\n",
      pmsg->arg1, pmsg->arg2);

  error |= AC_TEST(pmsg->arg1 == 1);
  error |= AC_TEST(pmsg->arg2 > 1);

  ac_debug_printf("t1_process_msg:- error=%d\n", error);

  AcMsgPool_ret_msg(pmsg);

  return AC_TRUE;
}

static AcComp t1_ac = {
  .process_msg = &t1_process_msg,
};

static ac_bool t1_done;
static AcDispatchableComp* t1_dc;
static AcReceptor* t1_receptor_ready;
static AcReceptor* t1_receptor_done;
static AcReceptor* t1_receptor_waiting;

void* t1(void *param) {
  ac_bool error = AC_FALSE;

  // Add an acq
  AcDispatcher* d;

  t1_receptor_waiting = AcReceptor_get();

  // Get a dispatcher
  d = AcDispatcher_get(1);
  error |= AC_TEST(d != AC_NULL);

  // Add ac1 and its Q dispatcher
  t1_dc = AcDispatcher_add_comp(d, &t1_ac);

  // Not done
  __atomic_store_n(&t1_done, AC_FALSE, __ATOMIC_RELEASE);

  // Signal t1 is ready
  ac_debug_printf("t1: ready\n");
  AcReceptor_signal(t1_receptor_ready);

  // Continuously dispatch messages until done
  ac_debug_printf("t1: looping\n");
  while (__atomic_load_n(&t1_done, __ATOMIC_ACQUIRE) == AC_FALSE) {
    if (!AcDispatcher_dispatch(d)) {
      ac_debug_printf("t1: waiting\n");
      AcReceptor_wait(t1_receptor_waiting);
    }
  }

  if (error) {
    ac_debug_printf("t1: error\n");
  }

  AcDispatcher_rmv_comp(d, t1_dc);

  AcReceptor_ret(t1_receptor_waiting);

  ac_debug_printf("t1: done\n");

  AcReceptor_signal_yield_if_waiting(t1_receptor_done);
  return AC_NULL;
}

void t1_add_msg(AcMsg* msg) {
  AcDispatcher_send_msg(t1_dc, msg);
  AcReceptor_signal(t1_receptor_waiting);
}

void t1_mark_done(void) {
  __atomic_store_n(&t1_done, AC_TRUE, __ATOMIC_RELEASE);
  AcReceptor_signal_yield_if_waiting(t1_receptor_waiting);
}

/**
 * Test threaded dispatching a message
 *
 * return AC_TRUE if an error.
 */
ac_bool test_threaded_dispatching() {
  ac_bool error = AC_FALSE;
  AcStatus status;

  ac_debug_printf("test_threaded_dispatching:+\n");
#if AC_PLATFORM == VersatilePB
  ac_debug_printf("test_threaded_dispatching: VersatilePB threading not working, skipping\n");
#else
  ac_thread_init(1);
  AcReceptor_init(256);
  AcMsg* msg;
  AcMsgPool mp;

  status = AcMsgPool_init(&mp, 1);
  error |= AC_TEST(status == AC_STATUS_OK);

  t1_receptor_ready = AcReceptor_get();
  t1_receptor_done = AcReceptor_get();

  ac_thread_rslt_t result = ac_thread_create(AC_THREAD_STACK_MIN, t1, AC_NULL);
  error |= AC_TEST(result.status == 0);

  ac_debug_printf("test_threaded_dispatching: wait until t1 is ready\n");
  AcReceptor_wait(t1_receptor_ready);

  ac_debug_printf("test_threaded_dispatching: send msg\n");
  status = AcMsgPool_get_msg(&mp, &msg);
  error |= AC_TEST(status == AC_STATUS_OK);
  error |= AC_TEST(msg != AC_NULL);
  msg->arg1 = 1;
  msg->arg2 = 2;
  t1_add_msg(msg);

  ac_debug_printf("test_threaded_dispatching: wait 100ms\n");
  ac_thread_wait_ns(100 * 1000000ll);

  ac_debug_printf("test_threaded_dispatching: mark done\n");
  t1_mark_done();

  ac_debug_printf("test_threaded_dispatching: wait until done\n");
  AcReceptor_wait(t1_receptor_done);

  ac_debug_printf("test_threaded_dispatching:- error=%d\n", error);

  AcReceptor_ret(t1_receptor_ready);
  AcReceptor_ret(t1_receptor_done);

#endif
  return error;
}
