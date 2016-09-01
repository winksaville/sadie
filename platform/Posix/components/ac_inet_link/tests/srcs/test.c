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

// LDR and ldr for print statements, allows you
// to customize the leading part of the debug output
#define LDR "%s"
#define ldr this->comp.name

// Forware declarations
static ac_bool test_comp_process_msg(AcComp* comp, AcMsg* msg);

// The test component structure the first element must be an AcComp
typedef struct {
  AcComp comp;
  ac_u32 a_u32;
} TestComp;

// The test component
static TestComp test_comp = {
  .comp.name=(ac_u8*)"test_comp_ipv4_ll",
  .comp.process_msg = test_comp_process_msg
};

// The test component message processor
#define LDR "%s"
#define ldr this->comp.name
ac_bool test_comp_process_msg(AcComp* comp, AcMsg* msg) {
  TestComp* this = (TestComp*)comp;
  ac_debug_printf(LDR ":+msg->op=%lx\n", ldr, msg->op);

  switch (msg->op) {
    case (AC_INIT_CMD): {
      ac_debug_printf(LDR ": AC_INIT_CMD\n", ldr);
      break;
    }
    case (AC_DEINIT_CMD): {
      ac_debug_printf(LDR ": AC_DEINIT_CMD\n", ldr);
      break;
    }
    case (AC_INET_SEND_PACKET_REQ): {
      ac_debug_printf(LDR ": AC_INET_SEND_PACKET_REQ\n", ldr);
      break;
    }
    default: {
      ac_printf(LDR ": unregonized AC_OP=%u\n", ldr, msg->op);
      break;
    }
  }

  AcMsgPool_ret_msg(msg);

  ac_debug_printf(LDR ":-msg->op=%lx\n", ldr, msg->op);
  return AC_TRUE;
}
#undef LDR
#undef ldr

/**
 * Test component deinit
 */
#define LDR "TestComp_deinit"
#define ldr
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
#define LDR "TestComp_init"
#define ldr
void TestComp_init(AcCompMgr* cm) {
  ac_debug_printf(LDR ":+cm=%p comp=%s\n", ldr, cm, test_comp.comp.name);

  ac_assert(AcCompMgr_add_comp(cm, &test_comp.comp) == AC_STATUS_OK);

  ac_debug_printf(LDR ":-cm=%p comp=%s\n", ldr, cm, test_comp.comp.name);
}
#undef LDR
#undef ldr

/**
 * Test inet link implemenation
 */
#define LDR "test_ient_link_impl"
#define ldr
AcUint test_inet_link_impl(void) {
  AcUint error = AC_FALSE;
  ac_debug_printf(LDR ":+\n", ldr);

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
    error |= test_inet_link_impl();
  }

  AcInetLink_deinit(&cm);

  AcCompMgr_deinit(&cm);

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
