/**
 * Declare a Msg_t
 */

#ifndef SADIE_LIBS_AC_MPSCFIFO_AC_MSG_H
#define SADIE_LIBS_AC_MPSCFIFO_AC_MSG_H

#include <ac_inttypes.h>

typedef struct _ac_msg {
  struct _ac_msg *pnext; // Next message
  void *prspq;           // Response queue, null if none
  void *pextra;          // Extra information, null if none
  ac_u32 cmd;            // Command to perform
  ac_u32 arg;            // argument
} ac_msg;

#endif
