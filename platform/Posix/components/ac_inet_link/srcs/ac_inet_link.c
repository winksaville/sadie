/*
 * copyright 2016 wink saville
 *
 * licensed under the apache license, version 2.0 (the "license");
 * you may not use this file except in compliance with the license.
 * you may obtain a copy of the license at
 *
 *     http://www.apache.org/licenses/license-2.0
 *
 * unless required by applicable law or agreed to in writing, software
 * distributed under the license is distributed on an "as is" basis,
 * without warranties or conditions of any kind, either express or implied.
 * see the license for the specific language governing permissions and
 * limitations under the license.
 */

//#define NDEBUG

#include <ac_inet_link.h>

#include <ac_assert.h>
#include <ac_comp_mgr.h>
#include <ac_debug_printf.h>
#include <ac_msg_pool.h>
#include <ac_status.h>

typedef struct {
  AcComp comp;
  ac_u32 a_u32;
} AcCompIpv4LinkLayer;

static void send_error_rsp(AcComp* comp, AcMsg* msg, AcStatus status) {
}

ac_bool comp_ipv4_ll_process_msg(AcComp* comp, AcMsg* msg) {
  AcCompIpv4LinkLayer* this = (AcCompIpv4LinkLayer*)comp;
  ac_debug_printf("%s: msg->arg1=%ls msg->arg2=%lx\n",
      this->comp.name, msg->arg1, msg->arg2);

  AcOperation op = { .operation = msg->arg1 };
  if (op.operation == AC_INIT_CMD.operation) {
    ac_debug_printf("%s: AC_INIT_CMD\n", this->comp.name);
  } else if (op.operation == AC_DEINIT_CMD.operation) {
    ac_debug_printf("%s: AC_DEINIT_CMD\n", this->comp.name);
  } else if (op.operation == AC_INET_SEND_PACKET_REQ.operation) {
    ac_debug_printf("%s: ok\n", this->comp.name);
  } else {
    send_error_rsp(comp, msg, AC_STATUS_UNRECOGNIZED_PROTOCOL);
  }

  AcMsgPool_ret_msg(msg);

  return AC_TRUE;
}

static AcCompIpv4LinkLayer comp_ipv4_ll = {
  .comp.name=(ac_u8*)"comp_ipv4_ll",
  .comp.process_msg = comp_ipv4_ll_process_msg
};

static AcCompInfo* ci;

/**
 * Initialize this module
 */
void AcInetLink_init(AcCompMgr* cm) {
  ac_debug_printf("AcInetLink_init:xxx # cm=%p\n", cm);

  ci = AcCompMgr_add_comp(cm, &comp_ipv4_ll.comp);
  ac_assert(ci != AC_NULL);
}
