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

typedef struct ac_mpscfifo {
  ac_msg *phead;
  ac_msg *ptail;
} ac_mpscfifo;

/**
 * Initialize an ac_mpscfifo. Don't forget to empty the fifo
 * and delete the stub before freeing ac_mpscfifo.
 */
extern ac_mpscfifo* ac_mpscfifo_init(ac_mpscfifo* pq, ac_msg* pstub);

/**
 * Deinitialize the ac_mpscfifo and return the stub which
 * needs to be disposed of properly. Assumes the fifo is empty.
 */
extern ac_msg* ac_mpscfifo_deinit(ac_mpscfifo* pq);

/**
 * Add a ac_msg to the FIFo. This maybe used by multiple
 * threads and never blocks.
 */
extern void ac_mpscfifo_add_msg(ac_mpscfifo* pq, ac_msg* pmsg);

/**
 * Remove a ac_msg from the FIFO. This maybe used only by
 * a single thread and returns AC_NULL if the FIFO is empty.
 */
extern ac_msg* ac_mpscfifo_rmv_msg(ac_mpscfifo* pq);

/**
 * Remove a raw ac_msg from the FIFO. A raw message contents
 * are NOT preserved and the caller will be initializing the
 * contents after removal. This is useful in managing a pool
 * of empty messages.
 */
extern ac_msg* ac_mpscfifo_rmv_msg_raw(ac_mpscfifo* pq);


#endif
