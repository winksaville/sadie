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

#ifndef SADIE_LIBS_INCS_AC_MESSAGE_H
#define SADIE_LIBS_INCS_AC_MESSAGE_H

#include <ac_assert.h>
#include <ac_attributes.h>
#include <ac_bits.h>
#include <ac_inttypes.h>
#include <ac_message.h>
#include <ac_status.h>

// Be sure AcStatus is 32 bits
ac_static_assert(sizeof(AcStatus) == sizeof(AcU32), L"sizeof(AcStatus != 4");


typedef struct AcMessagePool AcMessagePool;
typedef struct AcMessage AcMessage;
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
ac_static_assert(sizeof(AcOp) == sizeof(ac_u64), L"sizeof(AcOperation) != 8");

/**
 * Construct an operation
 */
#define OPERATION(p, ot, oc) \
   (AC_SET_BITS(AcU64, ((AcU64)0), oc,   0, 16)   \
   | AC_SET_BITS(AcU64, ((AcU64)0), ot,  16,  2)  \
   | AC_SET_BITS(AcU64, ((AcU64)0), 0,   18,  2)  \
   | AC_SET_BITS(AcU64, ((AcU64)0), p,   20, 44)) \

/**
 * Next AcMessage
 */
typedef struct AcNextPtr {
  AcNextPtr* next;
  AcMessage* msg;
} AcNextPtr AC_ATTR_ALIGNED_AC_U64;

/**
 * Message Header
 */
typedef struct AcMsgHdr {
  AcOp          op;        ///< Operation
  AcStatus      status;    ///< Status 0 == success
  AcU32         len_data;  ///< Length of data following AcMsgHdr in bytes
  AcU64         tag;       ///< tag defined by sender preserved in responses
} AcMsgHdr AC_ATTR_ALIGNED_AC_U64;

/**
 * Message
 */
typedef struct AcMessage {
  AcNextPtr*     next_ptr; ///< A 'pointer' the next message
  AcMessagePool* mp;       ///< The message pool this message belongs to
  AcMsgHdr       hdr;      ///< A message header
  AcU8           data[];   ///< Variable sized extra data, maybe 0
} AcMessage AC_ATTR_ALIGNED_AC_U64;

/**
 * The PROTOCOL for any system operations
 */
#define AC_SYSTEM_PROTOCOL 1

/**
 * AC_INIT_CMD is the first command sent to a component
 */
#define AC_INIT_CMDx OPERATION(AC_SYSTEM_PROTOCOL, AC_OPTYPE_CMD, 1)

/**
 * AC_DEINIT_CMD is the last command sent to a component
 */
#define AC_DEINIT_CMDx OPERATION(AC_SYSTEM_PROTOCOL, AC_OPTYPE_CMD, 2)

#endif
