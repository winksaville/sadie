/**
 * Declare a Msg_t
 */

#ifndef SADIE_LIBS_AC_MPSCFIFO_AC_MSG_H
#define SADIE_LIBS_AC_MPSCFIFO_AC_MSG_H

#include <ac_inttypes.h>

typedef struct _msg_t {
  struct _msg_t *pNext; // Next message
  void *pRspq;          // Response queue, null if none
  void *pExtra;         // Extra information, null if none
  ac_u32 cmd;           // Command to perform
  ac_u32 arg;           // argument
} Msg_t;

#endif
