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

#ifndef SADIE_COMPONENTS_AC_INET_LINK_INCS_AC_ARP_H
#define SADIE_COMPONENTS_AC_INET_LINK_INCS_AC_ARP_H

#include <ac_inttypes.h>

/**
 * AC_ARP_FRMT_HARD_A_xxx define the values for AcArp.frmt_hard_a. They define what "hardware" this
 * arp packet is being used to resvole.  A list can be found at
 * (Linux::/include/uapi/linux/if_arp.h)[http://goo.gl/w3SWYZ]
 */
#define AC_ARP_FRMT_HARD_A_ETHER  1 ///< Ethernet (see)[http://tools.ietf/.org/thml/rfc826]

/**
 * AC_ARP_OP_xxx defines the values for AcArp.op.
 */
#define AC_ARP_OP_REQ       1       ///< Request to resolve a protocol address to hardware
#define AC_ARP_OP_RPL       2       ///< Reply to AC_ARP_OP_REQ
                                    ///< (see)[https://tools.ietf.org/rfc/rfc826.txt]

#define AC_ARP_OP_REV_REQ   3       ///< Request to resolve a hardware address to protocol
#define AC_ARP_OP_REV_RPL   4       ///< Reply to AC_ARP_OP_REV_REQ
                                    ///< (see)[https://tools.ietf.org/rfc/rfc903.txt]

#define AC_ARP_OP_INV_REQ   8       ///< Request to resolve a protocol address to hardware
#define AC_ARP_OP_INV_RPL   9       ///< Reply to AC_ARP_OP_INV_REQ
                                    ///< (see)[https://tools.ietf.org/html/rfc2390] this
                                    ///< is used for Frame Relay stationa with a Data
                                    ///< Link Connection Identifier (DLCI) for use with
                                    ///< Permanent Virtual Circuit's (PVC)

/**
 * Arp packet
 *
 * See (RFC-826 Internet Protocol, V6)[https://tools.ietf.org/html/rfc826]
 */
typedef struct AC_ATTR_PACKED {
  AcU16 frmt_hard_a;      ///< Format Hardware address see AC_ARP_FRMT_HARD_A_xxx defines
  AcU16 frmt_proto_a;     ///< Format Protocol address see protocol defines for the particular
                          ///< hardware such as AC_ETHER_PROTO_ARP in ac_ether.h. A long list is
                          ///< found at (Linux::/include/uapi/linux/if_ether.h)[https://goo.gl/JVTe8s]
                          ///< in the ETH_P_xxx defines.
  AcU8  len_hard_a;       ///< length of hardware address see hardware specific header such as
                          ///< such AC_ETHER_ALEN in ac_ether.h
  AcU8  len_proto_a;      ///< length of protocol address fields
  AcU16 op;               ///< operation see AC_ARP_OP_xxx
  AcU8  addresses[];      ///< Variable length array with the source and destination addresses
} AcArp;

#define AC_ARP_SRC_HARD_ADDR(ac_arp) (&ac_arp.addresses[0])
#define AC_ARP_SRC_PROTO_ADDR(ac_arp) (AC_ARP_SRC_HARD_ADDR(ac_arp) + ac_req.len_hard_a)
#define AC_ARP_DST_HARD_ADDR(ac_arp) (AC_ARP_SRC_PROTO_ADDR(ac_arp) + ac_req.len_proto_a)
#define AC_ARP_DST_PROTO_ADDR(ac_arp) (AC_ARP_DST_HARD_ADR(ac_arp) + ac_req.len_hard_a)

/**
 * Initialize an AcArp
 *
 * See (RFC-826 Internet Protocol, V6)[https://tools.ietf.org/html/rfc826]
 * for the definition of the packet
 *
 * @return Size of the ArpReq
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
    AcU8*  dst_proto_addr);         ///< Destination protocol address, if AC_NULL zeroed

#endif
