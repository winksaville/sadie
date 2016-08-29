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
#include <ac_msg.h>
#include <ac_msg_pool.h>
#include <ac_status.h>

typedef struct {
  AcComp comp;
  ac_u32 a_u32;
} AcCompIpv4LinkLayer;

static void send_error_rsp(AcComp* comp, AcMsg* msg, AcStatus status) {
}

static ac_bool comp_ipv4_ll_process_msg(AcComp* comp, AcMsg* msg) {
  AcCompIpv4LinkLayer* this = (AcCompIpv4LinkLayer*)comp;
  ac_debug_printf("%s:+msg->hdr.op=%lx\n", this->comp.name, msg->hdr.op.operation);

  switch (msg->hdr.op.operation) {
    case (AC_INIT_CMDx): {
      ac_debug_printf("%s: AC_INIT_CMD\n", this->comp.name);
      break;
    }
    case (AC_DEINIT_CMDx): {
      ac_debug_printf("%s: AC_DEINIT_CMD\n", this->comp.name);
      break;
    }
    case (AC_INET_SEND_PACKET_REQ): {
      ac_debug_printf("%s: AC_INET_SEND_PACKET_REQ\n", this->comp.name);
      break;
    }
    default: {
      ac_debug_printf("%s: AC_STATUS_UNRECOGNIZED_PROTOCOL send error rsp\n", this->comp.name);
      send_error_rsp(comp, msg, AC_STATUS_UNRECOGNIZED_PROTOCOL);
      break;
    }
  }

  AcMsgPool_ret_msg(msg);

  ac_debug_printf("%s:-msg->op=%lx\n", this->comp.name, msg->hdr.op.operation);
  return AC_TRUE;
}

static AcCompIpv4LinkLayer comp_ipv4_ll = {
  .comp.name=(ac_u8*)"comp_ipv4_ll",
  .comp.process_msg = comp_ipv4_ll_process_msg
};

static AcCompInfo* ci;

/**
 * see ac_inet_link_internal.h
 */
void AcInetLink_deinit(AcCompMgr* cm) {
  ac_debug_printf("AcInetLink_deinit:+cm=%p\n", cm);

  AcComp* c = AcCompMgr_rmv_comp(ci);
  ac_assert(c == &comp_ipv4_ll.comp);

  ac_debug_printf("AcInetLink_deinit:-cm=%p\n", cm);
}

/**
 * see ac_inet_link.h
 */
void AcInetLink_init(AcCompMgr* cm) {
  ac_debug_printf("AcInetLink_init:+cm=%p\n", cm);

  ci = AcCompMgr_add_comp(cm, &comp_ipv4_ll.comp);
  ac_assert(ci != AC_NULL);

  ac_debug_printf("AcInetLink_init:-cm=%p\n", cm);
}
