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

#include <ac_comp_mgr.h>

#include <ac_msg_pool.h>
#include <ac_debug_printf.h>
#include <ac_printf.h>
#include <ac_receptor.h>
#include <ac_test.h>
#include <ac_time.h>
#include <ac_thread.h>

typedef struct T1Comp {
  AcComp comp;
  ac_bool error;
  ac_receptor_t done;
} T1Comp;

ac_bool t1mp(AcComp* ac, ac_msg* msg) {
  T1Comp* this = (T1Comp*)ac;

  ac_debug_printf("t1mp:+ this=%p\n", this);

  this->error |= AC_TEST(msg->cmd == 1);
  this->error |= AC_TEST(msg->arg == 2);
  this->error |= AC_TEST(msg->arg_u64 == 3);

  AcMsg_ret(msg);

  ac_receptor_signal(this->done);

  ac_debug_printf("t1mp:- this=%p\n", this);
  return AC_TRUE;
}

ac_bool test_AcCompMgr(void) {
  ac_bool error = AC_FALSE;
  ac_debug_printf("test_ac_comp_mgr:+\n");

  const ac_u32 max_component_threads = 1;
  const ac_u32 max_components = max_component_threads * 1;

  ac_debug_printf("test_ac_comp_mgr: init comp mgr\n");
  ac_u32 stack_size = 0;
  AcCompMgr_init(max_component_threads, max_components, stack_size);

  T1Comp t1 = {
    .comp.name = "t1",
    .comp.process_msg = t1mp,
    .done = ac_receptor_create(),
  };

  ac_debug_printf("test_ac_comp_mgr: add comp\n");
  AcCompInfo* ci = AcCompMgr_add_comp(&t1.comp);

  ac_debug_printf("test_ac_comp_mgr: create msg pool\n");
  AcMsgPool* mp = AcMsgPool_create(1);
  AcMsg* msg = AcMsg_get(mp);
  msg->cmd = 1;
  msg->arg = 2;
  msg->arg_u64 = 3;

  ac_debug_printf("test_ac_comp_mgr: send msg\n");
  AcCompMgr_send_msg(ci, msg);

  ac_debug_printf("test_ac_comp_mgr: wait until the message is received\n");
  ac_receptor_wait(t1.done);

  ac_debug_printf("test_ac_comp_mgr: get the result\n");
  error = t1.error;

  ac_debug_printf("test_ac_comp_mgr: remove comp\n");
  AcCompMgr_rmv_comp(ci);

  ac_debug_printf("test_ac_comp_mgr: deinit comp mgr\n");
  AcCompMgr_deinit();

  ac_debug_printf("test_ac_comp_mgr:-\n");
  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  ac_thread_init(10);
  ac_receptor_init(256);
  AcTime_init();

#if AC_PLATFORM == VersatilePB
  ac_printf("AC_PLATFORM == VersatilePB, skipping test ac_comp_mgr\n");
#else
  error|= test_AcCompMgr();
#endif

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
