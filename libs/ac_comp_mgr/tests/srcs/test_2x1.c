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

#include <ac_memmgr.h>
#include <ac_msg_pool.h>
#include <ac_debug_printf.h>
#include <ac_printf.h>
#include <ac_receptor.h>
#include <ac_test.h>
#include <ac_time.h>
#include <ac_thread.h>

typedef struct T1Comp {
  ac_u8 name_buf[10];
  AcComp comp;
  AcCompInfo* ci;
  ac_bool error;
  ac_u64* p_msg_counter;
  ac_receptor_t done;
} T1Comp;

static ac_bool t1mp(AcComp* ac, ac_msg* msg) {
  T1Comp* this = (T1Comp*)ac;

  ac_debug_printf("t1mp:+ this=%p\n", this);

  __atomic_add_fetch(this->p_msg_counter, 1, __ATOMIC_RELEASE);

  this->error |= AC_TEST(msg->cmd == 1);
  this->error |= AC_TEST(msg->arg == 2);
  this->error |= AC_TEST(msg->arg_u64 == 3);

  AcMsg_ret(msg);

  ac_receptor_signal(this->done);

  ac_debug_printf("t1mp:- this=%p\n", this);
  return AC_TRUE;
}

/**
 * Test we can create a T1Comp and send a message then remove it.
 *
 * @param: mp is AcMsgPool to use contains at least one message.
 *
 * @return: AC_TRUE if an error
 */
static ac_bool test_comps(AcCompMgr* cm, AcMsgPool* mp, ac_u32 comp_count) {
  ac_debug_printf("test_comps:+cm=%p mp=%p comp_count=%d\n", cm, mp, comp_count);
  ac_bool error = AC_FALSE;

  T1Comp* comps= ac_malloc(comp_count * sizeof(T1Comp));
  T1Comp* c;
  for (ac_u32 i; i < comp_count; i++) {
    c = &comps[i];
    ac_sprintf(c->name_buf, sizeof(c->name_buf), "t%d", i);
    c->comp.name = c->name_buf;
    c->comp.process_msg = t1mp,
    c->done = ac_receptor_create(),

    ac_debug_printf("test_comps: add %s\n", c->comp.name);
    c->ci = AcCompMgr_add_comp(cm, &c->comp);
    error |=AC_TEST(c->ci != AC_NULL);
  }

  if (!error) {
    ac_debug_printf("test_comps: send msgs\n");
    for (ac_u32 i; i < comp_count; i++) {
      c = &comps[i];
      AcMsg* msg = AcMsg_get(mp);
      msg->cmd = 1;
      msg->arg = 2;
      msg->arg_u64 = 3;
      ac_debug_printf("test_comps: send msg %s\n", c->comp.name);
      AcCompMgr_send_msg(cm, c->ci, msg);
    }

    ac_debug_printf("test_comps: wait until all messages are received\n");
    for (ac_u32 i; i < comp_count; i++) {
      c = &comps[i];
      ac_receptor_wait(c->done);
      ac_debug_printf("test_comps: %s error=%d\n", c->comp.name);
      error |= c->error;
    }

    ac_debug_printf("test_comps: remove comps\n");
    for (ac_u32 i; i < comp_count; i++) {
      c = &comps[i];
      ac_debug_printf("test_comps: remove %s\n", c->comp.name);
      AcComp* pt1 = AcCompMgr_rmv_comp(cm, c->ci);
      error |= AC_TEST(pt1 == &c->comp);
      error |= AC_TEST((T1Comp*)pt1 == c);
    }
  }

  ac_free(comps);

  ac_debug_printf("test_comps:-cm=%p mp=%p comp_count=%d error=d\n", cm, mp, comp_count, error);
  return error;
}

/**
 * Test we can init the component manager, create/remove a component tice.
 *
 * @return: AC_TRUE if an error
 */
ac_bool test_thread_comps(ac_u32 threads, ac_u32 comps_per_thread) {
  ac_bool error = AC_FALSE;
  ac_debug_printf("test_2x1:+\n");

  ac_debug_printf("test_2x1: create msg pool\n");
  AcMsgPool* mp = AcMsgPool_create(threads * comps_per_thread);

  ac_debug_printf("test_2x1: init comp mgr\n");
  ac_u32 stack_size = 0;
  AcCompMgr* cm = AcCompMgr_init(threads, comps_per_thread, stack_size);

  ac_debug_printf("test_2x1: first test_comps invocation\n");
  error |= AC_TEST(test_comps(cm, mp, threads * comps_per_thread) == AC_FALSE);

  ac_debug_printf("test_2x1: second test_comps invocation\n");
  error |= AC_TEST(test_comps(cm, mp, threads * comps_per_thread) == AC_FALSE);

  ac_debug_printf("test_2x1: deinit comp mgr\n");
  AcCompMgr_deinit(cm);

  ac_debug_printf("test_2x1:-\n");
  return error;
}
