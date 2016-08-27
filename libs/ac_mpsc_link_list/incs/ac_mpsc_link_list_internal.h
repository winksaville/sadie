/**
 * This software is released into the public domain.
 *
 * A MpscLinkList is a wait free/thread safe multi-producer
 * single consumer first in first out queue using a link list.
 * This algorithm is from Dimitry Vyukov's non intrusive MPSC code here:
 *   http://www.1024cores.net/home/lock-free-algorithms/queues/non-intrusive-mpsc-node-based-queue
 */

#ifndef SADIE_LIBS_AC_MPSC_LINK_LIST_INTERNAL_H
#define SADIE_LIBS_AC_MPSC_LINK_LIST_INTERNAL_H

#include <ac_message.h>

#include <ac_inttypes.h>

typedef struct AcMpscLinkList {
  AcNextPtr* head __attribute__(( aligned (64) ));
  AcNextPtr* tail __attribute__(( aligned (64) ));
  AcNextPtr stub;
  _Atomic(AcU32) count;
  _Atomic(AcU64) msgs_processed;
} AcMpscLinkList;

#endif
