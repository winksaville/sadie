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

#ifndef SADIE_COMPONENTS_AC_INET_LINK_INCS_AC_ETHER_H
#define SADIE_COMPONENTS_AC_INET_LINK_INCS_AC_ETHER_H

#include <ac_assert.h>
#include <ac_inttypes.h>
#include <ac_inet.h>

#define AC_ETHER_ADDR_LEN        6      ///< Ethernet Address length
#define AC_ETHER_MIN_LEN        60      ///< Minimum length of a Ethernet packet
#define AC_ETHER_HDR_LEN        14      ///< sizeof AcEtherHdr

/**
 * AC_ETHER_PROTO_xxx are values for AcEtherHdr.proto
 */
#define AC_ETHER_PROTO_IPV4     0x0800  ///< IPV4 protocol
#define AC_ETHER_PROTO_IPV6     0x86DD  ///< IPV6 protocol
#define AC_ETHER_PROTO_ARP      0x0806  ///< Address Resolution Protocol (ARP)

/**
 * Ethernet packet without Frame check sequence/CRC
 *
 * See (RFC-826 Internet Protocol, V6)[https://tools.ietf.org/html/rfc826]
 */
typedef struct AC_ATTR_PACKED {
  AcU8  dst[AC_ETHER_ADDR_LEN];  ///< Destination address
  AcU8  src[AC_ETHER_ADDR_LEN];  ///< Source address
  AcU16 proto;                   ///< Protocol type see AC_ETHER_PROTO_xxx
} AcEtherHdr;

/**
 * Some static asserts so we catch any obvious errors
 */
ac_static_assert(AC_ETHER_ADDR_LEN == 6, L"AC_ETHER_ADDR_LEN != 6");
ac_static_assert(AC_ETHER_MIN_LEN == 60, L"AC_ETHER_MIN_LEN != 60");
ac_static_assert(AC_ETHER_HDR_LEN == 14, L"AC_ETHER_HDR_LEN != 14");
ac_static_assert(sizeof(AcEtherHdr) == AC_ETHER_HDR_LEN, L"sizeof(AcEtherHdr) != 14");

/**
 * Ethernet ARP packet
 */
typedef struct AC_ATTR_PACKED {
  union {
    AcU8 min_packet[AC_ETHER_MIN_LEN];
    struct {
      AcEtherHdr ether_hdr;
      AcU8 src_hard_a[AC_ETHER_ADDR_LEN];
      AcU8 src_proto_a[AC_IPV4_ADDR_LEN];
      AcU8 dst_hard_a[AC_ETHER_ADDR_LEN];
      AcU8 dst_proto_a[AC_IPV4_ADDR_LEN];
    };
  };
} AcEtherArpIpv4;

/**
 * Initialize AcEtherHdr
 */
void ac_ether_init(
    AcEtherHdr* pEthHdr,            ///< Ether header to initialize
    const void *dst_addr,           ///< Destination address
    const void* src_addr,           ///< Source address
    AcU16 protocol);                ///< protocol of the contents

#endif
