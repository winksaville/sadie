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

#include <ac_inet_link.h>
#include <ac_inet_link_internal.h>

#include <ac_comp_mgr.h>
#include <ac_inttypes.h>
#include <ac_msg.h>
#include <ac_msg_pool.h>
#include <ac_printf.h>
#include <ac_debug_printf.h>
#include <ac_receptor.h>
#include <ac_thread.h>
#include <ac_test.h>

// Define states
#define DEF_STATE(name) AcBool name(AcComp* comp, AcMsg* msg)
DEF_STATE(initial_state);
DEF_STATE(work_state);
DEF_STATE(done_state);

#define SEND_ARP_REQ    AC_OP(0, AC_OPTYPE_CMD, 1)
#define DONE            AC_OP(0, AC_OPTYPE_CMD, 100)

// The test component structure the first element must be an AcComp
typedef struct {
  AcComp comp;
  AcReceptor* waiting;
} TestComp;

// The test component
static TestComp test_comp = {
  .comp.name=(ac_u8*)"test_comp_ipv4_ll",
  .comp.process_msg = initial_state
};

#define TRANS_TO(cm, x) ((AcComp*)(cm))->process_msg = (x)

// LDR and ldr for print statements, allows you
// to customize the leading part of the debug output
#define LDR "%s.%s"
#define ldr this->comp.name, STATE_NAME

/**
 * Initial state
 */
#define STATE_NAME "initial_state"
ac_bool initial_state(AcComp* comp, AcMsg* msg) {
  TestComp* this = (TestComp*)comp;
  ac_debug_printf(LDR ":+msg->op=%lx\n", ldr, msg->op);

  switch (msg->op) {
    case (AC_INIT_CMD): {
      ac_debug_printf(LDR ": AC_INIT_CMD\n", ldr);
      TRANS_TO(this, work_state);
      break;
    }
    default: {
      ac_printf(LDR ": unregonized AC_OP=%lx\n", ldr, msg->op);
      break;
    }
  }

  AcMsgPool_ret_msg(msg);

  ac_debug_printf(LDR ":-msg->op=%lx\n", ldr, msg->op);
  return AC_TRUE;
}
#undef STATE_NAME


/**
 * Working state
 */
#define STATE_NAME "work_state"
ac_bool work_state(AcComp* comp, AcMsg* msg) {
  TestComp* this = (TestComp*)comp;
  ac_debug_printf(LDR ":+msg->op=%lx\n", ldr, msg->op);

  switch (msg->op) {
    case SEND_ARP_REQ: {
      ac_printf(LDR ": SEND_ARP_REQ\n", ldr);
      AcReceptor_signal(this->waiting);
      break;
    }
    case DONE: {
      ac_printf(LDR ": DONE\n", ldr);
      TRANS_TO(this, done_state);
      AcReceptor_signal(this->waiting);
      break;
    }
    default: {
      ac_printf(LDR ": unregonized AC_OP=%lx\n", ldr, msg->op);
      TRANS_TO(this, done_state);
      break;
    }
  }

  AcMsgPool_ret_msg(msg);

  ac_debug_printf(LDR ":-msg->op=%lx\n", ldr, msg->op);
  return AC_TRUE;
}
#undef STATE_NAME


/**
 * Done state
 */
#define STATE_NAME "done_state"
ac_bool done_state(AcComp* comp, AcMsg* msg) {
  TestComp* this = (TestComp*)comp;
  ac_debug_printf(LDR ":+msg->op=%lx\n", ldr, msg->op);

  switch (msg->op) {
    case (AC_DEINIT_CMD): {
      ac_debug_printf(LDR ": AC_DEINIT_CMD\n", ldr);
      break;
    }
    default: {
      ac_printf(LDR ": unregonized AC_OP=%lx\n", ldr, msg->op);
      break;
    }
  }

  AcMsgPool_ret_msg(msg);

  ac_debug_printf(LDR ":-msg->op=%lx\n", ldr, msg->op);
  return AC_TRUE;
}
#undef STATE_NAME
#undef LDR
#undef ldr

/**
 * Test component deinit
 */
#define LDR "%s"
#define ldr "TestComp_deinit"
void TestComp_deinit(AcCompMgr* cm) {
  ac_debug_printf(LDR ":+cm=%p comp=%s\n", ldr, cm, test_comp.comp.name);

  ac_assert(AcCompMgr_rmv_comp((AcComp*)&test_comp) == AC_STATUS_OK);

  ac_debug_printf(LDR ":-cm=%p comp=%s\n", ldr, cm, test_comp.comp.name);
}
#undef LDR
#undef ldr

/**
 * Test component init
 */
#define LDR "%s"
#define ldr "TestComp_init"
void TestComp_init(AcCompMgr* cm) {
  ac_debug_printf(LDR ":+cm=%p comp=%s\n", ldr, cm, test_comp.comp.name);

  ac_assert(AcCompMgr_add_comp(cm, &test_comp.comp) == AC_STATUS_OK);
  test_comp.waiting = AcReceptor_get();

  ac_debug_printf(LDR ":-cm=%p comp=%s\n", ldr, cm, test_comp.comp.name);
}
#undef LDR
#undef ldr

/**
 * Test inet link implemenation
 */
#define LDR "%s"
#define ldr "test_ient_link_impl"
AcUint test_inet_link_impl(AcCompMgr* cm) {
  AcUint error = AC_FALSE;
  AcMsgPool mp;
  AcMsg* msg;

  ac_debug_printf(LDR ":+\n", ldr);

  // Initialize the test component
  TestComp_init(cm);

  // Create a message pool
  AcMsgPool_init(&mp, 8, sizeof(AcInetSendPacketExtra));

  // Send a ARP Request
  msg = AcMsgPool_get_msg(&mp);
  msg->op = SEND_ARP_REQ;
  AcCompMgr_send_msg(&test_comp.comp, msg);
  AcReceptor_wait(test_comp.waiting);

  // Send Done
  msg = AcMsgPool_get_msg(&mp);
  msg->op = DONE;
  AcCompMgr_send_msg(&test_comp.comp, msg);
  AcReceptor_wait(test_comp.waiting);

  // Deinitialize the test component
  TestComp_deinit(cm);

  ac_debug_printf(LDR ":-\n", ldr);
  return error;
}
#undef LDR
#undef ldr

int main(void) {
  AcUint error = AC_FALSE;

  ac_thread_init(3);
  AcReceptor_init(256);

  AcCompMgr cm;
  AcStatus status = AcCompMgr_init(&cm, 3, 10, 0);
  error |= AC_TEST(status == AC_STATUS_OK);

  AcInetLink_init(&cm);

  if (!error) {
    error |= test_inet_link_impl(&cm);
  }

  AcInetLink_deinit(&cm);

  AcCompMgr_deinit(&cm);

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
