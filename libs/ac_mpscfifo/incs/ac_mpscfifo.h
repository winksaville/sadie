/**
 * A MpscFifo is a wait free/thread safe multi-producer
 * single consumer first in first out queue. This algorithm
 * is from Dimitry Vyukov's non intrusive MPSC code here:
 *   http://www.1024cores.net/home/lock-free-algorithms/queues/non-intrusive-mpsc-node-based-queue
 *
 * The fifo has a head and tail, the elements are added
 * to the head of the queue and removed from the tail.
 * To allow for a wait free algorithm a stub element is used
 * so that a single atomic instruction can be used to add and
 * remove an element. Therefore, when you create a queue you
 * must pass in an areana which is used to manage the stub.
 *
 * A consequence of this algorithm is that when you add an
 * element to the queue a different element is returned when
 * you remove it from the queue. Of course the contents are
 * the same but the returned pointer will be different.
 */

#ifndef SADIE_LIBS_AC_MPSCFIFO_AC_MPSCFIFO_H
#define SADIE_LIBS_AC_MPSCFIFO_AC_MPSCFIFO_H

#include <ac_msg.h>

typedef struct _ac_mpscfifo {
  ac_msg *pHead;
  ac_msg *pTail;
} ac_mpscfifo;

/**
 * Initialize an ac_mpscfifo. Don't forget to empty the fifo
 * and delete the stub before freeing ac_mpscfifo.
 */
extern ac_mpscfifo *initMpscFifo(ac_mpscfifo *pQ, ac_msg *pStub);

/**
 * Deinitialize the ac_mpscfifo and return the stub which
 * needs to be disposed of properly. Assumes the fifo is empty.
 */
extern ac_msg *deinitMpscFifo(ac_mpscfifo *pQ);

/**
 * Add a ac_msg to the Queue. This maybe used by multiple
 * entities on the same or different thread. This will never
 * block as it is a wait free algorithm.
 */
extern void add(ac_mpscfifo *pQ, ac_msg *pMsg);

/**
 * Remove a ac_msg from the Queue. This maybe used only by
 * a single thread and returns nil if non-blocking.
 */
extern ac_msg *rmv(ac_mpscfifo *pQ);

#endif
