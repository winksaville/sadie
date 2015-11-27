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
#include <ac_assert.h>
#include <ac_inttypes.h>

#include "ac_mpscfifo_misc.h"
#include "ac_mpscfifo.h"

/**
 * @see mpscfifo.h
 */
MpscFifo_t *initMpscFifo(MpscFifo_t *pQ, Msg_t *pStub) {
  pStub->pNext = AC_NULL;
  pQ->pHead = pStub;
  pQ->pTail = pStub;
  return pQ;
}

/**
 * @see mpscfifo.h
 */
Msg_t *deinitMpscFifo(MpscFifo_t *pQ) {
  // Assert that the Q empty
  ac_assert(pQ->pTail->pNext == AC_NULL);
  ac_assert(pQ->pTail == pQ->pHead);

  // Get the stub and null head and tail
  Msg_t *pStub = pQ->pHead;
  pQ->pHead = AC_NULL;
  pQ->pTail = AC_NULL;
  return pStub;
}

/**
 * @see mpscifo.h
 */
void add(MpscFifo_t *pQ, Msg_t *pMsg) {
  ac_assert(pQ != AC_NULL);
  if (pMsg != AC_NULL) {
    // Be sure pMsg->pNext == AC_NULL
    pMsg->pNext = AC_NULL;

    // Using Builtin Clang doesn't seem to support stdatomic.h
    Msg_t *pPrevHead = __atomic_exchange_n(&pQ->pHead, pMsg, __ATOMIC_ACQ_REL);
    __atomic_store_n(&pPrevHead->pNext, pMsg, __ATOMIC_RELEASE);

    // TODO: Support "blocking" which means use condition variable
  }
}

/**
 * @see mpscifo.h
 */
Msg_t *rmv(MpscFifo_t *pQ) {
  ac_assert(pQ != AC_NULL);
  Msg_t *pResult = pQ->pTail;
  Msg_t *pNext = __atomic_load_n(&pResult->pNext, __ATOMIC_ACQUIRE);
  if (pNext != AC_NULL) {
    // TODO: Support "blocking" which means use condition variable
    pQ->pTail = pNext;
    pResult->pNext = AC_NULL;
    pResult->pRspq = pNext->pRspq;
    pResult->pExtra = pNext->pExtra;
    pResult->cmd = pNext->cmd;
    pResult->arg = pNext->arg;
  } else {
    pResult = AC_NULL;
  }
  return pResult;
}
