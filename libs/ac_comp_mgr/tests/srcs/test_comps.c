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
  AcComp comp;
  AcCompInfo* ci;
  ac_bool error;
  ac_receptor_t done;
  ac_u8 name_buf[10];
} T1Comp;

static ac_bool msg_proc(AcComp* ac, ac_msg* msg) {
  T1Comp* this = (T1Comp*)ac;

  ac_debug_printf("msg_proc:+%s\n", this->comp.name);

  this->error |= AC_TEST(msg->cmd == 1);
  this->error |= AC_TEST(msg->arg == 2);
  this->error |= AC_TEST(msg->arg_u64 == 3);

  AcMsg_ret(msg);

  ac_receptor_signal(this->done);

  ac_debug_printf("msg_proc:-%s\n", this->comp.name);
  return AC_TRUE;
}

/**
 * Test we can create, send a message and remove components.
 *
 * @param: cm is AcCompMgr to use
 * @param: mp is AcMsgPool to use
 * @param: comp_count is number of components to create
 *
 * @return: AC_TRUE if an error
 */
ac_bool test_comps(AcCompMgr* cm, AcMsgPool* mp, ac_u32 comp_count) {
  ac_debug_printf("test_comps:+cm=%p mp=%p comp_count=%d\n", cm, mp, comp_count);
  ac_bool error = AC_FALSE;

  T1Comp* comps= ac_malloc(comp_count * sizeof(T1Comp));
  T1Comp* c;
  for (ac_u32 i = 0; !error && (i < comp_count); i++) {
    c = &comps[i];
    ac_sprintf(c->name_buf, sizeof(c->name_buf), "t%d", i);
    c->comp.name = c->name_buf;
    c->comp.process_msg = msg_proc;
    c->done = ac_receptor_create(),

    ac_debug_printf("test_comps: adding %s\n", c->comp.name);
    c->ci = AcCompMgr_add_comp(cm, &c->comp);
    ac_debug_printf("test_comps: added %s ci=%p\n", c->comp.name, c->ci);
    error |=AC_TEST(c->ci != AC_NULL);
  }

  if (!error) {
    ac_debug_printf("test_comps: send msgs\n");
    for (ac_u32 i = 0; i < comp_count; i++) {
      c = &comps[i];
      AcMsg* msg = AcMsg_get(mp);
      msg->cmd = 1;
      msg->arg = 2;
      msg->arg_u64 = 3;
      ac_debug_printf("test_comps: send msg %s ci=%p\n", c->comp.name, c->ci);
      AcCompMgr_send_msg(cm, c->ci, msg);
    }

    ac_debug_printf("test_comps: wait until all messages are received\n");
    for (ac_u32 i = 0; !error && (i < comp_count); i++) {
      c = &comps[i];
      ac_receptor_wait(c->done);
      ac_debug_printf("test_comps: %s error=%d\n", c->comp.name, c->error);
      error |= c->error;
    }

    ac_debug_printf("test_comps: remove comps\n");
    for (ac_u32 i = 0; !error && (i < comp_count); i++) {
      c = &comps[i];
      ac_debug_printf("test_comps: remove %s ci=%p\n", c->comp.name, c->ci);
      AcComp* pt1 = AcCompMgr_rmv_comp(cm, c->ci);
      error |= AC_TEST(pt1 == &c->comp);
      error |= AC_TEST((T1Comp*)pt1 == c);
    }
  }

  ac_free(comps);

  ac_debug_printf("test_comps:-cm=%p mp=%p comp_count=%d error=%d\n", cm, mp, comp_count, error);
  return error;
}
