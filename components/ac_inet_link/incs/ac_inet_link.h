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

#ifndef SADIE_LIBS_AC_INET_LINK_INCS_AC_INET_LINK_H
#define SADIE_LIBS_AC_INET_LINK_INCS_AC_INET_LINK_H

#include <ac_assert.h>
#include <ac_attributes.h>
#include <ac_inttypes.h>
#include <ac_status.h>

/**
 * See (RFC-2460 Internet Protocol, V6)[https://tools.ietf.org/html/rfc2460]
 */
typedef struct AC_ATTR_PACKED {
  ac_u8 version:4;        // Version = 6
  ac_u8 traffic_class:8;  // Traffic class see (section 7)[https://tools.ietf.org/html/rfc2460#section-7]
  ac_u32 flow_label:20;   // Flow label set (section 6)[https://tools.ietf.org/html/rfc2460#section-6]
  ac_u16 payload_length;  // Includes extension headers, if any, and payload that follow this header
  ac_u8 next_header;      // 59 == no next_header, IP Protocol see (www.iana.org Protocol Numbers)
                          //   [http://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml]
  ac_u8 hop_limit;        // If arrives as 0 discard. Before forwarding decrement
                          // and if it becomes 0 discard and do not forward.
  ac_u8 source[16];       // 16 byte source address
  ac_u8 destination[16];  // 16 byte destination address
} AcInetIpv6Hdr;

/**
 * See (RFC-791 Internet Protocol)[https://tools.ietf.org/html/rfc791]
 */
typedef struct AC_ATTR_PACKED {
  union {
    ac_u8 raw_u8[2];
    ac_u16 raw_u16;
    struct {
      ac_u16 offset_in_u64s:13; // Bits 0-12 this fragments byte offset == offset_in_u64s * 8
      ac_bool more_fragments:1; // 0 = last fragment, 1 = more fragments
      ac_bool dont_fragment:1;  // 0 = May fragment,  1 = dont' fragment
      ac_bool zero:1;           // always 0
    };
  };
} AcInetIpv4FragmentOffset;

/**
 * See (RFC-791 Internet Protocol)[https://tools.ietf.org/html/rfc791]
 */
typedef struct AC_ATTR_PACKED {
  ac_u8 version:4;         // Version = 4
  ac_u8 ihl:4;             // Interent Header Lenth
  ac_u8 tos;               // Type of service
  ac_u16 total_length;     // Total length of the datagram
  ac_u16 identification;   // sequence number
  AcInetIpv4FragmentOffset fragment_offset; // bits 0-12 are offset
                                            // bit 13 == 0 = Last Fragment, 1 = More Fragments
                                            // bit 15 == 0 always
                                            // bit 14 == 0 = May Fragment,  1 = Don't Fragment
  ac_u8 time_to_live;      // 0 == time has expired and this packet is not forwarded
  ac_u8 protocol;          // IP Protocol see (www.iana.org Protocol Numbers)
                           //   [http://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml]
  ac_u16 header_checksum;  // Checksum for the header
  ac_u8 source[4];         // 4 byte source address
  ac_u8 destination[4];    // 4 byte destination address
} AcInetIpv4Hdr;

typedef struct {
  void*     base;       // Base address of the data
  ac_size_t len;        // Length of the data
} AcIoBuff;

/**
 * AcProtocolOpcode FLAG values
 */
#define AC_RSV 0x0
#define AC_CMD 0x1
#define AC_REQ 0x2
#define AC_RSP 0x3

typedef struct AC_ATTR_PACKED {
  union {
    ac_u64 operation;
    struct {
      ac_u64 opcode:16;     // Bits 0-15 is opcode
      ac_u64 flags:2;       // Bits 16-17 AC_RSV 0x0 == reserved
                            // Bits 16-17 AC_CMD 0x1 == cmd (no response expected)
                            // Bits 16-17 AC_REQ 0x2 == req (response expected)
                            // Bits 16-17 AC_RSP 0x3 == rsp A response to a previous command
      ac_u64 reserved:2;    // Bits 18-19 reserved
      ac_u64 protocol:44;   // Bits 20-63 protocol
    };
  };
} AcOperation;
ac_static_assert(sizeof(AcOperation) == sizeof(ac_u64),
   L"AcOperation != 4");

/**
 * Define an AcOperation
 */
#define DEFINE_AC_OPERATION(name, p, f, o) \
static const AcOperation name = { \
  .protocol = p, \
  .flags = f, \
  .opcode = o, \
}

/**
 * Define the name_CMD, name_REQ and name_RSP operations for
 * the name_PROTOCOL, which must be defined.
 */
#define DEFINE_AC_OPERATIONS(name, opcode) \
  DEFINE_AC_OPERATION(name ## _CMD, name ## _PROTOCOL, AC_CMD, opcode); \
  DEFINE_AC_OPERATION(name ## _REQ, name ## _PROTOCOL, AC_REQ, opcode); \
  DEFINE_AC_OPERATION(name ## _RSP, name ## _PROTOCOL, AC_RSP, opcode)

/**
 * For AC_INET_SEND_PACKET_PROTOCOL define:
 *   AC_INET_SEND_PACKET_CMD
 *   AC_INET_SEND_PACKET_REQ
 *   AC_INET_SEND_PACKET_RSP
 */
#define AC_INET_SEND_PACKET_PROTOCOL   0x1234
DEFINE_AC_OPERATIONS(AC_INET_SEND_PACKET, 0x1);

/**
 * AcInetSendPacetOpCr is either AC_INET_SEND_PACKET_CMD or _REQ.
 * If _REQ is sent then AcInetSendPacketOpRsp must be sent, for _CMD
 * the _RSP may be sent, typically when an error occurs, usually
 * detected by before transmitting.
 */
typedef struct {
  AcOperation op;       // Op is the operation AC_INET_SEND_PACKET_CMD | _REQ
  ac_u64    tag;        // tag an app defined tag
  ac_u64    app_vec_cnt;// Number of AcIoBuff's for application data
  AcIoBuff* link_buff;  // Pointer to the Link header buffer
  AcIoBuff* ip_buff;    // Pointer to the IP header buffer
  AcIoBuff* trans_buff; // Pointer to the Transport header buffer
  AcIoBuff* app_buffs[];// Pointer to the app data buffer
} AcInetSendPacketOpCr;

/**
 * AcInetSendPacketRsp is optional and returns 0 if succesfully
 * queued for sending does not mean it was sent or recevied.
 * Generally will only be returned if an error unless po.flags = AC_REQ
 */
typedef struct {
  AcOperation op;       // Opeeration is AC_INET_SEND_PACKET_RSP
  ac_u64    tag;        // tag an app defined tag
  AcStatus  status;     // 0 == successful, !0 == error
} AcInetSendPacketOpRsp;

/**
 * Initialize this module
 */
void AcInetLink_init(void);

#endif
