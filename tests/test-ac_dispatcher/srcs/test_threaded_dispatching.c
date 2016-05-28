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
#include <ac_mpscfifo.h>
#include <ac_msg_pool.h>
#include <ac_receptor.h>
#include <ac_thread.h>
#include <ac_test.h>

static ac_bool t1_process_msg(ac* this, AcMsg* pmsg) {
  ac_bool error = AC_FALSE;

  ac_debug_printf("t1_process_msg:+ pmsg->cmd=%d, pmsg->arg=%d\n",
      pmsg->cmd, pmsg->arg);

  error |= AC_TEST(pmsg->cmd == 1);
  error |= AC_TEST(pmsg->arg > 1);

  ac_debug_printf("t1_process_msg:- error=%d\n", error);

  AcMsg_ret(pmsg);

  return AC_TRUE;
}

static ac t1_ac = {
  .process_msg = &t1_process_msg,
};

static ac_mpscfifo t1_acq;
static ac_bool t1_done;
static ac_receptor_t t1_receptor_ready;
static ac_receptor_t t1_receptor_done;
static ac_receptor_t t1_receptor_waiting;

void* t1(void *param) {
  ac_bool error = AC_FALSE;

  // Add an acq
  ac_dispatcher* d;

  t1_receptor_waiting = ac_receptor_create();

  // Get a dispatcher
  d = ac_dispatcher_get(1);
  error |= AC_TEST(d != AC_NULL);

  // Init the queue
  ac_mpscfifo_init(&t1_acq);

  // Add ac1 and its Q dispatcher
  ac_dispatcher_add_acq(d, &t1_ac, &t1_acq);

  // Not done
  __atomic_store_n(&t1_done, AC_FALSE, __ATOMIC_RELEASE);

  // Signal t1 is ready
  ac_debug_printf("t1: ready\n");
  ac_receptor_signal(t1_receptor_ready);

  // Continuously dispatch messages until done
  ac_debug_printf("t1: looping\n");
  while (__atomic_load_n(&t1_done, __ATOMIC_ACQUIRE) == AC_FALSE) {
    if (!ac_dispatch(d)) {
      ac_debug_printf("t1: waiting\n");
      ac_receptor_wait(t1_receptor_waiting);
    }
  }

  if (error) {
    ac_debug_printf("t1: error\n");
  }

  ac_dispatcher_rmv_ac(d, &t1_ac);

  ac_debug_printf("t1: done\n");

  ac_receptor_signal_yield_if_waiting(t1_receptor_done);
  return AC_NULL;
}

void t1_add_msg(AcMsg* msg) {
  ac_mpscfifo_add_msg(&t1_acq, msg);
  ac_receptor_signal(t1_receptor_waiting);
}

void t1_mark_done(void) {
  __atomic_store_n(&t1_done, AC_TRUE, __ATOMIC_RELEASE);
  ac_receptor_signal_yield_if_waiting(t1_receptor_waiting);
}

/**
 * Test threaded dispatching a message
 *
 * return AC_TRUE if an error.
 */
ac_bool test_threaded_dispatching() {
  ac_bool error = AC_FALSE;
  ac_debug_printf("test_threaded_dispatching:+\n");
#if AC_PLATFORM == VersatilePB
  ac_debug_printf("test_threaded_dispatching: VersatilePB threading not working, skipping\n");
#else
  ac_thread_init(1);
  ac_receptor_init(256);
  AcMsgPool* mp = AcMsgPool_create(1);

  error |= AC_TEST(mp != AC_NULL);

  t1_receptor_ready = ac_receptor_create();
  t1_receptor_done = ac_receptor_create();

  ac_thread_rslt_t result = ac_thread_create(AC_THREAD_STACK_MIN, t1, AC_NULL);
  error |= AC_TEST(result.status == 0);

  ac_debug_printf("test_threaded_dispatching: wait until t1 is ready\n");
  ac_receptor_wait(t1_receptor_ready);

  ac_debug_printf("test_threaded_dispatching: send msg\n");
  AcMsg* msg1 = AcMsg_get(mp);
  msg1->cmd = 1;
  msg1->arg = 2;
  t1_add_msg(msg1);

  ac_debug_printf("test_threaded_dispatching: wait 100ms\n");
  ac_thread_wait_ns(100 * 1000000ll);

  ac_debug_printf("test_threaded_dispatching: mark done\n");
  t1_mark_done();

  ac_debug_printf("test_threaded_dispatching: wait until done\n");
  ac_receptor_wait(t1_receptor_done);

  ac_debug_printf("test_threaded_dispatching:- error=%d\n", error);

#endif
  return error;
}
