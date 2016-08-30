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

#ifndef SADIE_LIBS_INCS_AC_MSG_H
#define SADIE_LIBS_INCS_AC_MSG_H

#include <ac_assert.h>
#include <ac_attributes.h>
#include <ac_bits.h>
#include <ac_inttypes.h>
#include <ac_msg.h>
#include <ac_status.h>

typedef struct AcMsgPool AcMsgPool;
typedef struct AcMsg AcMsg;
typedef struct AcNextPtr AcNextPtr;


/**
 * AcProtocolOpcode FLAG values
 */
#define AC_OPTYPE_RSV 0x0
#define AC_OPTYPE_CMD 0x1
#define AC_OPTYPE_REQ 0x2
#define AC_OPTYPE_RSP 0x3

typedef struct AC_ATTR_PACKED {
  union {
    AcU64 operation;       ///< The full 64 bit operation
    struct {
      AcU32 opcode:16;     ///< Bits  0-15 is opcode
      AcU32 optype:2;      ///< Bits 16-17 AC_OPTYPE_RSV 0x0 == reserved
                           ///< Bits 16-16 AC_OPTYPE_CMD 0x1 == cmd (no response expected)
                           ///< Bits 16-17 AC_OPTYPE_REQ 0x2 == req (response expected)
                           ///< Bits 16-17 AC_OPTYPE_RSP 0x3 == rsp A response to a previous command
      AcU64 ver:2;         ///< Bits 18-19 version must be 0
      AcU64 protocol:44;   ///< Bits 20-63 protocol
    };
  };
} AcOp;
ac_static_assert(sizeof(AcOp) == sizeof(ac_u64), L"sizeof(AcOp) != 8");

/**
 * Construct an operation
 */
#define OPERATION(p, ot, oc) \
   (AC_SET_BITS(AcU64, ((AcU64)0), oc, 16, 0) \
   | AC_SET_BITS(AcU64, ((AcU64)0), ot, 2, 16) \
   | AC_SET_BITS(AcU64, ((AcU64)0), 0, 2, 18) \
   | AC_SET_BITS(AcU64, ((AcU64)0), p, 44, 20)) \

/**
 * Next AcMsg
 */
typedef struct AcNextPtr {
  AcNextPtr* next;
  AcMsg* msg;
} AcNextPtr AC_ATTR_ALIGNED_AC_U64;

/**
 * An Async Component Message. AcMsg's can be transported between
 * systems and therefore the position of certain fields must be
 * constant, see the static asserts below.
 */
typedef struct AcMsg {
  AcNextPtr*    next_ptr;  ///< A 'pointer' the next message
  AcMsgPool*    mp;        ///< The message pool this message belongs to

  AcOp          op;        ///< Operation
  AcU64         tag;       ///< tag defined by sender preserved in responses
  AcStatus      status;    ///< Status 0 == success
  AcU32         len_extra; ///< Length in bytes of extra data following AcMsg
  AcU8          extra[];   ///< Variable sized extra data, len_extra == 0 if no extra data
} AcMsg AC_ATTR_ALIGNED_AC_U64;

// Be sure AcOp is 64 bits
ac_static_assert(sizeof(AcOp) == sizeof(AcU64), L"sizeof(AcOp != 8");

// Be sure AcStatus is 32 bits
ac_static_assert(sizeof(AcStatus) == sizeof(AcU32), L"sizeof(AcStatus != 4");

// Be sure offset of AcMsg.op is on a AcU64 boundary
ac_static_assert((AC_OFFSET_OF(AcMsg, op) % sizeof(AcU64)) == 0,
    L"offset of AcMsg.op is not on AcU64 boundary");

// Check poision of tag
ac_static_assert((AC_OFFSET_OF(AcMsg, tag) - AC_OFFSET_OF(AcMsg, op)) ==  8,
    L"Expecting AcMsg.tag to follow AcMsg.op");

// Check poision of status
ac_static_assert((AC_OFFSET_OF(AcMsg, status) - AC_OFFSET_OF(AcMsg, tag)) ==  8,
    L"Expecting AcMsg.status to follow AcMsg.tag");

// Check poision of len_extra
ac_static_assert((AC_OFFSET_OF(AcMsg, len_extra) - AC_OFFSET_OF(AcMsg, status)) ==  4,
    L"Expecting AcMsg.len_extra to follow AcMsg.status");

// Check poision of extra
ac_static_assert((AC_OFFSET_OF(AcMsg, extra) - AC_OFFSET_OF(AcMsg, len_extra)) ==  4,
    L"Expecting AcMsg.extra to follow AcMsg.len_extra");

/**
 * The PROTOCOL for any system operations
 */
#define AC_SYSTEM_PROTOCOL 1

/**
 * AC_INIT_CMD is the first command sent to a component
 */
#define AC_INIT_CMD OPERATION(AC_SYSTEM_PROTOCOL, AC_OPTYPE_CMD, 1)

/**
 * AC_DEINIT_CMD is the last command sent to a component
 */
#define AC_DEINIT_CMD OPERATION(AC_SYSTEM_PROTOCOL, AC_OPTYPE_CMD, 2)

#endif
