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

#include <ac_inttypes.h>

#include <ac_assert.h>

typedef struct AcMsgPool AcMsgPool;

/**
 * A AC Message
 */
typedef struct AcMsg {
  ac_u64 arg1;          ///< argument1
  ac_u64 arg2;          ///< argument2
} AcMsg;

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
      ac_u32 opcode:16;     // Bits 0-15 is opcode
      ac_u32 flags:2;       // Bits 16-17 AC_RSV 0x0 == reserved
                            // Bits 16-17 AC_CMD 0x1 == cmd (no response expected)
                            // Bits 16-17 AC_REQ 0x2 == req (response expected)
                            // Bits 16-17 AC_RSP 0x3 == rsp A response to a previous command
      ac_u32 reserved:2;    // Bits 18-19 reserved
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
 * The PROTOCOL for any system operations
 */
#define AC_SYSTEM_PROTOCOL 1

/**
 * AC_INIT_CMD is the first command sent to a component
 */
DEFINE_AC_OPERATION(AC_INIT_CMD, AC_SYSTEM_PROTOCOL, AC_CMD, 1);

/**
 * AC_DEINIT_CMD is the last command sent to a component
 */
DEFINE_AC_OPERATION(AC_DEINIT_CMD, AC_SYSTEM_PROTOCOL, AC_CMD, 2);

#endif
