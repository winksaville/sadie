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

#include <ac_debug_printf.h>
#include <ac_inttypes.h>
#include <ac_msg.h>
#include <ac_msg_pool.h>
#include <ac_receptor.h>
#include <ac_status.h>
#include <ac_thread.h>
#include <ac_test.h>

static ac_bool t1_process_msg(AcComp* this, AcMsg* pmsg);

typedef struct {
  AcComp comp;
  ac_u32 ac_init_cmd_count;
  ac_u32 ac_deinit_cmd_count;
  ac_bool error;
} AcCompT1;

static AcCompT1 t1_ac = {
  .comp = {
    .name = (ac_u8*)"t1_ac",
    .process_msg = &t1_process_msg,
  },
};

static ac_bool t1_process_msg(AcComp* comp, AcMsg* msg) {
  AcCompT1* this = (AcCompT1*)comp;

  if (msg->hdr.op.operation == AC_INIT_CMD) {
    ac_debug_printf("t1_process_msg:+msg->hdr.op.operation=AC_INIT_CMD\n");
    this->ac_init_cmd_count += 1;
    this->error |= AC_TEST(this->ac_init_cmd_count == 1);
    this->error |= AC_TEST(this->ac_deinit_cmd_count == 0);
  } else if (msg->hdr.op.operation == AC_DEINIT_CMD) {
    ac_debug_printf("t1_process_msg:+msg->hdr.op.operation=AC_DEINIT_CMD\n");
    this->ac_deinit_cmd_count += 1;
    this->error |= AC_TEST(this->ac_init_cmd_count == 1);
    this->error |= AC_TEST(this->ac_deinit_cmd_count == 1);
  } else {
    ac_debug_printf("t1_process_msg:+msg->hdr.op.operation=%lx\n",
        msg->hdr.op.operation);
    // Handle other messages
  }

  ac_debug_printf("t1_process_msg:- msg->hdr.op.operation=%lx error=%d\n",
      msg->hdr.op.operation, error);

  AcMsgPool_ret_msg(msg);

  return AC_TRUE;
}

static ac_bool t1_done;
static AcStatus t1_error;
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
  t1_ac.ac_init_cmd_count = 0;
  t1_ac.ac_deinit_cmd_count = 0;
  t1_ac.error = AC_FALSE;
  t1_dc = AcDispatcher_add_comp(d, &t1_ac.comp);

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

  error |= AC_TEST(t1_ac.ac_init_cmd_count == 1);
  error |= AC_TEST(t1_ac.ac_deinit_cmd_count == 0);

  AcDispatcher_rmv_comp(d, t1_dc);
  error |= AC_TEST(t1_ac.ac_init_cmd_count == 1);
  error |= AC_TEST(t1_ac.ac_deinit_cmd_count == 1);

  AcReceptor_ret(t1_receptor_waiting);

  error |= AC_TEST(t1_ac.error == AC_FALSE);

  t1_error = error;
  ac_debug_printf("t1: done t1_error=%d\n", t1_error);

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

  ac_debug_printf("test_threaded_dispatching:+\n");
#if AC_PLATFORM == VersatilePB
  ac_debug_printf("test_threaded_dispatching: VersatilePB threading not working, skipping\n");
#else
  ac_thread_init(1);
  AcReceptor_init(256);
  AcMsg* msg;
  AcMsgPool mp;
  AcStatus status;

  status = AcMsgPool_init(&mp, 1, 0);
  error |= AC_TEST(status == AC_STATUS_OK);

  t1_receptor_ready = AcReceptor_get();
  t1_receptor_done = AcReceptor_get();

  ac_thread_rslt_t result = ac_thread_create(AC_THREAD_STACK_MIN, t1, AC_NULL);
  error |= AC_TEST(result.status == 0);

  ac_debug_printf("test_threaded_dispatching: wait until t1 is ready\n");
  AcReceptor_wait(t1_receptor_ready);

  ac_debug_printf("test_threaded_dispatching: send msg\n");
  msg = AcMsgPool_get_msg(&mp);
  error |= AC_TEST(msg != AC_NULL);
  msg->hdr.op.operation = 1;
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

  error |= t1_error;
#endif
  return error;
}
