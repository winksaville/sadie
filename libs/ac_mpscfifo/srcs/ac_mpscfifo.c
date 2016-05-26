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

/**
 * A ac_mpscfifo is a wait free/thread safe multi-producer
 * single consumer first in first out queue. This algorithm
 * is from Dimitry Vyukov's non intrusive MPSC code here:
 *   http://www.1024cores.net/home/lock-free-algorithms/queues/non-intrusive-mpsc-node-based-queue
 *
 * The FIFO has a head and tail, the elements are added
 * to the head of the queue and removed from the tail. To
 * allow for a wait free algorithm a stub element is used
 * so that a single atomic instruction can be used to add and
 * remove an element.
 *
 * The FIFO is composed of a simgularly linked list of next
 * pointers stored in each element. The list starts at the
 * ptail->pnext and ends with a phead->pnext being null.
 *
 * A consequence of the algorithm is that when an element is
 * added to the FIFO a different element is returned when
 * you remove it from the queue. Of course the contents are
 * the same, but the returned pointer will be different.
 *
 *
 *
 * Some details:
 *
 * A ac_mpscfifo has two fields phead and ptail:
 *   typedef struct _ac_mpscfifo {
 *     ac_msg *phead;
 *     ac_msg *ptail;
 *   } ac_mpscfifo;
 *
 * A ac_msg is declared as:
 * typedef struct _ac_msg {
 *   struct _ac_msg *pnext; // Next message
 *   ....
 * }
 *
 * When ac_mpscfifo_initialized an empty fifo is created
 * with one stub ac_msg whose pnext field is AC_NULL and
 * ac_mpscfifo.phead and .ptail both point at this stub msg.
 *
 * The pq->phead is where ac_msg's are added to the queue and
 * points to the most recent element and is at the end of
 * the list thus this element always has its pnext == AC_NULL.
 *
 * pq->ptail->pnext is the oldest ac_msg and is the next
 * element that will be removed and is only AC_NULL when the
 * FIFO is empty.
 *
 *
 *
 * Add an ac_msg to the FIFO, invoked by multiple threads
 * (Multiple Producers):
 *
 *  Step 1) Set pmsg's pnext to AC_NULL as this is the end
 *  of the list.
 *      pmsg->pnext = AC_NULL
 *
 *  Step 2) Exchange pq->phead and pmsg so pq->phead now
 *  points at the new newest ac_msg and pprev_head points
 *  at the previous head.
 *      pprev_head = exchange(&pq->phead, pmsg)
 *
 *  Step 3) Store pmsg into the pnext of the previous head
 *  which actually adds the new msg to the FIFO.
 *      pprev_head->pnext = pmsg;
 *
 *
 *
 * Remove an ac_msg from the FIFO, invoked by one thread
 * (Single Consumer):
 *
 * Step 1) Use the current stub value to return the result
 *     ac_msg *presult = pq->ptail;
 *
 * Step 2) Get the oldest element, serialize with Step 3 in add_msg
 *     ac_msg *poldest = __atomic_load_n(&presult->pnext, __ATOMIC_ACQUIRE);
 *
 * Step 3) If list is empty return AC_NULL
 *     if (poldest == AC_NULL) {
 *       return AC_NULL;
 *     }
 *
 * Step 4) The poldest becomes new stub which if we are removing
 * the last element then poldest->pnext is AC_NULL because add_msg
 * made it so.
 *     pq->ptail = poldest;
 *
 * Step 5) Copy the contents of poldest ac_msg to presult, although
 * not strictly necessary we set presult->pnext to AC_NULL so we
 * fail fast is someone uses it.
 *     presult->pnext = AC_NULL;
 *     presult->prspq = poldest->prspq;
 *     presult->cmd = poldest->cmd;
 *     presult->arg = poldest->arg;
 *     presult->arg_u64 = poldest->arg_u64;
 *
 * Step 6) Return presult
 *     return presult
 */

#include <ac_mpscfifo.h>

#include <ac_assert.h>
#include <ac_inttypes.h>
#include <ac_memmgr.h>
#include <ac_msg_pool.h>

AcMsgPool* pstubs = AC_NULL;

/**
 * @see mpscifo.h
 */
void ac_mpscfifo_add_msg(ac_mpscfifo* pq, ac_msg* pmsg) {
  ac_assert(pq != AC_NULL);
  if (pmsg != AC_NULL) {
    // Step 1) Set pmsg's pnext to AC_NULL as this is the end
    //  of the list.
    pmsg->pnext = AC_NULL;

    // Step 2) Exchange pq->phead and pmsg so pq->phead now
    // points at the new newest. Serialize with other producers.
    ac_msg *pprev_head = __atomic_exchange_n(&pq->phead, pmsg, __ATOMIC_ACQ_REL);

    // Step 3) Store pmsg into the pnext of the previous head
    // which actually adds the new msg to the FIFO. Serialize
    // with rmv_msg Step 4 if the list is empty.
    __atomic_store_n(&pprev_head->pnext, pmsg, __ATOMIC_RELEASE);
  }
}

/**
 * @see mpscifo.h
 */
ac_msg* ac_mpscfifo_rmv_msg(ac_mpscfifo* pq) {
  ac_assert(pq != AC_NULL);
  // Step 1) Use the current stub value to return the result
  ac_msg *presult = pq->ptail;

  // Step 2) Get the oldest element, serialize with Step 3 in add_msg
  ac_msg *poldest = __atomic_load_n(&presult->pnext, __ATOMIC_ACQUIRE);

  // Step 3) If list is empty return AC_NULL
  if (poldest == AC_NULL) {
    return AC_NULL;
  }

  // Step 4) The poldest becomes new ptail stub. If we are removing
  // the last element then poldest->pnext is AC_NULL because add_msg
  // made it so.
  pq->ptail = poldest;

  // Step 5) Copy the contents of poldest ac_msg to presult, although
  // not strictly necessary we set presult->pnext to AC_NULL so we
  // fail fast is someone uses it.
  presult->pnext = AC_NULL;
  presult->cmd = poldest->cmd;
  presult->arg = poldest->arg;
  presult->arg_u64 = poldest->arg_u64;
  // DO Not copy AcMsg.pool its associated with the AcMsg as originally allocated.
  //presult->pool = poldest->pool;

  // Step 6) Return presult
  return presult;
}

/**
 * @see mpscifo.h
 */
ac_msg* ac_mpscfifo_rmv_msg_raw(ac_mpscfifo* pq) {
  ac_assert(pq != AC_NULL);
  // Step 1) Use the current stub value to return the result
  ac_msg *presult = pq->ptail;

  // Step 2) Get the oldest element, serialize with Step 3 in add_msg
  ac_msg *poldest = __atomic_load_n(&presult->pnext, __ATOMIC_ACQUIRE);

  // Step 3) If list is empty return AC_NULL
  if (poldest == AC_NULL) {
    return AC_NULL;
  }

  // Step 4) The poldest becomes new ptail stub. If we are removing
  // the last element then poldest->pnext is AC_NULL because add_msg
  // made it so.
  pq->ptail = poldest;

  // Step 5) Return presult
  return presult;
}

/**
 * @see ac_mpscfifo.h
 */
void ac_mpscfifo_deinit(ac_mpscfifo* pq) {
  // Assert that the Q is empty
  ac_assert(pq->ptail->pnext == AC_NULL);
  ac_assert(pq->phead != AC_NULL);
  ac_assert(pq->ptail != AC_NULL);
  ac_assert(pq->ptail == pq->phead);

  // Return the stub and null head and tail
  AcMsg_ret(pq->phead);
  pq->phead = AC_NULL;
  pq->ptail = AC_NULL;
}

/**
 * @see ac_mpscfifo.h
 */
void ac_mpscfifo_init_with_stub(ac_mpscfifo* pq, AcMsg* pstub) {
  ac_assert(pq != AC_NULL);
  ac_assert(pstub != AC_NULL);

  pstub->pnext = AC_NULL;
  pq->phead = pstub;
  pq->ptail = pstub;
}

/**
 * @see ac_mpscfifo.h
 */
void ac_mpscfifo_init(ac_mpscfifo* pq) {
  AcMsg* pstub = AcMsg_get(pstubs);
  if (pstub == AC_NULL) {
    // Stubs is empty to create a new one.
    pstub = AcMsg_alloc();
    pstub->pool = pstubs;
  }
  ac_assert(pstub != AC_NULL);
  ac_mpscfifo_init_with_stub(pq, pstub);
}

/**
 * Early initialize module
 */
__attribute__((constructor))
void ac_mpscfifo_early_init(void) {
  // Create a small pool of stubs we'll add
  // more as needed, adjust as necessary.
  pstubs = AcMsgPool_create(1);
}
