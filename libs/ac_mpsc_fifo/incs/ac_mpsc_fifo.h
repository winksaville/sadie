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
 * The fifo has a head and tail, the AcMem elements are added
 * to the head of the queue and removed from the tail.
 * To allow for a wait free algorithm a there is an extra AcMem,
 * called the stubm, so that a single atomic instruction can be
 * used to add and remove an AcMem. The AcMpscFifo_deinit must
 * be called so that the stub maybe returned to is proper pool.
 *
 * A consequence of this algorithm is that when you add an
 * AcMem to the queue a different AcMem is returned when
 * you remove it from the queue. Of course the contents are
 * the same but the returned pointer will be different.
 */

#ifndef SADIE_LIBS_AC_MPSCFIFO_AC_MPSCFIFO_H
#define SADIE_LIBS_AC_MPSCFIFO_AC_MPSCFIFO_H

#include <ac_mem.h>

typedef struct AcMpscFifo {
  AcMem* head;
  AcMem* tail;
} AcMpscFifo;

/**
 * Add a AcMem to the fifo. This maybe used by multiple
 * threads and never blocks.
 *
 * @params fifo is an iniitalized AcMpscFifo
 */
void AcMpscFifo_add_ac_mem(AcMpscFifo* fifo, AcMem* mem);

/**
 * Remove a AcMem from the FIFO. This maybe used only by
 * a single thread and returns AC_NULL if the FIFO is empty.
 *
 * @params fifo is an iniitalized AcMpscFifo
 *
 * @return AcMem or AC_NULL if empty
 */
AcMem* AcMpscFifo_rmv_ac_mem(AcMpscFifo* fifo);

/**
 * Remove a raw AcMem from the FIFO. A raw memer contents
 * are NOT preserved and the caller will be initializing the
 * contents after removal. This is useful in managing a pool
 * of empty memers.
 *
 * @params fifo is an iniitalized AcMpscFifo
 *
 * @return AcMem or AC_NULL if empty
 */
AcMem* AcMpscFifo_rmv_ac_mem_raw(AcMpscFifo* fifo);

/**
 * Deinitialize the AcMpscFifo and return the stub which
 * needs to be disposed of properly. Assumes the fifo is empty.
 *
 * @params fifo is an iniitalized AcMpscFifo
 */
void AcMpscFifo_deinit(AcMpscFifo* fifo);

/**
 * Initialize an AcMpscFifo using the stub
 *
 * @params fifo is an uniniitalized AcMpscFifo
 * @params stub is the AcMem which will be the stub
 *
 * @return 0 (AC_STATUS_OK) if successfull
 */
AcStatus AcMpscFifo_init_with_stub(AcMpscFifo* fifo, AcMem* stub);

/**
 * Initialize an AcMpscFifo.
 *
 * @params fifo is an uniniitalized AcMpscFifo
 * @params mem_size number of bytes in AcMem.data[]
 *
 * @return 0 (AC_STATUS_OK) if successfull
 */
AcStatus AcMpscFifo_init(AcMpscFifo* fifo, ac_u32 mem_size);

/**
 * Early initialize module
 */
void AcMpscFifo_early_init(void);

#endif
