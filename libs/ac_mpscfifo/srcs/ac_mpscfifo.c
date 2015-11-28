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

#include <ac_mpscfifo_misc.h>
#include <ac_mpscfifo.h>

/**
 * @see mpscfifo.h
 */
ac_mpscfifo *ac_init_mpscfifo(ac_mpscfifo *pq, ac_msg *pStub) {
  pStub->pnext = AC_NULL;
  pq->phead = pStub;
  pq->ptail = pStub;
  return pq;
}

/**
 * @see mpscfifo.h
 */
ac_msg *ac_deinit_mpscfifo(ac_mpscfifo *pq) {
  // Assert that the Q empty
  ac_assert(pq->ptail->pnext == AC_NULL);
  ac_assert(pq->ptail == pq->phead);

  // Get the stub and null head and tail
  ac_msg *pStub = pq->phead;
  pq->phead = AC_NULL;
  pq->ptail = AC_NULL;
  return pStub;
}

/**
 * @see mpscifo.h
 */
void ac_add_msg(ac_mpscfifo *pq, ac_msg *pmsg) {
  ac_assert(pq != AC_NULL);
  if (pmsg != AC_NULL) {
    // Be sure pmsg->pnext == AC_NULL
    pmsg->pnext = AC_NULL;

    // Using Builtin Clang doesn't seem to support stdatomic.h
    ac_msg *pprev_head = __atomic_exchange_n(&pq->phead, pmsg, __ATOMIC_ACQ_REL);
    __atomic_store_n(&pprev_head->pnext, pmsg, __ATOMIC_RELEASE);

    // TODO: Support "blocking" which means use condition variable
  }
}

/**
 * @see mpscifo.h
 */
ac_msg *ac_rmv_msg(ac_mpscfifo *pq) {
  ac_assert(pq != AC_NULL);
  ac_msg *presult = pq->ptail;
  ac_msg *pnext = __atomic_load_n(&presult->pnext, __ATOMIC_ACQUIRE);
  if (pnext != AC_NULL) {
    // TODO: Support "blocking" which means use condition variable
    pq->ptail = pnext;
    presult->pnext = AC_NULL;
    presult->prspq = pnext->prspq;
    presult->pextra = pnext->pextra;
    presult->cmd = pnext->cmd;
    presult->arg = pnext->arg;
  } else {
    presult = AC_NULL;
  }
  return presult;
}
