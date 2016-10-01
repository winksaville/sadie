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

#include <ac_arp.h>

#include <ac_inet_link.h>
#include <ac_inet_hton.h>
#include <ac_inet_ntoh.h>

#include <ac_memcpy.h>
#include <ac_memset.h>
#include <ac_inttypes.h>

/**
 * Initialize an AcArp
 *
 * See (RFC-826 Internet Protocol, V6)[https://tools.ietf.org/html/rfc826]
 * for the definition of the packet
 *
 * @return Length of the ArpReq
 */
AcU32 ac_arp_init(
    AcArp* pArpReq,                 ///< Arp Request to initialize
    AcU16  op,                      ///< Operation
    AcU16  hard_addr_frmt,          ///< Format of the hardware address
    AcU8   hard_addr_len,           ///< Length hardware address
    AcU16  proto_addr_frmt,         ///< Format of the protocol address
    AcU8   proto_addr_len,          ///< Length of a protocol address to convert
    AcU8*  src_hard_addr,           ///< Source hardware address, if AC_NULL zeroed
    AcU8*  src_proto_addr,          ///< Source protocol address, if AC_NULL zeroed
    AcU8*  dst_hard_addr,           ///< Destination hardware address, if AC_NULL zeroed
    AcU8*  dst_proto_addr) {        ///< Destination protocol address, if AC_NULL zeroed

  // Init header area
  pArpReq->frmt_hard_a = AC_HTON_U16(hard_addr_frmt);
  pArpReq->frmt_proto_a = AC_HTON_U16(proto_addr_frmt);
  pArpReq->len_hard_a = hard_addr_len;
  pArpReq->len_proto_a = proto_addr_len;
  pArpReq->op = AC_HTON_U16(op);

  AcU8* p = pArpReq->addresses;

  // Initialzie source hardware address
  if (src_hard_addr == AC_NULL) {
    ac_memset(p, 0, pArpReq->len_hard_a);
  } else {
    ac_memcpy(p, src_hard_addr, pArpReq->len_hard_a);
  }
  p += pArpReq->len_hard_a;

  // Initialzie source protocol address
  if (src_proto_addr == AC_NULL) {
    ac_memset(p, 0, pArpReq->len_proto_a);
  } else {
    ac_memcpy(p, src_proto_addr, pArpReq->len_proto_a);
  }
  p += pArpReq->len_proto_a;

  // Initialzie destination hardware address
  if (dst_hard_addr == AC_NULL) {
    ac_memset(p, 0, pArpReq->len_hard_a);
  } else {
    ac_memcpy(p, dst_hard_addr, pArpReq->len_hard_a);
  }
  p += pArpReq->len_hard_a;

  // Initialzie destination protocol address
  if (dst_proto_addr == AC_NULL) {
    ac_memset(p, 0, pArpReq->len_proto_a);
  } else {
    ac_memcpy(p, dst_proto_addr, pArpReq->len_proto_a);
  }
  p += pArpReq->len_proto_a;

  return p - (AcU8*)pArpReq;
}
